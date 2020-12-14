/*此程序为socket套接字服务器端，多路IO并发服务器*/
#include "wrap.h"
#include <sys/select.h>
/*设置端口号*/
#define TCP_PORT  9527

int main(int args, char *argv[])
{
    int listenfd, cfd, maxfd, ret, i, j;
    pid_t pid;
    struct sockaddr_in ser_addr, cli_addr;
    socklen_t cli_addrlen;
    ssize_t rlen;
    char rbuf[BUFSIZ];
    char Client_IP[BUFSIZ];

    memset(rbuf, 0, BUFSIZ);
    listenfd = Socket(AF_INET, SOCK_STREAM, 0);

    int opt = 1;
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    bzero(&ser_addr, sizeof(ser_addr));  
    cli_addrlen = sizeof(cli_addr);
    ser_addr.sin_family = AF_INET;
    ser_addr.sin_port = htons(TCP_PORT);
    ser_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    
    Bind(listenfd, (struct sockaddr *)&ser_addr, sizeof(ser_addr));
    Listen(listenfd, 128);
    maxfd = listenfd;

    fd_set rset, allset;  //定义读集和，备份集和allset
    FD_ZERO(&allset);  //清空监听集和
    FD_SET(listenfd, &allset);  //将待监听fd添加到监听集和中

    while(1)
    {
        rset = allset;  //备份
        ret = select(maxfd+1, &rset, NULL, NULL, NULL);  //使用select监听
        if (ret < 0)
        {
            Perr_exit("select error");
        }

        if (FD_ISSET(listenfd, &rset))    //listenfd满足监听的读事件
        {
            cli_addrlen = sizeof(cli_addr);
            cfd = Accept(listenfd, (struct sockaddr *)&cli_addr, &cli_addrlen);  //建立连接 ---不会阻塞

            FD_SET(cfd, &allset);

            if (maxfd < cfd)  //修改maxfd
            {
                maxfd = cfd;
            }
            /*说明select只返回一个listenfd，后续无需执行*/
            if (ret == 1) 
            {
                continue;
            }
        }

        for (i = listenfd+1; i < maxfd+1; ++i)  //处理满足读事件的fd
        {
            if (FD_ISSET(i, &rset))  //找到满足读事件的那个fd
            {
                rlen = Read(i, rbuf, sizeof(rbuf));
                if (rlen == 0)  //检测到客户端已经关闭
                {
                    Close(i);
                    FD_CLR(i, &allset);  //取消对关闭客户端的监听
                }

                for (j = 0; j < rlen; ++j)
                {
                    rbuf[j] = toupper(rbuf[j]);
                }

                Write(i, rbuf, rlen);
                Write(STDOUT_FILENO, rbuf, rlen);
            }
        }
    }

    Close(listenfd);
   
    return 0;
}
