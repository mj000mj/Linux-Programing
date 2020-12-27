#include "wrap.h"

#define SERVER_PORT 8000

int main(int argc, char const *argv[])
{
	int i, j, maxi;
	int res;
	int listenfd, cfd, epfd;
	struct sockaddr_in ServerAddr, ClientAddr;
	socklen_t CSlen;
	int retNum;
	char str[16];
	char readBuf[BUFSIZ];
	ssize_t readLen;
	struct epoll_event tep, ep[MAX_LISTEN];
	int client[MAX_LISTEN];

	bzero(&ServerAddr, sizeof(ServerAddr));
	bzero(&ClientAddr, sizeof(ClientAddr));
	for (i = 0; i < MAX_LISTEN; ++i)
	{
		client[i] = -1;
	}

	listenfd = initTCPSocket(&ServerAddr, (short)SERVER_PORT);
	epfd = epoll_create(MAX_LISTEN);
	if (epfd < 0)
	{
		Perr_exit("epoll_create error");
	}

	tep.events = EPOLLIN;
	tep.data.fd = listenfd;
	res = epoll_ctl(epfd, EPOLL_CTL_ADD, listenfd, &tep);
	if (res == -1)
	{
		Perr_exit("epoll_ctl error\n");
	}

	maxi = 0;

	while(1)
	{
		retNum = epoll_wait(epfd, ep, maxi+1, -1);

		for (i = 0; i < retNum; ++i)
		{
			if (!(ep[i].events & EPOLLIN))
			{
				continue;
			}

			if ((ep[i].data.fd == listenfd))
			{
				CSlen = sizeof(ClientAddr);
				cfd = Accept(listenfd, (struct sockaddr *)&ClientAddr, &CSlen);

				for (j = 0; j < MAX_LISTEN; ++j)
				{
					if (client[j] < 0)
					{
						client[j] = cfd;
						break;
					}
				}

				if (j == MAX_LISTEN)
				{
					Perr_exit("too many client\n");
				}

				if (maxi < j)
				{
					maxi = j;
				}

				tep.events = EPOLLIN;
				tep.data.fd = cfd;
				res = epoll_ctl(epfd, EPOLL_CTL_ADD, cfd, &tep);
				if (res == -1)
				{
					Perr_exit("epoll_ctl error\n");
				}
			}
			else
			{
				cfd = ep[i].data.fd;
				readLen = Read(cfd, readBuf, BUFSIZ);

				if (readLen == 0)
				{
					printf("Client %d has been closed!\n", cfd);
					for (j = 0; j < maxi; ++j)
					{
						if (client[j] == cfd)
						{
							client[j] = -1;
							break;
						}
					}
					
					res = epoll_ctl(epfd, EPOLL_CTL_DEL, cfd, NULL);
					if (res == -1)
					{
						Perr_exit("epoll_ctl error\n");
					}
					Close(cfd);
				}
				else if (readLen < 0)
				{
					if (errno == ECONNRESET)
					{
						printf("Client %d has been closed for ECONNRESET!\n", cfd);
						for (j = 0; j < maxi; ++j)
						{
							if (client[j] == cfd)
							{
								client[j] = -1;
								break;
							}
						}
						
						res = epoll_ctl(epfd, EPOLL_CTL_DEL, cfd, NULL);
						if (res == -1)
						{
							Perr_exit("epoll_ctl error\n");
						}
						Close(cfd);
					}
					else
					{
						Perr_exit("read error\n");
					}
				}
				else
				{
					Write(STDOUT_FILENO, "Message : ", sizeof("Message : "));
					Write(STDOUT_FILENO, readBuf, readLen);

					for (j = 0; j < readLen; ++j)
					{
						readBuf[j] = toupper(readBuf[j]);
					}

					Write(ep[i].data.fd, readBuf, readLen);
				}
			}
			
		}
	}

	Close(listenfd);
	Close(epfd);

	return 0;
}