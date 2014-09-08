#include "c_socket.h"

int serverfd;
FILE *fp ;

/* Signal Handler for SIGINT */
void sigintHandler(int sig_num){
    /* clean up */
    fclose(fp);
    close ( serverfd) ;
    printf("Finish cleanup\n");
    exit(EXIT_SUCCESS); 
}
int 
main (int argc,
      char *argv[]){
    /* argument check */
    if(argc != 2 )
        ERROR_EXIT(0,"Usage: argv[0] port\n" );

    /* socket create bind listen  */
    serverfd = socket_bind(argv[1]);
    socket_non_blocking(serverfd);
    if( listen(serverfd,SOMAXCONN) == -1 )
        ERROR_EXIT_NORMAL;

    /* epoll create */
    int eventfd = epoll_create1(0);
    if( eventfd == -1 )
        ERROR_EXIT_NORMAL;

    /* event setting  */
    struct epoll_event event;
    event.data.fd = serverfd;
    event.events = EPOLLIN | EPOLLOUT ;

    /* register serverfd with event into eventfd  */
    if(epoll_ctl(eventfd,EPOLL_CTL_ADD,serverfd,&event) ==-1 )
        ERROR_EXIT_NORMAL;

#define MAXEVENTS 64
    struct epoll_event *events;
    events = calloc(MAXEVENTS,sizeof(struct  epoll_event) );

    /* the event loop  */
    signal(SIGINT,sigintHandler);
    fp=fopen("log.txt","wb+"); 
    while (1){
        int num_event = epoll_wait(eventfd,events,MAXEVENTS,-1);
        for(int i=0;i<num_event;i++ ){
            /*  new client connect  */
            if(serverfd == events[i].data.fd) {
                struct sockaddr in_addr;
                socklen_t  in_len = sizeof( struct sockaddr);
                char host_buf[NI_MAXHOST],service_buf[NI_MAXSERV];
                int clientfd=accept(serverfd,&in_addr,&in_len);
                if(getnameinfo(&in_addr,in_len,
                               host_buf,sizeof (host_buf) ,
                               service_buf,sizeof(service_buf),
                               NI_NUMERICHOST | NI_NUMERICSERV) == 0 ){
                    printf("Accept connection on descriptor %d "
                           "(host=%s, port=%s) \n",clientfd,host_buf,service_buf);
                }
                else
                    ERROR_EXIT_NORMAL;

                socket_non_blocking(clientfd);
                event.data.fd = clientfd;
                event.events = EPOLLIN | EPOLLOUT | EPOLLRDHUP  ;
                if(epoll_ctl(eventfd,EPOLL_CTL_ADD,clientfd,&event) ==-1 )
                    ERROR_EXIT_NORMAL; 
            }
            else {
                if(events[i].events & EPOLLRDHUP ){
                    printf("EPOLLRDHUP: close descriptor %d\n",events[i].data.fd);
                    epoll_ctl(eventfd,EPOLL_CTL_DEL,events[i].data.fd,NULL);
                    close(events[i].data.fd);
                }
                else if ( events[i].events & EPOLLIN) {
                    printf("EPOLLIN: read data from descriptor %d\n",events[i].data.fd);
#define DATA_BUF 50
                    char data_buf[DATA_BUF];
                    ssize_t read_bytes;
                    read_bytes = read(events[i].data.fd,data_buf,DATA_BUF);
                    /* write(1,data_buf,read_bytes); */
                    fwrite(data_buf,1,read_bytes,fp);
                }
                /* write(events[i].data.fd,data_buf,read_bytes); */
            }
        }
    }

    return 0;
}
