/* Sample TCP server */
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <zconf.h>
#include <time.h>
#include <stdlib.h>
#include <pthread.h>

#define WINDOW_SIZE 4
#define MAX_SEQ_NUM 8
int nextseqnum = 0;
int listenfd, connfd;
struct sockaddr_in servaddr, cliaddr;
socklen_t clilen;

void sendInitialSequenceNumber(int connfd, struct sockaddr_in *cliaddr, int ISN) {
    char str[10];
    sprintf(str, "%d", ISN);
    sendto(connfd, str, strlen(str), 0, (struct sockaddr *) &cliaddr, sizeof(cliaddr));
    printf("Sent Initial Sequence Number :%s \n", str);

}

int extractACKNumber(char *str) {
    int init_size = strlen(str);
    char delim[] = "_";

    char *ptr = strtok(str, delim);
    ptr = strtok(NULL, delim);

    return atoi(ptr);

}

void *receiveAcknowledgement() {


    printf("thread started \n");
    while (1) {
        char buffer[6];
        int n;

        int number = -1;
        n = recvfrom(connfd, buffer, 6, 0, (struct sockaddr *) &cliaddr,
                     &clilen);// information of the client by recvfrom function
        if (n > 0) {
            buffer[5] = 0;
            number = extractACKNumber(buffer);
            printf("Received ACK # %d \n", number);

        }
        buffer[n] = 0;

    }

    return NULL;


}


int main(int argc, char **argv) {
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
    pthread_t inc_x_thread;


    /* create a second thread which Receives Acknowledgements */
    if (pthread_create(&inc_x_thread, NULL, receiveAcknowledgement, NULL)) {

        fprintf(stderr, "Error creating thread\n");
        return 1;

    }

    for (int i = 0; i < WINDOW_SIZE; ++i) {
        sprintf(str, "%d", nextseqnum);
        sendto(connfd, str, strlen(str), 0, (struct sockaddr *) &cliaddr, sizeof(cliaddr));
        timeout[i] = clock();
        printf("Sent :%s \n", str);
        currentWindow[i] = nextseqnum;
        nextseqnum = (nextseqnum + 1) % MAX_SEQ_NUM;
        sleep(1);
    }


    if(pthread_join(inc_x_thread, NULL)) {

        fprintf(stderr, "Error joining thread\n");
        return 2;

    }

    return 0;
}