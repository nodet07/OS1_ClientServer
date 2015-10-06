#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
/* port we're listening on */
#define PORT 2020
#define MAXCLIENTS 10

struct client{
        int serve_port;
        int file_count;
        int load;
        char files[10][50];
};

void init_clients(struct client(*all_clients)[MAXCLIENTS]){
    int i,j;
    for(i=0; i<MAXCLIENTS; i++){
        (*all_clients)[i].serve_port = -1;
        (*all_clients)[i].file_count = 0;
        (*all_clients)[i].load = 0;
        for(j=0;j<10;j++){
            (*all_clients)[i].files[j][0]='-';
        }
    }
}

//void create_client(int client_port_no,)
void add_file_to_client(int client_port_no,char* file_name,struct client(*all_clients)[MAXCLIENTS]){
    int i;
    for(i=0;i<MAXCLIENTS;i++){
        if( (*all_clients)[i].serve_port == client_port_no ) {
                    strcpy((*all_clients)[i].files[(*all_clients)[i].file_count],file_name);
                    (*all_clients[i]).file_count += 1;
                    break;
        }
    }
}

int main(int argc, char *argv[]) {

    struct client all_clients[MAXCLIENTS];
    init_clients(&all_clients);
    all_clients[3].serve_port = 9090;
    all_clients[0].serve_port = 8080;
    add_file_to_client(8080,"kosse bibit.pdf",&all_clients);
    add_file_to_client(8080,"kosse bibit.mp3",&all_clients);
    add_file_to_client(9090,"porn.mp4",&all_clients);
    int k,l;

    for(k=0;k<MAXCLIENTS;k++){
        printf("%d\n",(all_clients)[k].serve_port);
        for(l=0;l<10;l++){
                if( (all_clients)[k].files[l][0] != '-' ){
                    printf("%s\n",(all_clients)[k].files[l]);
                }
            }
        printf("################\n");
    }
    int kir;
    scanf("%d",&kir);

    /* master file descriptor list */
    fd_set master;
    /* temp file descriptor list for select() */
    fd_set read_fds;
    /* server address */
    struct sockaddr_in serveraddr;
    /* client address */
    struct sockaddr_in clientaddr;
    /* maximum file descriptor number */
    int fdmax;
    /* listening socket descriptor */
    int listener;
    /* newly accept()ed socket descriptor */
    int newfd;
    /* buffer for client data */
    char buf[1024];
    int nbytes;
    /* for setsockopt() SO_REUSEADDR, below */
    int yes = 1;
    int addrlen;
    int i, j;
    /* clear the master and temp sets */
    FD_ZERO(&master);
    FD_ZERO(&read_fds);

    /* get the listener */
    if((listener = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("Server-socket() error lol!");
        /*just exit lol!*/
        exit(1);
    }
    printf("Server-socket() is OK...\n");
    /*"address already in use" error message */
    if(setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
        perror("Server-setsockopt() error lol!");
        exit(1);
    }
    printf("Server-setsockopt() is OK...\n");

    /* bind */
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = INADDR_ANY;
    serveraddr.sin_port = htons(PORT);
    memset(&(serveraddr.sin_zero), '\0', 8);

    if(bind(listener, (struct sockaddr *)&serveraddr, sizeof(serveraddr)) == -1) {
        perror("Server-bind() error lol!");
        exit(1);
    }
    printf("Server-bind() is OK...\n");
    /* listen */
    if(listen(listener, 10) == -1) {
        perror("Server-listen() error lol!");
        exit(1);
    }
    printf("Server-listen() is OK...\n");

    /* add the listener to the master set */
    FD_SET(listener, &master);
    /* keep track of the biggest file descriptor */
    fdmax = listener; /* so far, it's this one*/
    printf("listerner = %d",listener);
    /* loop */
    for(;;) {
        /* copy it */
        read_fds = master;

        if(select(fdmax+1, &read_fds, NULL, NULL, NULL) == -1) {
            perror("Server-select() error lol!");
            exit(1);
        }
        printf("Server-select() is OK...\n");

        /*run through the existing connections looking for data to be read*/
        for(i = 0; i <= fdmax; i++) {
            if(FD_ISSET(i, &read_fds)) { /* we got one... */
                if(i == listener) {
                    /* handle new connections */
                    addrlen = sizeof(clientaddr);
                    if((newfd = accept(listener, (struct sockaddr *)&clientaddr, &addrlen)) == -1) {
                        perror("Server-accept() error lol!");
                    }
                    else {
                        printf("Server-accept() is OK...\n");
                        FD_SET(newfd, &master); /* add to master set */
                        if(newfd > fdmax) { /* keep track of the maximum */
                            fdmax = newfd;
                        }
                        printf("%s: New connection from %s on socket %d\n", argv[0], inet_ntoa(clientaddr.sin_addr), newfd);
                    }
                }
                else {
                    /* handle data from a client */
                    if((nbytes = recv(i, buf, sizeof(buf), 0)) <= 0) {
                        /* got error or connection closed by client */
                        if(nbytes == 0)
                            /* connection closed */
                            printf("%s: socket %d hung up\n", argv[0], i);

                        else{
                            perror("recv() error lol!");
                        }
                        /* close it... */
                        close(i);
                        /* remove from master set */
                        FD_CLR(i, &master);
                    }
                    else {
                        /* we got some data from a client*/
                        for(j = 0; j <= fdmax; j++) {
                            /* send to everyone! */
                            if(FD_ISSET(j, &master)) {
                                /* except the listener and ourselves */
                                if(j != listener && j == i) {
                                    if(send(j, buf, nbytes, 0) == -1)
                                        perror("send() error lol!");
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    return 0;
}
