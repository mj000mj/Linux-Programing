#include "wrap.h"

#define MAX_EVENTS  1024
#define BUFLEN  128
#define SERVER_PORT 9527


void senddata(int fd, int events, void *arg);
void recvdata(int fd, int events, void *arg);

struct myevent_s
{
	int fd;                                                                           //要监听的事件描述符
	int events;                                                                   //要监听的事件类型
	void *arg;                                                                     //范型参数，本结构体
	void (* call_back)(int fd, int events, void *arg);  //监听事件触发的回调函数
	int status;                                                                   //是否在监听
	char buf[BUFLEN];                                                         //监听事件的缓存
	int len;                                                                         //监听事件的数据长度
	long last_active;                                                        //每次加入监听的当前时间
};

int g_efd;   //全局监听红黑树句柄
struct myevent_s g_events[MAX_EVENTS + 1];

/*将结构体myevent_s成员变量初始化*/
void eventset(struct myevent_s *ev, int fd, void (*call_back)(int, int, void *), void *arg)
{
	ev->fd = fd;
	ev->call_back = call_back;
	ev->events = 0;
	ev->arg = arg;
	ev->status = 0;
	//memset(ev->buf, 0, sizeof(ev->buf));
	//ev->len = 0;
	ev->last_active = time(NULL);

	return;
}

/*向红黑树中增加一个监听事件*/
void eventadd(int efd, int events, struct myevent_s *ev)
{
	struct epoll_event epv = {0, {0}};
	int op;
	epv.data.ptr = ev;
	epv.events = ev->events = events;

	if (ev->status == 1)
	{
		op = EPOLL_CTL_MOD;
	}
	else
	{
		op = EPOLL_CTL_ADD;
		ev->status = 1;
	}

	if (epoll_ctl(efd, op, ev->fd, &epv) < 0)
	{
		printf("event add failed [fd = %d], events[%d]\n", ev->fd, events);
	}
	else
	{
		printf("event add OK [fd = %d], op=%d, events[%d]\n", ev->fd, op, events);
	}

	return;
}

/*从红黑树中删除一个监听事件*/
void eventdel(int efd, struct myevent_s *ev)
{
	struct epoll_event epv = {0, {0}};

	if (ev->status != 1)
	{
		return;
	}

	epv.data.ptr = ev;
	ev->status = 0;

	epoll_ctl(efd, EPOLL_CTL_DEL, ev->fd, &epv);

	return;
}


/*新客户端连接回调函数*/
void acceptconn(int lfd, int events, void *arg)
{
	struct sockaddr_in cin;
	socklen_t len = sizeof(cin);
	int cfd, i;

	if (cfd = accept(lfd, (struct sockaddr *)&cin, &len) == -1)
	{
		printf("%s: accept, %s\n", __func__, strerror(errno));
	}

	do
	{
		for (i = 0; i < MAX_EVENTS; i++)
		{
			if (g_events[i].status == 0)
			{
				break;
			}
		}

		if (i == MAX_EVENTS)
		{
			printf("%s: max connect limit[%d]\n", __func__, MAX_EVENTS);
			break;
		}

		int flag = 0;
		if ((flag = fcntl(cfd, F_SETFL, O_NONBLOCK)) < 0)
		{
			printf("%s: fcntl nonblocking failed, %s\n", __func__, strerror(errno));
			break;
		}

		/*给新的cfd初始化一个监听结构体myevent_s, 设置回调函数为recvdata*/
		eventset(&g_events[i], cfd, recvdata, &g_events[i]);
		/*将新的cfd添加到监听红黑树g_efd中*/
		eventadd(g_efd, EPOLLIN, &g_events[i]);

	}while(0);

	printf("new connect [%s:%d][time:%ld], pos[%d]\n", 
		          inet_ntoa(cin.sin_addr), ntohs(cin.sin_port), g_events[i].last_active, i);
	return;
}

