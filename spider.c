#include "../../sk.h"


int
main(int argc ,
     char* argv[]){

    /* check argument */
    if(argc < 3){
        fprintf(stderr,"Usage: %s domain url\n",
                argv[0]);
        return -1;
    }
    
    /* create a socket  */
    int sockfd = socket_connect(argv[1],"80");

    /* send http request message  */
    char request_message[1000]; 
    sprintf(request_message,"GET %s HTTP/1.1\r\nHost: %s\r\n\r\n",argv[2],argv[1]);
    
    ssize_t send_bytes = send(sockfd,
                              request_message,
                              strlen(request_message),
                              0);
    if(send_bytes == -1 ) {
        fprintf(stderr,"ERR: request_message can't send\n");
        return 4;
    }
    
    
    /* get http response message  */
    FILE * fp = fopen(argv[2]+1,"wb");
    if(!fp )
        ERROR_EXIT_NORMAL;
    while (1 ) {
        char response_meesage[100000];
        ssize_t recv_bytes =recv(sockfd,
                                 response_meesage,
                                  sizeof(response_meesage),
                                 0);
        if(recv_bytes == -1 ){
            fprintf(stderr,"ERR: response_message can't recv\n");
            return 5;
        }
        if(recv_bytes ==0 )
            break;
        
        /* write response  message to console */
        fwrite(response_meesage,1,recv_bytes,fp);
        printf("fwrite(%ld)\n",recv_bytes);
    }

    /* clean up  */
    printf("clean up\n");
    close(sockfd);
    fclose(fp);

    
    return 0;
}
