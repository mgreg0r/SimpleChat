/* Computer Networks 2016 project 1
 * Server
 * author: Marcin Gregorczyk (mg359198) */

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#define BUFFER_SIZE 1004
#define QUEUE_LENGTH 20

int clients[QUEUE_LENGTH];
char buffer[BUFFER_SIZE];

void printError(char* msg) {
  printf("ERROR: %s\n", msg);
  exit(1);
}


//Reads message from descriptor fd,
//validates, and broadcasts it to clients
int readFromClient(int fd) {
  int len = read(fd, buffer, BUFFER_SIZE);
  if(len <= 0) {
    return -1;
  }
  buffer[len] = '\0';
  uint16_t mSize;
  memcpy(&mSize, buffer, 2);
  mSize = ntohs(mSize);
  if(mSize > 1000 || mSize != strlen(buffer+2)) {
    return -1;
  }
  printf("%s\n", buffer+2);
  for(int i = 0; i < QUEUE_LENGTH; i++) {
    if(clients[i] != 0 && clients[i] != fd) {
      int sndLen = write(clients[i], buffer, len);
      if(sndLen != len) printError("cannot send message");
    }
  }
  return 0;
}

int main(int argc, char** argv) {
  char* lport = "20160";
  if(argc > 2) {
    printf("Invalid parameters\n");
    exit(1);
  }
  if(argc == 2)
    lport = argv[1];
  
  struct sockaddr_in serverAddress, clientAddress;
  fd_set fds, afds;
  int sock = socket(PF_INET, SOCK_STREAM, 0);
  if(sock < 0)
    printError("cannot create socket");

  serverAddress.sin_family = AF_INET;
  serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);
  serverAddress.sin_port = htons(atoi(lport));

  if(bind(sock, (struct sockaddr*) &serverAddress, (socklen_t) sizeof(serverAddress)) < 0)
    printError("cannot bind socket");

  if(listen(sock, 1) < 0)
    printError("cannot use a socket for listen");

  FD_ZERO(&afds);
  FD_SET(sock, &afds);
  
  while(666) {

    fds = afds;
    if(select(FD_SETSIZE, &fds, NULL, NULL, NULL) < 0)
      printError("cannot select descriptor");

    for(int i = 0; i < FD_SETSIZE; i++) {
      if(FD_ISSET(i, &fds)) {
        if(i == sock) {
          //New connections are handled here
          socklen_t size = sizeof(clientAddress);
          int newCon = accept(sock, (struct sockaddr *) &clientAddress,
                              &size);
          if(newCon < 0)
            printError("cannot accept new connection");

          FD_SET(newCon, &afds);
          for(int j = 0; j < QUEUE_LENGTH; j++)
            if(clients[j] == 0) {
              clients[j] = newCon;
              break;
            }
        }
        else {
          //Existing connections are handled here
          if(readFromClient(i) < 0) {
            close(i);
            FD_CLR(i, &afds);
            for(int j = 0; j < QUEUE_LENGTH; j++)
              if(clients[j] == i)
                clients[j] = 0;
          }
          
        }
      }
    }
    
  }
  
  return 0;
}
