/*Sample TCP client */
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <zconf.h>

#define MAX_SEQ_NUM 8


int receiveInitialSequenceNumber(int sockfd) {
    char buffer[2];
    int n;
    n = recvfrom(sockfd, buffer, 2, 0, NULL, NULL);
    if (n > 0) {
        buffer[1] = 0;
        printf("Received the Initial Sequence Number :%s \n", buffer);
    }
    return atoi(buffer);
}

//returns true if the correct sequence number is arrived

int receiveExpectedSequenceNumber(int sockfd, int expectedSequenceNumber) {
    char buffer[2];
    int n;
    n = recvfrom(sockfd, buffer, 2, 0, NULL, NULL);
    if (n > 0) {
        buffer[1] = 0;
        printf("Received Sequence Number :%s \n", buffer);
    }

    return expectedSequenceNumber == atoi(buffer);
}

void sendAcknowledgement(int sockfd, struct sockaddr_in *servaddr, int sequenceNumber) {

    char banner[6];
    snprintf(banner, 6, "ACK_%d", sequenceNumber);
    sendto(sockfd, banner, strlen(banner), 0, (struct sockaddr *) &servaddr, sizeof(servaddr));
    sleep(1);
    printf("ACK sent to Packet %d \n", sequenceNumber);

}


int main(int argc, char **argv) {
    int sockfd, n;
    struct sockaddr_in servaddr;
    char banner[] = "Hello TCP server! This is client";
    char buffer[2];

    if (argc != 2) {
        printf("usage: ./%s <IP address> \n", argv[0]);
        return -1;
    }

    /* socket to connect */
    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    /* IP address information of the server to connect */
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr(argv[1]);
    servaddr.sin_port = htons(32000);

    connect(sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr));
    int ISN = receiveInitialSequenceNumber(sockfd);

    int expectedSequenceNumber = ISN;
    int state = 0;
    while (1) {

        state = receiveExpectedSequenceNumber(sockfd, expectedSequenceNumber);

        // send ack for the received packet
        sendAcknowledgement(sockfd, &servaddr, expectedSequenceNumber);

        if (state == 1) {
            //increment the next packet sequence number
            expectedSequenceNumber = (expectedSequenceNumber + 1) % MAX_SEQ_NUM;
        }

    }

    return 0;
}
