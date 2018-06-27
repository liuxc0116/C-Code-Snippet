#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
/*

NOTES
       To accept connections, the following steps are performed:

           1.  A socket is created with socket(2).

           2.  The socket is bound to a local address using bind(2), so that other sockets may be connect(2)ed to it.

           3.  A willingness to accept incoming connections and a queue limit for incoming connections are specified with listen().

           4.  Connections are accepted with accept(2).

       POSIX.1 does not require the inclusion of <sys/types.h>, and this header file is not required on Linux.  However,  some  historical
       (BSD) implementations required this header file, and portable applications are probably wise to include it.

       The  behavior  of  the  backlog  argument  on TCP sockets changed with Linux 2.2.  Now it specifies the queue length for completely
       established sockets waiting to be accepted, instead of the number of incomplete connection requests.  The  maximum  length  of  the
       queue for incomplete sockets can be set using /proc/sys/net/ipv4/tcp_max_syn_backlog.  When syncookies are enabled there is no log‐
       ical maximum length and this setting is ignored.  See tcp(7) for more information.

       If the backlog argument is greater than the value in /proc/sys/net/core/somaxconn, then it is silently truncated to that value; the
       default value in this file is 128.  In kernels before 2.4.25, this limit was a hard coded value, SOMAXCONN, with the value 128.
*/

/*
struct sockaddr {
    unsigned short sa_family;  //地址族, AF_xxx
    char sa_data[14];   //14字节的协议地址
};

struct sockaddr_in {
    short int sin_family;  //地址族
    unsigned short int sin_port;  //端口号
    struct in_addr sin_addr;  //Internet地址
    unsigned char sin_zero[8];  //与struct sockaddr一样的长度
};

int inet_aton(const char *cp, struct in_addr *inp);
char *inet_ntoa(struct in_addr in);
in_addr_t inet_addr(const char *cp);//最好不要使用，存在无妨装换255.255.255.255的情况
in_addr_t inet_network(const char *cp);
struct in_addr inet_makeaddr(in_addr_t net, in_addr_t host);
in_addr_t inet_lnaof(struct in_addr in);
in_addr_t inet_netof(struct in_addr in);

//推荐使用下面两个函数
int inet_pton(int af, const char *src, void *dst);
const char *inet_ntop(int af, const void *src, char *dst, socklen_t size);
*/

//设置系统文件描述符
int init_server(char *ip, int port)
{
    int sd, cd, ret;
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
    //my_addr.sin_addr.s_addr = inet_addr(ip);
    /*if (inet_aton(ip, &my_addr.sin_addr) == 0) {
        perror("minet_aton error");
        return -1;
    }*/

    ret = inet_pton(AF_INET, ip, &my_addr.sin_addr);
    if(ret <= 0) {
        if (ret == 0) {
            printf("Not in presentation format\n");
        } else {
            perror("inet_pton");
        }
        exit(-1);
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

    while (1) {
        char buf[INET_ADDRSTRLEN];
        len = (socklen_t)sizeof(struct sockaddr_in);
        bzero(&client_addr, len);
        cd = accept(sd, (struct sockaddr *)&client_addr, &len);
        if (cd == -1) {
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
        //inet_ntoa(client_addr.sin_addr)
        printf("accept a client(%d), ip(%s), port(%d)\n", cd, buf, ntohs(client_addr.sin_port));
        close(cd);
    }
}

int main(int argc char *argv[])
{
    init_server("127.0.0.1", 8888);
    return 0;
}
