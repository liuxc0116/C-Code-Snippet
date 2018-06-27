#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <poll.h>
#include <limits.h>
#include <signal.h>

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
    char buf[1024];
    bzero(buf, 1024);
    nread = read(fd, buf, 1024);
    if (nread < 0) {
        if (errno == EINTR) {
            return 0;
        }
        perror("read error");
        return -1;
    } else if (nread == 0) {
        printf("client closed\n");
        return -1;
    }
    printf("read %d bytes data: %s", nread, buf);
    return 0;
}

int main(int argc, char *argv[])
{
    signal(SIGPIPE, SIG_IGN);

    struct sockaddr_in client_addr;
    socklen_t len = (socklen_t)sizeof(struct sockaddr_in);
    struct pollfd clients[OPEN_MAX];
    int listen_fd, client_fd, max_fd, max_index, nready, i, ret;
    int is_reuse = 1;
    listen_fd = init_server(NULL, 8888);
    if (setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, (const char*)&is_reuse, sizeof(is_reuse)) != 0) {
        perror("setsockopt error");
        exit(-1);
    }

    clients[0].fd = listen_fd;
    clients[0].events = POLLRDNORM;


    for (i = 1; i < OPEN_MAX; i++) {
        clients[i].fd = -1;
    }

    max_index = 0;
    while (1) {
        nready = poll(clients, max_index + 1, -1);
        printf("%d socket ready to read\n", nready);

        if (nready < 0) {
            if (errno == EINTR) {
                continue;
            }
            perror("select error");
            exit(-1);
        }

        if (clients[0].revents & POLLRDNORM) {
            char buf[INET_ADDRSTRLEN];
            len = (socklen_t)sizeof(struct sockaddr_in);
            bzero(&client_addr, len);
            client_fd = accept(listen_fd, (struct sockaddr *)&client_addr, &len);
            if (client_fd == -1) {
                if (errno == EINTR) {
                    continue;
                }
                perror("accept error");
                exit(-1);
            }

            if (inet_ntop(AF_INET, &client_addr.sin_addr, buf, INET_ADDRSTRLEN) == NULL) {
                perror("inet_ntop error");
                exit(-1);
            }

            printf("accept a client(%d), ip(%s), port(%d)\n", client_fd, buf, ntohs(client_addr.sin_port));
            for (i = 0; i < FD_SETSIZE; i++) {
                if (clients[i].fd != -1) {
                    continue;
                }
                break;
            }

            if (i == OPEN_MAX) {
                printf("too many clients, close this one\n");
                close(client_fd);
            } else {
                clients[i].fd = client_fd;
                clients[i].events = POLLRDNORM;
                if (i > max_index) {
                    max_index = i;
                }
            }

            if (--nready <= 0) {
                continue;
            }
        }

        for (i = 0; i <= max_index; i++) {
            if ((client_fd = clients[i].fd) < 0) {
                continue;
            }

            if (clients[i].revents & (POLLRDNORM | POLLERR)) {
                ret = read_fd(client_fd);
                if (ret != 0) {
                    close(client_fd);
                    clients[i].fd = -1;
                }

                if(--nready <= 0) {
                    break;
                }
            }
        }
    }

    return 0;
}
