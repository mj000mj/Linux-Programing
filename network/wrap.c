#include "wrap.h"

void Perr_exit(char *err)
{
	perror(err);
	exit(EXIT_FAILURE);
}

int Socket(int domain, int type, int protocol)
{
	int fd;
	fd = socket(domain, type, protocol);

	if (fd < 0)
	{
		Perr_exit("socket error");
	}
}

int Bind(int sockfd, struct  sockaddr  *my_addr,  socklen_t addrlen)
{
	int ret;
	ret = bind(sockfd, my_addr,  addrlen);

	if (ret < 0)
	{
		Perr_exit("bind error");
	}

	return ret;
}

int Connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen)
{
	int ret;
	ret = connect(sockfd, addr, addrlen);
	if (ret < 0)
	{
		Perr_exit("connect error");
	}
	return ret;
}

int Accept(int   s,   struct  sockaddr  *addr,  socklen_t *addrlen)
{
	int ret;
again:
	if ((ret = accept(s, addr, addrlen)) < 0) {
		if ((errno == ECONNABORTED) || (errno == EINTR))
			goto again;
		else
			Perr_exit("accept error");
	}
	return ret;
}

int Listen(int s, int backlog)
{
	int ret;
	ret = listen(s, backlog);
	if (ret < 0)
	{
		Perr_exit("listen error");
	}
	return ret;
}

int Close(int fd)
{
	int ret;
	ret = close(fd);
	if (ret < 0)
	{
		Perr_exit("close error");
	}
	return ret;
}

ssize_t Read(int fd, void *ptr, size_t nbytes)
{
	ssize_t len;

again:
	len = read(fd, ptr, nbytes);
	if (len == -1)
	{
		if (errno == EINTR)
			goto again;
		else
			return -1;
	}

	return len;
}
ssize_t Write(int fd, const void *ptr, size_t nbytes)
{
	ssize_t len;

again:
	len = write(fd, ptr, nbytes);
	if (len == -1) {
		if (errno == EINTR)
			goto again;
		else
			return -1;
	}
	return len;
}

ssize_t Readn(int fd, void *vptr, size_t n)
{
	;
}
ssize_t Writen(int fd, const void *vptr, size_t n)
{
	;
}
ssize_t my_read(int fd, char *ptr)
{
	;
}
ssize_t Readline(int fd, void *vptr, size_t maxlen)
{
	;
}
