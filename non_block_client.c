#include "c_socket.h"


void sk_connect(int i,void *userdata){
    int serverfd = socket_connect("127.0.0.1","8080");
    char message[50];
    sprintf(message,"this message form %d\n",getpid());
    printf("%s",message);
    write(serverfd,message,strlen(message));
    wait(0);
    close(serverfd);
}


int
main (int argc,
      char *argv[] ) {
    process(10,&sk_connect);
    return 0;
}
