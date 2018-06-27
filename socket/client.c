#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>

int main(int argc char *argv[])
{
    int sd, ret;
    struct sockaddr_in server_addr;
    sd = socket(AF_INET, SOCK_STREAM, 0);

    if (sd == -1) {
        perror("create socket error");
        exit(-1);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(8888);
    ret = inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr);
    if(ret <= 0) {
        if (ret == 0) {
            printf("Not in presentation format\n");
        } else {
            perror("inet_pton");
        }
        exit(-1);
    }

    if (connect(sd, (struct sockaddr *)&server_addr, sizeof(server_addr)) != 0) {
        perror("connect error");
        exit(-1);
    }

    close(sd);
}
