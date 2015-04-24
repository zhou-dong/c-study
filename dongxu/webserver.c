/*
 * Http webserver
 *
 * 1. C Language
 * 2. Multithread
 *
 * gcc -o webserver webserver.c
 * ./webserver
 *
 */

#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <errno.h>

#define CHECK "\r\n"
#define CHECK_SIZE 2

int main(int argc, char *argv[]) {

    void error(const char *msg) ;
    int connection(int socket_connection) ; 
    struct sockaddr_in serve_address, client_address ;
    socklen_t client_length ;
    int socket_connection ;
    int port_number = 8080 ;
    int new_socket_connection ;
    int pid ;
    printf("begin to start server\n") ;

    socket_connection = socket(AF_INET, SOCK_STREAM, 0);
    if(socket_connection < 0)
        error("open socket faild");
    printf("creat socket finish\n") ;

    serve_address.sin_family = AF_INET;
    serve_address.sin_addr.s_addr = INADDR_ANY;
    serve_address.sin_port = htons(port_number);
    if (bind(socket_connection, (struct sockaddr *) &serve_address, sizeof(serve_address)) < 0)
        error("fail on bind socket") ;
    printf("bind socked fnish\n") ;

    listen(socket_connection,5);
    client_length = sizeof(client_address);

    while (1) {
        new_socket_connection = accept(socket_connection, (struct sockaddr *) &client_address, &client_length);
        if (new_socket_connection < 0)
            error("faild on accept");
        pid = fork();
        if (pid < 0)
            error("fail on fork");
        if (pid == 0) {
            close(socket_connection);
            connection(new_socket_connection);
            exit(0);
        } else
            close(new_socket_connection) ;
    }

    close(socket_connection);
    return 0;
}

int connection(int socket_connection) {
    int get_request(int fd, char *buffer) ;
    void send_error(int socket_connection) ;
    void send_ok(int socket_connection) ;
    void send_page(int socket_connection, FILE *fp); 
    char request[500], *ptr, result[1000];
    int file, length;
    if (get_request(socket_connection, request) == 0)
        printf("received failed\n");
    printf("%s\n", request);
    ptr = strstr(request, " HTTP/");
    if (ptr == NULL) 
        printf("Hey we git an not HTTP request!\n");
    else {
        *ptr = 0;
        ptr = NULL;
        if (strncmp(request, "GET ", 4) == 0) 
            ptr = request + 4;
        if (ptr[strlen(ptr) - 1] == '/'){
            FILE *fp = fopen("index.html","r") ;
            send_ok(socket_connection) ;
            send_page(socket_connection, fp) ;
            return 1;
        }
        int len = strlen(ptr) ;
        char tmp[strlen(ptr)];
        strncpy(tmp,&ptr[1],len-1) ;
        tmp[strlen(ptr)-1] = '\0' ;
        FILE *fp = fopen(tmp,"r") ;
        if (fp==NULL) {
            fp = fopen("404.html","r");
            send_error(socket_connection);
        } else
            send_ok(socket_connection) ;
        send_page(socket_connection, fp) ;
    }
    return 1;
}

void send_ok(int socket_connection){
    char *msg1 = "HTTP/1.1 200 OK\r\n" ;
    char *msg2 = "Server : Web Server with C\r\n\r\n" ;
    send(socket_connection, msg1, strlen(msg1), 0);
    send(socket_connection, msg2, strlen(msg2), 0);
}

void send_error(int socket_connection){
    char *msg1 = "HTTP/1.1 404 Not Found\r\n" ;
    char *msg2 = "Server : Web Server with C\r\n\r\n" ;
    send(socket_connection, msg1, strlen(msg1), 0);
    send(socket_connection, msg2, strlen(msg2), 0);
}

void send_page(int socket_connection, FILE *fp){
    char result[1000] ;
    fread(result,1,1000,fp);
    send(socket_connection, result, strlen(result), 0); 
    fclose(fp);
    fp=NULL;
}

int get_request(int fd, char *buffer) {
    char *point = buffer ;
    int match_check = 0;
    while (recv(fd, point, 1, 0) != 0){
        if (*point == CHECK[match_check]){
            ++match_check ; 
            if (match_check == CHECK_SIZE){
                *(point + 1 - CHECK_SIZE) = '\0';
                return (strlen(buffer)) ;
            }
        } else
            match_check = 0 ;
        point ++ ;
    }
    return (0) ;
}

void error(const char *msg) {
    perror(msg);
    exit(1);
}
