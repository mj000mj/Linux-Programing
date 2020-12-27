#include "wrap.h"

#define TCP_PORT 9527
#define MAX_LISTEN 1024

struct client_info
{
	int fd;
	struct sockaddr_in c_addr;
};

int main(int args, char *argv[])
{
	int i, j, maxi;
	int listenfd, sockfd, cfd, maxfd;
	struct sockaddr_in ServerAddr, ClientAddr;
	socklen_t CSlen;
	int ret;
	int opt;
	pthread_t pthrd;
	struct client_info client[FD_SETSIZE];
	fd_set rset, allset;
	char str[16];
	char readBuf[BUFSIZ];
	ssize_t readLen;

	bzero(&ServerAddr, sizeof(ServerAddr));
	bzero(&ClientAddr, sizeof(ClientAddr));

	listenfd = Socket(AF_INET, SOCK_STREAM, 0);

	ServerAddr.sin_family = AF_INET;
	ServerAddr.sin_port = htons(TCP_PORT);
	ServerAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	opt = 1;
	setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, (const void *)&opt, sizeof(opt));

	Bind(listenfd, (struct sockaddr *)&ServerAddr, sizeof(ServerAddr));

	Listen(listenfd, MAX_LISTEN);

	FD_ZERO(&allset);

	FD_SET(listenfd, &allset);

	for (i = 0; i < FD_SETSIZE; ++i)
	{
		client[i].fd = -1;
	}

	client[0].fd = listenfd;

	maxfd = listenfd;
	maxi = -1;

	while(1)
	{
		rset = allset;
		ret = select(maxfd+1, &rset, NULL, NULL, NULL);
		if (ret < 0)
		{
			Perr_exit("Select error\n");
		}

		if (ret > 0)
		{
			//printf("select return value : %d\n", ret);
			if (FD_ISSET(listenfd, &rset))
			{
				printf("listenfd is in the rset!\n", ret);
				CSlen = sizeof(ClientAddr);
				cfd = Accept(listenfd, (struct sockaddr *)&ClientAddr, &CSlen);

				if (cfd > 1024)
				{
					printf("Exceed the max listen socket fd, Deny Accept");
					Close(cfd);
					goto Msg;
				}

				for (i = 1; i < FD_SETSIZE; ++i)
				{
					if (client[i].fd < 0)
					{
						client[i].fd = cfd;
						client[i].c_addr = ClientAddr;
		
						printf("New client connected Address : %s, Port: %d\n", 
			    				inet_ntop(AF_INET, &client[i].c_addr.sin_addr, 
								str, sizeof(str)), ntohs(client[i].c_addr.sin_port));
						break;
					}
				}
				if (i == FD_SETSIZE)
				{
					fputs("too many clients\n", stderr);
					exit(1);
				}

				FD_SET(cfd, &allset);

				if (maxfd < cfd)
				{
					maxfd = cfd;
				}

				if (maxi < i)
				{
					maxi = i;
				}

				if (--ret == 0)
				{
					printf("only new socket come and Continue!\n");
					continue;
				}
			}
Msg:
			//printf("Any socket msg com!\n");
			for (j = 0; j <= maxi; j++)
			{
				if (client[j].fd < 0)
				{
					continue;
				}

				if (FD_ISSET(client[j].fd, &rset))
				{
					printf("client %d is received msg , fd = %d!\n", j, client[j].fd);
					readLen = Read(client[j].fd, readBuf, BUFSIZ);

					if (readLen == 0)
					{
						printf("client %d is closed!\n", j);
						Close(client[j].fd);
						FD_CLR(client[j].fd, &allset);
						client[j].fd = -1;
					}
					else
					{
						Write(STDOUT_FILENO, "Receive msg : ", sizeof("Receive msg : "));
						Write(STDOUT_FILENO, readBuf, readLen);

						for (i = 0; i < readLen; ++i)
						{
							readBuf[i] = toupper(readBuf[i]);
						}

						Write(client[j].fd, readBuf, readLen);
					}

					if (--ret == 0)
					{
						break;
					}
				}
			}
		}
		
	}

	Close(listenfd);

	return 0;
}
