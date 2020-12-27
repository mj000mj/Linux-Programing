#include "wrap.h"

#define SERVER_PORT 8000

int main(int argc, char const *argv[])
{
	int i, j, maxi;
	int listenfd, cfd;
	struct sockaddr_in ServerAddr, ClientAddr;
	socklen_t CSlen;
	int retNum;
	char str[16];
	char readBuf[BUFSIZ];
	ssize_t readLen;
	struct pollfd client[MAX_LISTEN];

	bzero(&ServerAddr, sizeof(ServerAddr));
	bzero(&ClientAddr, sizeof(ClientAddr));
	for (i = 0; i < MAX_LISTEN; ++i)
	{
		client[i].fd = -1;
	}

	listenfd = initTCPSocket(&ServerAddr, (short)SERVER_PORT);

	client[0].fd = listenfd;
	client[0].events = POLLRDNORM;
	maxi = 0;

	while(1)
	{
		retNum = poll(client, maxi+1, -1);

		if (client[0].revents & POLLRDNORM)
		{
			CSlen = sizeof(ClientAddr);
			cfd = Accept(listenfd, (struct sockaddr *)&ClientAddr, &CSlen);
			printf("New client connected Address : %s, Port: %d\n", 
			    	inet_ntop(AF_INET, &ClientAddr.sin_addr, 
					str, sizeof(str)), ntohs(ClientAddr.sin_port));

			for (i = 1; i < MAX_LISTEN; ++i)
			{
				if (client[i].fd < 0)
				{
					client[i].fd = cfd;
					break;
				}
			}

			if (i == MAX_LISTEN)
			{
				Perr_exit("too many client\n");
			}

			client[i].events = POLLRDNORM;

			if (i > maxi)
			{
				maxi = i;
			}

			if (--retNum == 0)
			{
				continue;
			}
		}

		for (i = 1; i <= maxi; ++i)
		{
			if (client[i].fd < 0)
			{
				continue;
			}

			if (client[i].revents & (POLLRDNORM | POLLERR))
			{
				printf("Client %dth received message!\n", i);
				readLen = Read(client[i].fd, readBuf, BUFSIZ);

				if (readLen == 0)
				{
					printf("Client %dth has been closed!\n", i);
					Close(client[i].fd);
					client[i].fd = -1;
				}
				else if (readLen < 0)
				{
					if (errno == ECONNRESET)
					{
						printf("Client %dth aborted connection!\n", i);
						Close(client[i].fd);
						client[i].fd = -1;
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

					Write(client[i].fd, readBuf, readLen);
				}

				if (--retNum == 0)
				{
					continue;
				}
			}
		}
	}

	Close(listenfd);

	return 0;
}