#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>


#define MAXDATASIZE 1024 // max number of bytes we can get at once

int main(int argc, char const *argv[]) {
  int sockfd, numbytes;
  char buf[MAXDATASIZE];
  struct hostent *he;
  struct sockaddr_in servaddr; // server's address structure

  if (argc != 2&&argc!= 3) { // must provide server name
    printf("usage: client servername\n");
    exit(1);
  }
  if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
    printf("socket error!\n");
    exit(1);
  }


// ---------------- parse input url  to hostname ,  port , identifier--------
   // without -h option
    char *hostname;
    int port = 0;
    char *identifier;
    char *aa;
    char *url;
    if (argc == 2) {
      url = malloc(1+strlen(argv[1]));
      strcpy(url,argv[1]);
    }else{
      url = malloc(1+strlen(argv[2]));
      strcpy(url,argv[2]);
    }
    //printf("---%lu\n ---%s\n", strlen(url),url);
    char *p,*r;
    p = strtok(url, "/");  //get http:
    //printf("---%s\n",p);
    p = strtok(NULL, ""); //get  /iastate.edu:90/path...
    //printf("---%s\n",p);
    p = strtok(p, "/");  //get iastate.ed:90  or  iastate.edu         p
    //printf("---%s\n",p);
    r = strtok(NULL, "/"); //if no identifier, get null or get /path  r
    //printf("---%s\n",r);

    // then we manipulate the p and r corresponding string.
    if (strstr(p,":") != NULL) {   // have ":"  get port and host
      aa =  strtok(p, ":");
      if((hostname = malloc(strlen(aa)+1)) != NULL){
        hostname[0] = '\0';   // ensures the memory is an empty string
        strcat(hostname,aa);
      } else {
        printf("malloc failed!\n");
      }
      //printf("%s\n",hostname );
      port =  atoi(strtok(NULL, ":"));
      //char *a = "80";
      //printf("%d\n",port);
    }else{                        //dont have ":"  directly get host
      //printf("adfs\n" );
      port = 80;
        //printf("adfs\n" );
        //aa =  strtok(p, "/");
        if((hostname = malloc(strlen(p)+1)) != NULL){
        hostname[0] = '\0';   // ensures the memory is an empty string

        strcat(hostname,p);
        } else {
        printf("malloc failed!\n");
        }
        //printf("%s\n",hostname );
    }
    if (r != NULL) {  // 统一get identifier from r
      if((identifier = malloc(strlen("/")+strlen(r)+1)) != NULL){
      identifier[0] = '\0';   // ensures the memory is an empty string
      strcat(identifier,"/");
      strcat(identifier,r);
      } else printf("malloc failed!\n");
    }else identifier = "/";
    //printf("%s\n",identifier); //strlen(identifier) =6 for /hejin  ok

// ---------------- -------- -------- constrcut request msg -------- --------
    char* request_msg;
    if (argc == 2) {
      if((request_msg = malloc(strlen("GET ")+strlen(identifier)+strlen(" HTTP/1.0\r\n\r\n")+1)) != NULL){
        strcat(request_msg,"GET ");
        strcat(request_msg,identifier);
        strcat(request_msg," HTTP/1.0\r\n\r\n");
      }else printf("malloc failed!\n");
      //printf("%s\n %lu\n", request_msg,strlen(request_msg)); //except \r\n\r\n is 24, total is 28.
    }else if (argc == 3) {
      if (strcmp(argv[1],"-h")==0) { //option -h
        if((request_msg = malloc(strlen("HEAD ")+strlen(identifier)+strlen(" HTTP/1.0\r\n\r\n")+1)) != NULL){
          strcat(request_msg,"HEAD ");
          strcat(request_msg,identifier);
          strcat(request_msg," HTTP/1.0\r\n\r\n");
        }else printf("malloc failed!\n");
        //printf("%s\n %lu\n", request_msg,strlen(request_msg)); //except \r\n\r\n is 24, total is 28.
      }else{
        printf("invalid option argument!\n");
        exit(1);
      }
    }else{
      printf("invalid argument: (-h) URL\n");
    }

    printf("hostname: %s\nport:%d\nidentifier:%s\n",hostname,port,identifier);
    printf("%s\n", request_msg); //except \r\n\r\n is 24, total is 28.
//-----------------------------connect to server--------------------------

  //printf("%s\n%lu",hostname,strlen(hostname));
  if ((he=gethostbyname(hostname)) == NULL) { // get the host info of server
    printf("gethostbyname error!\n");
    exit(1);
  }

  //memset (&servaddr, 0, sizeof(servaddr));
  bzero(&servaddr, sizeof(servaddr));
  servaddr.sin_family = AF_INET;
  servaddr.sin_port = htons(port);
  // if (inet_aton(hostname, (struct in_addr *) &servaddr.sin_addr.s_addr) == 0) {
  //   perror(argv[1]);
  //   exit(errno);
  // }

memcpy(&servaddr.sin_addr, he->h_addr_list[0], he->h_length); //copy server’s IP address to servaddr.sin_addr
//printf(" %s\n", inet_ntoa(servaddr.sin_addr));
  if (connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) == -1) {
    printf("connect error!\n");
    exit(1);
  }

//----------------------------send request msg to server-------------------
  if (write(sockfd,request_msg,strlen(request_msg)) < 0) {
    // send GET or HEAD request msg to server
    printf("send error!\n");
    exit(1);
  }
  //  printf("wait for response");
  // read message from server
  // if ((numbytes = read(sockfd, buf, MAXDATASIZE)) < 0) {
  //   printf("read error!\n");
  //   exit(1);
  // }

// Open a file for writing.
// (This will replace any existing file. Use "w+" for appending)
  int in,index = 0,limit = MAXDATASIZE;
  char message[MAXDATASIZE+1];
  FILE *file = fopen("response.txt", "w+");
  // while ((in = recv(sockfd, &message[index], MAXDATASIZE, 0)) > 0) {
  //     index += in;
  //     limit -= in;
  //     if (limit == 0) {
  //       int results = fputs(message, file);
  //       index = 0;
  //       limit = MAXDATASIZE;
  //       if (results == EOF) {
  //           printf("store in file error!\n");
  //       }
  //     }
  // }

//-------------only use while recv can dynamically receive large file---------
  do {
    bzero(message, MAXDATASIZE+1);
    in = recv(sockfd, message, MAXDATASIZE, 0);
    if ( in > 0 ) {
        int results = fputs(message, file);
        if (results == EOF) {
            printf("store in file error!\n");
        }
    }
  }
  while ( in > 0 );
  fclose(file);



  //print server’s message
  //printf("Received: %s",buf);
  close(sockfd);


  return 0;
}
