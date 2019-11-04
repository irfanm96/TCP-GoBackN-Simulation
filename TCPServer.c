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

int lastAcknowledged = 0;

clock_t timeout[WINDOW_SIZE];
int currentWindow[WINDOW_SIZE];

int isTimeout = 0;

int timeoutDemo = 1;

void msleep(long msec) {
    struct timespec ts;
    int res;
    ts.tv_sec = msec / 1000;
    ts.tv_nsec = (msec % 1000) * 1000000;

    res = nanosleep(&ts, &ts);
}

void shiftWindow() {


    for (int i = 0; i < WINDOW_SIZE - 1; ++i) {
        currentWindow[i] = currentWindow[i + 1];
    }

    currentWindow[WINDOW_SIZE - 1] = nextseqnum;
    nextseqnum = (nextseqnum + 1) % MAX_SEQ_NUM;


    for (int j = 0; j < WINDOW_SIZE; ++j) {
        printf(" %d ", currentWindow[j]);
    }
    printf("\n");


}

void sendInitialSequenceNumber(int ISN) {
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


void *checkTimeout() {

    printf("timeout thread started \n");
    char str[10];
    double cpu_time_used = 0.00;
    while (1) {
        cpu_time_used = ((double) (clock() - timeout[0])) / CLOCKS_PER_SEC;
        if (cpu_time_used > 8) {
            //timed out
            isTimeout = 1;
            printf("its timeout %f \n", cpu_time_used);

            for (int i = 0; i < WINDOW_SIZE; ++i) {
                sprintf(str, "%d", currentWindow[i]);
                sendto(connfd, str, strlen(str), 0, (struct sockaddr *) &cliaddr, sizeof(cliaddr));
                timeout[i] = clock();
                printf("ReSent after timeout :%s \n", str);
                msleep(10);
            }
            cpu_time_used = 0;
            isTimeout = 0;
            break;
        } else {
            isTimeout = 0;
        }


    }
}

void *receiveAcknowledgement() {


    printf("acknowledgment thread started \n");
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
            if (number == 0 || number == lastAcknowledged + 1) {

                if (number == 0) {

                    lastAcknowledged = 0;
                } else {
                    lastAcknowledged = (lastAcknowledged + 1) % MAX_SEQ_NUM;

                }
                shiftWindow();

            } else {
                printf("cannot shift window \n");
            }
        }
        buffer[n] = 0;

    }

    return NULL;


}


int main(int argc, char **argv) {
    char *banner = "H";
    char buffer[1000];


    if (argc > 2) {
        printf("Usage : Give timeout , to simulate the corresponding feature \n Leave Blank for normal operation \n");
    }

    if (strcmp("timeout", argv[1]) == 0) {
        timeoutDemo = 1;
    } else {
        printf("Invalid argument \n");
        return -1;
    }


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

    sendInitialSequenceNumber(0);


    char str[10];
    pthread_t receive_thread;
    pthread_t timeout_thread;


    /* create a second thread which Receives Acknowledgements */
    if (pthread_create(&receive_thread, NULL, receiveAcknowledgement, NULL)) {

        fprintf(stderr, "Error creating thread for receiving acknowledgments\n");
        return 1;

    }

    if (timeoutDemo == 1) {
        /* create a second thread which checks timeout */
        if (pthread_create(&timeout_thread, NULL, checkTimeout, NULL)) {
            fprintf(stderr, "Error creating thread for timeout check\n");
            return 1;
        }
    }

    for (int j = 0; j < WINDOW_SIZE; ++j) {
        currentWindow[j] = nextseqnum;
        nextseqnum = (nextseqnum + 1) % MAX_SEQ_NUM;
    }

    if (isTimeout != 1) {
        for (int i = 0; i < WINDOW_SIZE; ++i) {
            sprintf(str, "%d", currentWindow[0]);
            sendto(connfd, str, strlen(str), 0, (struct sockaddr *) &cliaddr, sizeof(cliaddr));
            timeout[i] = clock();
            printf("Sent :%s \n", str);
            msleep(1000);

        }

    }

    while (1) {

    }

    return 0;
}