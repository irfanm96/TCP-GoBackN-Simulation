/* Sample TCP server */
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <zconf.h>
#include <time.h>

#define WINDOW_SIZE 4
#define MAX_SEQ_NUM 8
int nextseqnum = 0;

void sendInitialSequenceNumber(int connfd, struct sockaddr_in *cliaddr, int ISN) {
    char str[10];
    sprintf(str, "%d", ISN);
    sendto(connfd, str, strlen(str), 0, (struct sockaddr *) &cliaddr, sizeof(cliaddr));
    printf("Sent Initial Sequence Number :%s \n", str);

}

int receiveAcknowledgement(int connfd, struct sockaddr_in *cliaddr, socklen_t clilen) {

    char buffer[6];
    int n;

    if (1==1) {
        n = recvfrom(connfd, buffer, 6, 0, (struct sockaddr *) &cliaddr,
                     &clilen);// information of the client by recvfrom function
        if (n > 0) {
            buffer[5] = 0;
            printf("Received %s \n", buffer);
        }
        buffer[n] = 0;

        return 0;
    }

    return -1;

}


int main(int argc, char **argv) {
    int listenfd, connfd, n;
    struct sockaddr_in servaddr, cliaddr;
    socklen_t clilen;
    char *banner = "H";
    char buffer[1000];

    /* one socket is dedicated to listening */
    listenfd = socket(AF_INET, SOCK_STREAM, 0);

    /* initialize a sockaddr_in struct with our own address information for binding the socket */
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(32000);

    /* binding */
    bind(listenfd, (struct sockaddr *) &servaddr, sizeof(servaddr));
    listen(listenfd, 0);
    clilen = sizeof(cliaddr);

    /* accept the client with a different socket. */
    connfd = accept(listenfd, (struct sockaddr *) &cliaddr,
                    &clilen); // the uninitialized cliaddr variable is filled in with the

    sendInitialSequenceNumber(connfd, &cliaddr, 0);


    char str[10];
    clock_t timeout[WINDOW_SIZE];
    int currentWindow[WINDOW_SIZE];

    for (int i = 0; i < WINDOW_SIZE; ++i) {
        sprintf(str, "%d", nextseqnum);
        sendto(connfd, str, strlen(str), 0, (struct sockaddr *) &cliaddr, sizeof(cliaddr));
        timeout[i] = clock();
        printf("Sent :%s \n", str);
        currentWindow[i] = nextseqnum;
        nextseqnum = (nextseqnum + 1) % MAX_SEQ_NUM;
        fork();
        receiveAcknowledgement(connfd, &cliaddr, clilen);

    }

    return 0;
}