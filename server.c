#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <time.h>

#define MYPORT 3509
#define BACKLOG 10

int main() {
  int sockfd, new_fd; // listen on sockfd, new connection on new_fd struct
  struct sockaddr_in myaddr; // server’s address structure
  struct sockaddr_in cliaddr; // client's address structure
  socklen_t len;
  char buff[80];
  char message[1024];
  char resp[100];

  if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
    printf("socket error!\n");
    exit(1);
  }

  bzero(&myaddr, sizeof(myaddr));
  myaddr.sin_family = AF_INET;
  myaddr.sin_port = htons(MYPORT);
  myaddr.sin_addr.s_addr = htonl(INADDR_ANY);

  if (bind(sockfd, (struct sockaddr *)&myaddr, sizeof(myaddr)) == -1) {
    printf("bind error!\n");
    exit(1);
  }

  if (listen(sockfd, BACKLOG) == -1) {
    printf("listen error!\n");
    exit(1);
  }

  while(1) {
    len = sizeof(cliaddr);
    if ((new_fd = accept(sockfd, (struct sockaddr *)&cliaddr, &len)) == -1) {
      printf("accept error!\n"); exit(1);
    }
    printf("Connection from: %s, Port: %d\n", inet_ntop(AF_INET, &cliaddr.sin_addr, buff, sizeof(buff)), ntohs(cliaddr.sin_port)); // print client’s IP address and port number
    int in;

    // if((in =recv(new_fd, message, 1024, 0))==-1){
    //   printf("successffuly received: %s\n",message);
    // }
//---------------------recv the client's request ------------------------
    int byte_count;
    byte_count = recv(new_fd, message, sizeof(message), 0);
    printf("recv()'d: %s\n", message);
    //printf("recv()'d %d bytes of data in buf\n", byte_count);


//---------------------parse the request URL----------------------------
    char *p,*r,*identifier;
    p = strtok(message, " ");  //get GET or HEAD
    //printf("---%s\n",p);
    r = strtok(NULL, ""); //get identifier HTTP/1.0
    //printf("---%s\n",r);
    identifier = strtok(r, " ");  //get /identifier
    //printf("---%s\n",identifier);
    identifier = strtok(r, "/"); //get identifier
    //printf("---%s\n",identifier);
  char *source = NULL;
  char *response = NULL;

//---------------------if option is empty then go GET----------------------------
  if (strstr(p,"GET") != NULL) {
    printf("get access\n");

//---------------------get the file length----------------------------
    FILE *fp = fopen(identifier, "r");
    if (fp != NULL) { // get the file
      if (fseek(fp, 0L, SEEK_END) == 0) {
          /* Get the size of the file. */
          long bufsize = ftell(fp);
          if (bufsize == -1) { printf("Wrong in getting size\n");}

          /* Allocate our buffer to that size. */
          source = malloc(sizeof(char) * (bufsize + 1));

          /* Go back to the start of the file. */
          if (fseek(fp, 0L, SEEK_SET) != 0) { printf("error get back\n");}

          /* Read the entire file into memory. */
          size_t newLen = fread(source, sizeof(char), bufsize, fp);
          if ( ferror( fp ) != 0 ) {
              fputs("Error reading file", stderr);
          } else {
              source[newLen++] = '\0'; /* Just to be safe. */
          }
      }


  //---------------------send the header first----------------------------
      int length = strlen(source);
      int len = snprintf( NULL, 0, "%d", length );
      char *number;
      number = malloc(len+1);
      sprintf(number, "%d", length);
      response = malloc (strlen("HTTP/1.1 200 OK\n")+strlen("Content-Length:")+strlen(number)+strlen("\r\n\r\n")+strlen(source)+1);
      memset(response,'\0',strlen("HTTP/1.1 200 OK\n")+strlen("Content-Length:")+strlen(number)+strlen("\r\n\r\n")+strlen(source)+1);
      strcat(response,"HTTP/1.1 200 OK\n");
      strcat(response,"Content-Length:");
      strcat(response,number);
      strcat(response,"\r\n\r\n");
      //strcat(response,source);
      printf("%s\n",response);
      if (write(new_fd,response,strlen(response)) < 0) {
        // send GET or HEAD request msg to server
        printf("send error!\n");
        exit(1);
      }

  //--------send the file by using while write in case that file is so large--------
      fp = fopen(identifier, "r");
      for (int i = 0; i < 100; ++i){
        resp[i]='\0';
      }

      while(fgets(resp, 100, fp)) {
        if (write(new_fd,resp,strlen(resp)) < 0) {
            // send GET or HEAD request msg to server
          printf("send error!\n");
          exit(1);
        }
      }
      fclose(fp);
    }else{ //no such file
      response = malloc(strlen("HTTP/1.0 404 Not Found")+strlen("\r\n\r\n")+1);
      memset(response,'\0',strlen("HTTP/1.0 404 Not Found")+strlen("\r\n\r\n")+1);
      strcat(response,"HTTP/1.0 404 Not Found");
      strcat(response,"\r\n\r\n");

//---------------------send the header if no such file----------------------------
      if (write(new_fd,response,strlen(response)) < 0) {
        // send GET or HEAD request msg to server
        printf("send error!\n");
        exit(1);
      }
    }
  }else if(strstr(p,"HEAD") != NULL){  //--------------if option is -h --------
    printf("Head access\n");
    FILE *fp = fopen(identifier, "r");
    if (fp != NULL) {// under HEAD request, have the file
      struct stat buf;
      //buf = malloc(sizeof(struct stat));

//---------------------get the modification time----------------------------
      if(stat(identifier, &buf)==-1){
        perror("stat");
        exit(EXIT_FAILURE);
      }
      //printf("Last file modification:   %s", ctime(&buf.st_mtime));
      response = malloc(strlen("HTTP/1.0 200 OK\n")+strlen("Last-Modified:")+strlen(ctime(&buf.st_mtime))+strlen("\r\n\r\n")+1);
      memset(response,'\0',strlen("HTTP/1.0 200 OK\n")+strlen("Last-Modified:")+strlen(ctime(&buf.st_mtime))+strlen("\r\n\r\n")+1);
      strcat(response,"HTTP/1.0 200 OK\n");
      strcat(response,"Last-Modified:");
      strcat(response,ctime(&buf.st_mtime));
      strcat(response,"\r\n\r\n");
      printf("%s\n", response);
      if (write(new_fd,response,strlen(response)) < 0) {
        // send GET or HEAD request msg to server
        printf("send error!\n");
        exit(1);
      }
    }else{// ------------under request HEAD,  no such file ----------------
      response = malloc(strlen("HTTP/1.0 404 Not Found")+strlen("\r\n\r\n")+1);
      memset(response,'\0',strlen("HTTP/1.0 404 Not Found")+strlen("\r\n\r\n")+1);
      strcat(response,"HTTP/1.0 404 Not Found");
      strcat(response,"\r\n\r\n");
      if (write(new_fd,response,strlen(response)) < 0) {
        // send GET or HEAD request msg to server
        printf("send error!\n");
        exit(1);
      }
    }

  }else{
    printf("wrong request\n");
  }
  free(source);

    close(new_fd);
  }//end of while loop
}