void recvdata(int fd, int events, void *arg)
{
    struct myevent_s *ev = (struct myevent_s *)arg;
    int len;

    len = recv(fd, ev->buf, sizeof(ev->buf), 0);     //读文件描述符, 数据存入myevent_s成员buf中

    eventdel(g_efd, ev);    //将该节点从红黑树上摘除

    if (len > 0) {

        ev->len = len;
        ev->buf[len] = '\0';     //手动添加字符串结束标记
        printf("C[%d]:%s\n", fd, ev->buf);

        eventset(ev, fd, senddata, ev);                     //设置该 fd 对应的回调函数为 senddata
        eventadd(g_efd, EPOLLOUT, ev);                      //将fd加入红黑树g_efd中,监听其写事件

    } else if (len == 0) {
        close(ev->fd);
        /* ev-g_events 地址相减得到偏移元素位置 */
        printf("[fd=%d] pos[%ld], closed\n", fd, ev-g_events);
    } else {
        close(ev->fd);
        printf("recv[fd=%d] error[%d]:%s\n", fd, errno, strerror(errno));
    }
    return;
}

void senddata(int fd, int events, void *arg)
{
    struct myevent_s *ev = (struct myevent_s *)arg;
    int len;

    len = send(fd, ev->buf, ev->len, 0);   //直接将数据 回写给客户端。未作处理

    eventdel(g_efd, ev);     //从红黑树g_efd中移除

    if (len > 0) 
    {
        printf("send[fd=%d], [%d]%s\n", fd, len, ev->buf);
        eventset(ev, fd, recvdata, ev);  //将该fd的 回调函数改为 recvdata
        eventadd(g_efd, EPOLLIN, ev);   //从新添加到红黑树上， 设为监听读事件
    } 
    else 
    {
        close(ev->fd);  //关闭链接
        printf("send[fd=%d] error %s\n", fd, strerror(errno));
    }

    return;
}

void initListenSocket(int efd, short port)
{
	struct sockaddr_in sin;

	int lfd = Socket(AF_INET, SOCK_STREAM, 0);
	fcntl(lfd, F_SETFL, O_NONBLOCK);

	memset(&sin, 0, sizeof(sin));

	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = INADDR_ANY;
	sin.sin_port = htons(port);

	bind(lfd, (struct sockaddr *)&sin, sizeof(sin));

	listen(lfd, MAX_LISTEN);

	eventset(&g_events[MAX_EVENTS], lfd, acceptconn, &g_events[MAX_EVENTS]);
	eventadd(efd, EPOLLIN, &g_events[MAX_EVENTS]);

	return;
}

int main(int argc, char const *argv[])
{
	unsigned short port = SERVER_PORT;

	if (argc == 2)
	{
		port = atoi(argv[1]);
	}

	//创建监听红黑树
	g_efd = epoll_create(MAX_EVENTS + 1);
	if (g_efd <= 0)
	{
		printf("create g_efd in %s err %s\n", __func__, strerror(errno));
	}

	initListenSocket(g_efd, port);

	struct epoll_event events[MAX_EVENTS+1];
	printf("server running:port[%d]\n", port);

	int checkpos = 0, i;

	while(1)
	{
		/*检测1min无响应的客户端进行断开连接操作*/
		long now = time(NULL);                          //当前时间
        for (i = 0; i < 100; i++, checkpos++) 
        {        
            //一次循环检测100个。 使用checkpos控制检测对象
            if (checkpos == MAX_EVENTS)
            {
                checkpos = 0;
            }
            if (g_events[checkpos].status != 1)  //不在红黑树 g_efd 上
                continue;

            long duration = now - g_events[checkpos].last_active; //客户端不活跃的世间

            if (duration >= 60) 
            {
                close(g_events[checkpos].fd);  //关闭与该客户端链接
                printf("[fd=%d] timeout\n", g_events[checkpos].fd);
                eventdel(g_efd, &g_events[checkpos]);  //将该客户端 从红黑树 g_efd移除
            }
        }

        int nfd = epoll_wait(g_efd, events, MAX_EVENTS+1, 1000);
        if (nfd < 0)
        {
        	printf("epoll_wait error, exit\n");
            break;
        }

        for (i = 0; i < nfd; ++i)
        {
        	struct myevent_s *ev = (struct myevent_s *)events[i].data.ptr;
        	if ((events[i].events & EPOLLIN) && (ev->events & EPOLLIN))
        	{
        		ev->call_back(ev->fd, events[i].events, ev->arg);
        	}
        	if ((events[i].events & EPOLLOUT) && (ev->events & EPOLLOUT))
        	{
        		ev->call_back(ev->fd, events[i].events, ev->arg);
        	}
        }
	}
	return 0;
}
