/* 
 * Team members: 
 *
 * 1. Jinhui Liu
 * 2. MingYang Xu
 * 3. Kai Liang
 *
 * compile:
 *  gcc -o server server.c
 *
 * Start server:
 *  ./server
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

#define EOF_CHECK "\r\n"
#define EOF_SIZE 2

struct message {
    int socket_fd;
} ;

void error(const char *msg)
{
    perror(msg);
    exit(1);
}

void error(const char *msg) ;
void run_thread(struct message *m) ;
int handler_request(int socket_connection) ;
int get_request(int fd, char *buffer) ;
void no_found_header(int socket_connection) ;
void ok_header(int socket_connection) ;
void page_content(int socket_connection, FILE *fp); 

int main(int argc, char *argv[]) 
{

    if (argc < 2) 
    {
        fprintf(stderr,"pls send port provided\n");
        exit(1);
    }

    int port_no = atoi(argv[1]) ;
    struct sockaddr_in serve_address ;
    int socket_fd = socket(AF_INET, SOCK_STREAM, 0);

    if(socket_fd < 0)
    {
        error("sorry error of open socket");
    }
    serve_address.sin_family = AF_INET;
    serve_address.sin_addr.s_addr = INADDR_ANY;
    serve_address.sin_port = htons(port_no);
    if (bind(socket_fd, (struct sockaddr *) &serve_address, sizeof(serve_address)) < 0)
    {
        error("sorry error on bind socket") ;
    }
    listen(socket_fd,5);

    while (1) 
    {
        pthread_t thread ;
        struct message info = { .socket_fd = socket_fd } ;
        pthread_create(&thread, NULL, (void *)&run_thread,  (void *) &info);
    }

    close(socket_fd);
    return 0;
}

int handler_request(int socket_connection) {
    char request[500], *ptr, result[1000];
    int file, length;
    if (get_request(socket_connection, request) == 0)
    {
        printf("received failed\n");
    }
    printf("%s\n", request);
    ptr = strstr(request, " HTTP/");
    if (ptr == NULL) 
    {
        printf("Hey we git an not HTTP request!\n");
    }
    else
    {
        *ptr = 0;
        ptr = NULL;
        if (strncmp(request, "GET ", 4) == 0) 
        {
            ptr = request + 4;
        }
        if (ptr[strlen(ptr) - 1] == '/')
        {
            FILE *fp = fopen("index.html","r") ;
            ok_header(socket_connection) ;
            page_content(socket_connection, fp) ;
            return 1;
        }
        int len = strlen(ptr) ;
        char tmp[strlen(ptr)];
        strncpy(tmp,&ptr[1],len-1) ;
        tmp[strlen(ptr)-1] = '\0' ;
        FILE *fp = fopen(tmp,"r") ;
        if (fp==NULL)
        {
            fp = fopen("no_page.html","r");
            no_found_header(socket_connection);
        }
        else
        {
            ok_header(socket_connection) ;
        }
        page_content(socket_connection, fp) ;
    }
    return 1;
}

void run_thread(struct message *info)
{
    struct sockaddr_in client_address ;
    socklen_t client_length ;
    client_length = sizeof(client_address);
    void error(const char *msg) ;
    int new_socket_fd = accept(info->socket_fd, (struct sockaddr *) &client_address, &client_length);
    if (new_socket_fd < 0)
    {
        error("error on accept new socket!");
    }
    int pid = fork();
    if (pid < 0) 
    {
        error("error on on fork");
    }
    if (pid == 0) {
        close(info->socket_fd);
        handler_request(new_socket_fd);
        exit(0);
    }
    else
    {
        close(new_socket_fd) ;
    }
    pthread_exit(NULL);
}

void ok_header(int socket_fd)
{
    char *message_one = "HTTP/1.1 200 OK\r\n" ;
    char *message_two = "Server : Web Server with C\r\n" ;
    send(socket_fd, message_one, strlen(message_one), 0);
    send(socket_fd, message_two, strlen(message_two), 0);
}

void no_found_header(int socket_fd)
{
    char *message_one = "HTTP/1.1 404 Not Found\r\n" ;
    char *message_two = "Server : Web Server with C\r\n" ;
    send(socket_fd, message_one, strlen(message_one), 0);
    send(socket_fd, message_two, strlen(message_two), 0);
}

void page_content(int socket_fd, FILE *fp)
{
    char content[1000] ;
    fread(content,1,1000,fp);
    send(socket_fd, content, strlen(content), 0); 
    fclose(fp);
    fp=NULL;
}

int get_request(int fd, char *buffer)
{
    char *point = buffer ;
    int match_check = 0;
    while (recv(fd, point, 1, 0) != 0)
    {
        if (*point == EOF_CHECK[match_check])
        {
            ++match_check ; 
            if (match_check == EOF_SIZE)
            {
                *(point + 1 - EOF_SIZE) = '\0';
                return (strlen(buffer)) ;
            }
        } 
        else
        {
            match_check = 0 ;
        }
        point ++ ;
    }
    return (0) ;
}
