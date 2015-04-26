/* 
 * C language for Web Http Server 
 * 
 * Some description:
 *
 * (1) Http protocol
 *
 * (2) Multi-threaded
 *
 * (3) Html Page
 *
 */

#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <pthread.h>
#define REQUEST "\r\n"
#define INDEX 2

struct thread_parameter
{
    int s_fd;
} ;

void error(const char *msg)
{
    perror(msg);
    exit(1);
}

void error(const char *msg) ;
void execute(struct thread_parameter *tParam) ;
void print_html(int socket_connection, FILE *fp); 
void print_header(int s_fd, int index) ;
int do_request(int socket_connection) ;
int req(int fd, char *buffer) ;

int main(int argc, char *argv[]) 
{
    struct sockaddr_in serve_addr ;
    if (argc < 2) 
    {
        fprintf(stderr, "there is no port number type in\n");
        exit(1);
    }
    int port_no = atoi(argv[1]) ;
    int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(socket_fd < 0)
    {
        error("can not open socket in this server");
    }
    serve_addr.sin_family = AF_INET;
    serve_addr.sin_addr.s_addr = INADDR_ANY;
    serve_addr.sin_port = htons(port_no);
    int r = bind(socket_fd, (struct sockaddr *) &serve_addr, sizeof(serve_addr)) ;
    if (r < 0)
    {
        error("binding socket error") ;
    }
    listen(socket_fd,5);
    while (1) 
    {
        struct thread_parameter info = { .s_fd = socket_fd } ;
        pthread_t t;
        pthread_create(&t, NULL, (void *)&execute,  (void *) &info);
    }
    close(socket_fd);
    return 0;
}


void execute(struct thread_parameter *info)
{
    struct sockaddr_in client_address ;
    socklen_t client_length ;
    client_length = sizeof(client_address);
    void error(const char *msg) ;
    int new_socket_fd = accept(info->s_fd, (struct sockaddr *) &client_address, &client_length);
    if (new_socket_fd < 0)
    {
        error("failed on new socktet");
    }
    int pid = fork();
    if (pid < 0) 
    {
        error("failed on fork");
    }
    if (pid == 0) {
        close(info->s_fd);
        do_request(new_socket_fd);
        exit(0);
    }
    else
    {
        close(new_socket_fd) ;
    }
    pthread_exit(NULL);
}

void print_header(int s_fd, int index)
{
    char *result ;
    if (index == 1)
    {
        result = "HTTP/1.1 200 OK\r\n Server : Web Server with C\r\n" ;
    }
    else
    {
        result = "HTTP/1.1 404 Not Found\r\n Server : Web Server with C\r\n" ;
    }
    send(s_fd, result, strlen(result), 0);
}

void print_html(int socket_fd, FILE *fp)
{
    char result[1500] ;
    fread(result,1,1500,fp);
    int len = strlen(result) ;
    send(socket_fd, result, len, 0); 
    fclose(fp);
}

int req(int fd, char *buffer)
{
    char *point = buffer ;
    int match_check = 0;
    while (recv(fd, point, 1, 0) != 0)
    {
        if (*point == REQUEST[match_check])
        {
            ++match_check ; 
            if (match_check == INDEX)
            {
                *(point + 1 - INDEX) = '\0';
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

int do_request(int socket_connection)
{
    char request[500], *ptr, result[1000];
    int file, length;
    if (req(socket_connection, request) == 0)
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
            print_header(socket_connection, 1) ;
            print_html(socket_connection, fp) ;
            return 1;
        }
        int len = strlen(ptr) ;
        char tmp[strlen(ptr)];
        strncpy(tmp,&ptr[1],len-1) ;
        tmp[strlen(ptr)-1] = '\0' ;
        FILE *fp = fopen(tmp,"r") ;
        if (fp==NULL)
        {
            fp = fopen("no.html","r");
            print_header(socket_connection, 0);
        }
        else
        {
            print_header(socket_connection, 1) ;
        }
        print_html(socket_connection, fp) ;
    }
    return 1;
}
