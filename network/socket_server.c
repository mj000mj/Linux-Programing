/*此程序为socket套接字服务器端，多进程并发服务器*/
#include "wrap.h"
/*设置端口号*/
#define TCP_PORT  9527
/*子进程信号回调函数，用户回收子进程*/
void do_sigchild(int num)
{
    while(waitpid(0, NULL, WNOHANG) > 0)
        ;
}

int main(int args, char *argv[])
{
    int sfd, cfd, ret, i;
    pid_t pid;
    struct sockaddr_in ser_addr, cli_addr;
    socklen_t cli_addrlen;
    ssize_t rlen;
    char rbuf[BUFSIZ];
    char Client_IP[BUFSIZ];
    struct sigaction newact;

    newact.sa_handler = do_sigchild;
    sigemptyset(&newact.sa_mask);
    newact.sa_flags = 0;
    sigaction(SIGCHLD, &newact, NULL);


    memset(rbuf, 0, BUFSIZ);
    int opt = 1;
    setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    bzero(&ser_addr, sizeof(ser_addr));  
    cli_addrlen = sizeof(cli_addr);
    ser_addr.sin_family = AF_INET;
    ser_addr.sin_port = htons(TCP_PORT);
    ser_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    
    sfd = Socket(AF_INET, SOCK_STREAM, 0);

    Bind(sfd, (struct sockaddr *)&ser_addr, sizeof(ser_addr));
    Listen(sfd, 20);

    while(1)
    {
        cli_addrlen = sizeof(cli_addr);
        cfd = Accept(sfd, (struct sockaddr *)&cli_addr, &cli_addrlen);
        printf("-------------------------%d\n", cfd);
        pid = fork();

        if (pid == 0)
        {
            Close(sfd);
            printf("fork close socket of child success\n");
            while(1)
            {
                rlen = Read(cfd, rbuf, BUFSIZ);
                if (rlen == 0)
                {
                    printf("the other side has been closed.\n");
                    break;
                }
                printf("Client Address: %s  Port: %d\n", inet_ntop(AF_INET, &cli_addr.sin_addr, Client_IP, sizeof(Client_IP)),
                                                                                        ntohs(cli_addr.sin_port));

                Write(STDOUT_FILENO, rbuf, rlen);
                for(i = 0; i < rlen; i++)
                    rbuf[i] = toupper(rbuf[i]);
                Write(cfd, rbuf, rlen);
            }
            Close(cfd);
            return 0;
        }
        else if (pid > 0)
        {
            Close(cfd);
        }
        else
        {
            Perr_exit("fork error");
        }
    }
    return 0;
}
