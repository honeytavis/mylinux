#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/epoll.h>

#define NUM 10

int main(int argc, char** argv)
{
    if (argc != 3) {
        perror("error args!\n"); 
        exit(0); 
    }

    int socket_fd = socket(AF_INET, SOCK_STREAM, 0); 
    if (socket_fd < 0) {
        perror("socket()"); 
        exit(EXIT_FAILURE); 
    }

    struct sockaddr_in local; 
    bzero(&local, sizeof(struct sockaddr_in)); 
    local.sin_family = AF_INET; 
    local.sin_addr.s_addr = inet_addr(argv[1]); 
    local.sin_port = htons(atoi(argv[2])); 
    int ret = connect(socket_fd, (struct sockaddr*)&local, \
        sizeof(struct sockaddr)); 
    if (-1 == ret) {
        perror("connect()"); 
        exit(EXIT_FAILURE); 
    }

    int epoll_fd = epoll_create(1); 
    if (-1 == epoll_fd) {
        perror("epoll_creat()"); 
        exit(EXIT_FAILURE);
    }

    struct epoll_event ev, evs[2]; 
    ev.events = EPOLLIN; 
    ev.data.fd = socket_fd; 
    ret = epoll_ctl(epoll_fd, EPOLL_CTL_ADD, socket_fd, &ev); 
    if (-1 == ret) {
        perror("epoll_ctl()"); 
        exit(EXIT_FAILURE); 
    }

    ev.events = EPOLLIN; 
    ev.data.fd = 0; 
    ret = epoll_ctl(epoll_fd, EPOLL_CTL_ADD, 0, &ev); 
    if (-1 == ret) {
        perror("epoll_ctl()"); 
        exit(EXIT_FAILURE); 
    }
    
    char buf[128]; 
    int i; 
    for (;;) {
        bzero(evs, sizeof(evs)); 
        ret = epoll_wait(epoll_fd, evs, 3, -1); 
        if (ret > 0) {
            for (i = 0; i < ret; ++i) {
                if (evs[i].data.fd == 0 && evs[i].events == EPOLLIN) {
                    bzero(buf, sizeof(buf)); 
                    read(0, buf, sizeof(buf)); 
                    int size = send(socket_fd, buf, strlen(buf)-1, 0); 
                    if (-1 == size) {
                        perror("send()"); 
                        exit(EXIT_FAILURE); 
                    } 
                }

                if (evs[i].data.fd == socket_fd && evs[i].events == EPOLLIN) {
                    bzero(buf, sizeof(buf)); 
                    int size = recv(socket_fd, buf, sizeof(buf), 0); 
                    if (size > 0) {
                        printf("%s\n", buf); 
                    } else {
                        printf("recv return is %d\n", size); 
                    }
                }
            }
            
        }
    }
    close(socket_fd); 

    return 0; 
}
