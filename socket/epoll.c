#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <limits.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <signal.h>

#define MAX_EPOLL_EVENTS 512
#define EPOLL_LT 1
#define EPOLL_ET 0

//设置系统文件描述符
int init_server(char *ip, int port)
{
    int sd, ret;
    struct sockaddr_in my_addr, client_addr;
    socklen_t len = (socklen_t)sizeof(struct sockaddr_in);
    sd = socket(PF_INET, SOCK_STREAM, 0);

    if (sd == -1) {
        perror("create socket error");
        exit(-1);
    }

    bzero(&my_addr, sizeof(struct sockaddr_in));
    my_addr.sin_family = AF_INET;
    my_addr.sin_port = htons(port);

    if (ip == NULL) {
        my_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    } else {
        ret = inet_pton(AF_INET, ip, &my_addr.sin_addr);
        if(ret <= 0) {
            if (ret == 0) {
                printf("Not in presentation format\n");
            } else {
                perror("inet_pton");
            }
            exit(-1);
        }
    }

    if (bind(sd, (struct sockaddr *)&my_addr, len) != 0) {
        perror("bind socket error");
        exit(-1);
    }

    if (listen(sd, 512) != 0) {
        perror("listen socket error");
        exit(-1);
    }
    printf("server listen on %s:%d\n", ip, port);
    return sd;
}

int read_fd(int fd)
{
    int nread;
    char buf[5];
    while (1) {
        nread = read(fd, buf, 5);
        if (nread < 0) {
            if (errno == EINTR) {
                continue;
            } else if (errno == EAGAIN || errno == EWOULDBLOCK) {
                printf("finish read\n");
                break;
            }
            perror("read error");
            return -1;
        } else if (nread == 0) {
            printf("client closed\n");
            return -1;
        }
        buf[nread] = '\0';
        printf("read %d bytes data: %s\n", nread, buf);
    }
    return 0;
}

int set_nonblock(int fd)
{
    int old_flags = fcntl(fd, F_GETFL);
    fcntl(fd, F_SETFL, old_flags | O_NONBLOCK);
    return old_flags;
}

void add_epoll_read_event(int epoll_fd, int fd, int epoll_type, int block_type)
{
    struct epoll_event ep_event;
    ep_event.data.fd = fd;
    ep_event.events = EPOLLIN;

    if (epoll_type == EPOLL_ET) {
        ep_event.events |= EPOLLET;
    }

    if (block_type == O_NONBLOCK) {
        set_nonblock(fd);
    }

    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &ep_event);
}

void del_epoll_event(int epoll_fd, int fd)
{
    struct epoll_event ep_event;
    ep_event.events = 0;
    epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, &ep_event);
}


int main(int argc, char *argv[])
{
    signal(SIGPIPE, SIG_IGN);
    struct epoll_event events[MAX_EPOLL_EVENTS];
    struct sockaddr_in client_addr;
    socklen_t len = (socklen_t)sizeof(struct sockaddr_in);
    int listen_fd, client_fd, epoll_fd, nevents, event_fd, ret, i;
    int is_reuse = 1;
    listen_fd = init_server(NULL, 8888);
    if (setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, (const char*)&is_reuse, sizeof(is_reuse)) != 0) {
        perror("setsockopt error");
        exit(-1);
    }

    if ((epoll_fd = epoll_create(512)) == -1) {
        perror("epoll_create error");
        exit(-1);
    }

    add_epoll_read_event(epoll_fd, listen_fd, EPOLL_LT, O_NONBLOCK);

    while (1) {
        nevents = epoll_wait(epoll_fd, events, MAX_EPOLL_EVENTS, -1);
        if (nevents == -1) {
            if (errno == EINTR) {
                continue;
            }
            printf("epoll error");
            exit(-1);
        }

        for (i = 0; i < nevents; i++) {
            event_fd = events[i].data.fd;

            if (event_fd == listen_fd) {
                char buf[INET_ADDRSTRLEN];
                len = (socklen_t)sizeof(struct sockaddr_in);
                bzero(&client_addr, len);
                client_fd = accept(listen_fd, (struct sockaddr *)&client_addr, &len);
                if (client_fd == -1) {
                    if (errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK) {
                        continue;
                    }
                    perror("accept error");
                    close(epoll_fd);
                    exit(-1);
                }

                if (inet_ntop(AF_INET, &client_addr.sin_addr, buf, INET_ADDRSTRLEN) == NULL) {
                    perror("inet_ntop error");
                    exit(-1);
                }
                add_epoll_read_event(epoll_fd, client_fd, EPOLL_ET, O_NONBLOCK);
                continue;
            }

            if (events[i].events & EPOLLIN) {
                ret = read_fd(event_fd);
                if (ret == -1) {
                    del_epoll_event(epoll_fd, event_fd);
                    close(event_fd);
                }
            }
        }
    }

    close(epoll_fd);
}
