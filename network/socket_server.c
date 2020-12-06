#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <errno.h>
#include <arpa/inet.h>

#define TCP_PORT  9527

void sys_err(char *err)
{
    perror(err);
    exit(EXIT_FAILURE);
}

int main(int args, char *argv[])
{
    int sfd, cfd, ret, i;
    struct sockaddr_in ser_addr, cli_addr;
    socklen_t cli_addrlen;
    ssize_t rlen;
    char rbuf[BUFSIZ];
    char Client_IP[BUFSIZ];
    memset(rbuf, 0, BUFSIZ);

    ser_addr.sin_family = AF_INET;
    ser_addr.sin_port = htons(TCP_PORT);
    ser_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    
    sfd = socket(AF_INET, SOCK_STREAM, 0);
  
    if(sfd == -1)
    {
        sys_err("socket error");
    }

    ret = bind(sfd, (struct sockaddr *)&ser_addr, sizeof(ser_addr));
    if(ret == -1)
    {
        sys_err("bind error");
    }

    ret = listen(sfd, 5);
    if(ret == -1)
    {
        sys_err("listen error");
    }

    cli_addrlen = sizeof(cli_addr);
    cfd = accept(sfd, (struct sockaddr *)&cli_addr, &cli_addrlen);
    if(cfd == -1)
    {
        sys_err("accept error");   
    }

    printf("Client Address: %s  Port: %d\n", inet_ntop(AF_INET, &cli_addr.sin_addr.s_addr, Client_IP, sizeof(Client_IP)),
                                             ntohs(cli_addr.sin_port));
    
    while(1)
    {
        rlen = read(cfd, rbuf, BUFSIZ);

        if(rlen > 0)
        {
            write(STDOUT_FILENO, rbuf, rlen);
            for(i = 0; i < rlen; i++)
                rbuf[i] = toupper(rbuf[i]);
            write(cfd, rbuf, rlen);
        }
        else if(rlen == -1)
        {
           sys_err("read error");
        }
    }

    close(sfd);
    close(cfd);
    return 0;
}
