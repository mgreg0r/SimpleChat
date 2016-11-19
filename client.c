/* Computer Networks 2016 project 1
 * Client
 * author: Marcin Gregorczyk (mg359198) */

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>

#define BUFFER_SIZE 1004
#define STDIN 0

char msg[BUFFER_SIZE];

void printError(const char* msg) {
  fprintf(stderr, "ERROR: %s\n", msg);
  exit(1);
}


//Reads line from stdin
//and converts message to proper format
int readLine() {
  memset(msg, 0, sizeof(msg));
  int end = 2;
  msg[end] = getchar();
  while(msg[end] != '\n') {
    
    if(end > 1002)
      printError("message is too long");
    
    end++;
    msg[end] = getchar();
  }
  msg[end] = '\0';
  uint16_t size = htons((uint16_t)strlen(msg+2));
  memcpy(msg, &size, 2);
  return end;
}


int main(int argc, char** argv) {
  char* inAddr;
  char* inPort = "20160";
  if(argc > 1)
    inAddr = argv[1];
  if(argc == 3)
    inPort = argv[2];
  else if(argc > 3 || argc < 2) printError("invalid parameters");

  int sock;
  fd_set fds, afds;
  
  struct addrinfo addr_hints;
  struct addrinfo *addr_result;
  
  memset(&addr_hints, 0, sizeof(struct addrinfo));
  addr_hints.ai_family = AF_INET;
  addr_hints.ai_socktype = SOCK_STREAM;
  addr_hints.ai_protocol = IPPROTO_TCP;
  
  int err = getaddrinfo(inAddr, inPort, &addr_hints, &addr_result);
  if(err != 0)
    printError("cannot get addres info");

  sock = socket(addr_result->ai_family, addr_result->ai_socktype, addr_result->ai_protocol);
  if(sock < 0) printError("cannot create socket");
  
  if(connect(sock, addr_result->ai_addr, addr_result->ai_addrlen) < 0)
    printError("connection error");

  FD_ZERO(&afds);
  FD_SET(sock, &afds);
  FD_SET(STDIN, &afds);
  
  while(666) {

    fds = afds;
    int selr = select(sock+1, &fds, NULL, NULL, NULL);
    if(selr < 0)
      printError("cannot select descriptor");
    
    if(FD_ISSET(STDIN, &fds)) {
      //stdin is handled here
      int len = readLine();
      if(write(sock, msg, len) != len)
        printError("cannot send a message");

      memset(msg, 0, sizeof(msg));
      
    }

    if(FD_ISSET(sock, &fds)) {
      //server messages are handled here
      int len = read(sock, msg, BUFFER_SIZE-1);
      if(len < 0)
        printError("couldn't read a message");
      if(len == 0) {
        fprintf(stderr, "serwer zamknął połączenie");
        exit(100);
      }
      msg[len] = '\0';
      printf("%s\n", msg+2);
      memset(msg, 0, sizeof(msg));
    }
  }
  
  return 0;
}
