#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <sys/select.h>
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

int max(int a, int b)
{
    return a > b ? a : b;
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
    fd_set read_fdset, all_fdset;
    int listen_fd, client_fd, max_fd, max_index, nready, i, ret;
    int is_reuse = 1;
    int clients[FD_SETSIZE];
    listen_fd = init_server(NULL, 8888);
    if (setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, (const char*)&is_reuse, sizeof(is_reuse)) != 0) {
        perror("setsockopt error");
        exit(-1);
    }

    FD_ZERO(&all_fdset);
    FD_SET(listen_fd, &all_fdset);

    for (i = 0; i < FD_SETSIZE; i++) {
        clients[i] = -1;
    }

    max_fd = listen_fd;
    max_index = -1;
    while (1) {
        read_fdset = all_fdset;
        nready = select(max_fd + 1, &read_fdset, NULL, NULL, NULL);
        printf("%d socket ready to read\n", nready);

        if (nready < 0) {
            if (errno == EINTR) {
                continue;
            }
            perror("select error");
            exit(-1);
        }

        if (FD_ISSET(listen_fd, &read_fdset)) {
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
                if (clients[i] != -1) {
                    continue;
                }
                break;
            }

            if (i == FD_SETSIZE) {
                printf("too many clients, close this one\n");
                close(client_fd);
            } else {
                FD_SET(client_fd, &all_fdset);
                clients[i] = client_fd;
                max_fd = max(max_fd, client_fd);
                if (i > max_index) {
                    max_index = i;
                }
            }

            if (--nready <= 0) {
                continue;
            }
        }

        for (i = 0; i <= max_index; i++) {
            if ((client_fd = clients[i]) < 0) {
                continue;
            }

            if (FD_ISSET(client_fd, &read_fdset)) {
                ret = read_fd(client_fd);
                if (ret != 0) {
                    close(client_fd);
                    FD_CLR(client_fd, &all_fdset);
                    clients[i] = -1;
                }

                if(--nready <= 0) {
                    break;
                }
            }
        }
    }

    return 0;
}

