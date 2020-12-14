#include "wrap.h"

/*定义端口号*/
#define TCP_PORT 8000
/*定义最大监听创建socket数量*/
#define MAX_LISTEN 10

/*客户端socket信息结构体*/
struct client_info
{
	int fd;
	struct sockaddr_in c_addr;
};

/*新客户端线程*/
void *NewClient(void *arg)
{
	struct client_info *CliInfo;
	char clientIP[128];
	char readBuf[BUFSIZ];
	ssize_t readLen;
	CliInfo = (struct client_info *)arg;
	int cfd = CliInfo->fd;
	int i;

	printf("New client connected Address : %s, Port: %d\n", 
		    inet_ntop(AF_INET, &CliInfo->c_addr.sin_addr, 
			clientIP, sizeof(clientIP)), ntohs(CliInfo->c_addr.sin_port));

	while(1)
	{
		readLen = 0;
		readLen = Read(cfd, readBuf, BUFSIZ);
		if (readLen == 0)
		{
			printf("The other side is closed!\n");
			break;
		}

		Write(STDOUT_FILENO, readBuf, readLen);
		for (i = 0; i < readLen; ++i)
		{
			readBuf[i] = toupper(readBuf[i]);
		}
		Write(cfd, readBuf, readLen);
	}

	close(cfd);
}

int main(int argc, char const *argv[])
{
	/**/
	int sfd,listenfd;
	struct sockaddr_in ServerAddr, ClientAddr;
	socklen_t CSlen;
	int ret;
	int opt;
	pthread_t pthrd;
	struct client_info *clientInfo;

	bzero(&ServerAddr, sizeof(ServerAddr));
	bzero(&ClientAddr, sizeof(ClientAddr));

	sfd = Socket(AF_INET, SOCK_STREAM, 0);

	ServerAddr.sin_family = AF_INET;
	ServerAddr.sin_port = htons(TCP_PORT);
	ServerAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	opt = 1;
	setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, (const void *)&opt, sizeof(opt));

	Bind(sfd, (struct sockaddr *)&ServerAddr, sizeof(ServerAddr));

	Listen(sfd, MAX_LISTEN);

	while(1)
	{
		CSlen = sizeof(ClientAddr);
		listenfd = Accept(sfd, (struct sockaddr *)&ClientAddr, &CSlen);
		printf("---------------------------%d\n", listenfd);

		clientInfo = malloc(sizeof(struct client_info));
		clientInfo->fd = listenfd;
		clientInfo->c_addr = ClientAddr;

		ret = pthread_create(&pthrd, NULL, NewClient, (void *)clientInfo);

		if (ret == -1)
		{
			perror("pthread create error");
			continue;
		}

		ret = pthread_detach(pthrd);
		if (ret == -1)
		{
			Perr_exit("pthread detack error");
		}

		close(sfd);

	}
	
	return 0;
}
