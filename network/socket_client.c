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
    int cfd, ret, i;
    char rbuf[BUFSIZ];
    ssize_t rlen;
    struct sockaddr_in ser_addr;

    memset(rbuf, 0, BUFSIZ);
    cfd = socket(AF_INET, SOCK_STREAM, 0);
    if(cfd == -1)
    {
        sys_err("socket error");
    }
    
    ser_addr.sin_family = AF_INET;
    ser_addr.sin_port = htons(TCP_PORT);
    inet_pton(AF_INET, "192.168.106.129", &ser_addr.sin_addr);

    ret = connect(cfd, (struct sockaddr *)&ser_addr, sizeof(ser_addr));
    if(ret == -1)
    {
        sys_err("connect error");
    }

    while(1)
    {
        write(cfd, "hello\n", sizeof("hello\n"));

        sleep(2);

        rlen = read(cfd, rbuf, BUFSIZ);

        write(STDOUT_FILENO, rbuf, rlen);
    }
    close(cfd);
    return 0;
}
