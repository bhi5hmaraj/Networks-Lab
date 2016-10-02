#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#define SERVER_PORT 5400
#define MAX_PENDING 5
#define MAX_LINE 256


int main(){

  struct sockaddr_in sin;
  char buf[MAX_LINE];
  int len;
  int s, new_s;
  char str[INET_ADDRSTRLEN];

  bzero((char *)&sin, sizeof(sin));
  sin.sin_family = AF_INET;
  sin.sin_addr.s_addr = INADDR_ANY;
  sin.sin_port = htons(SERVER_PORT);
  /* setup passive open */
  if ((s = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
    perror("simplex-talk: socket");
    exit(1);
  }
 
  inet_ntop(AF_INET, &(sin.sin_addr), str, INET_ADDRSTRLEN);
  printf("Server is using address %s and port %d.\n", str, SERVER_PORT);

  if ((bind(s, (struct sockaddr *)&sin, sizeof(sin))) < 0) {
    perror("simplex-talk: bind");
    exit(1);
  }
  else
    printf("Server bind done.\n");

  listen(s, MAX_PENDING);
  /* wait for connection, then receive and print text */
  while(1) {

    if ((new_s = accept(s, (struct sockaddr *)&sin, &len)) < 0) {
      perror("simplex-talk: accept");
      exit(1);
    }
    char * message = "Hello\n";
    send(new_s,message,strlen(message) + 1,0);
    printf("Server Listening.\n");
    while (len = recv(new_s, buf, sizeof(buf), 0)){
      
      if(strcmp(buf , "Bye\n") == 0)
        break;

      fputs(buf, stdout);
      int flen = (int)strlen(buf);
      char file_name[flen];
      file_name[flen - 1] = '\0';
      strncpy(file_name , buf , flen - 1);      
      FILE* fptr;
      
      if((fptr = fopen(file_name, "rb")) != NULL){
        fputs("OK\n",stdout);
        message = "OK";
        send(new_s,message,strlen(message) + 1,0);

        fseek(fptr, 0, SEEK_END);
        int length = ftell(fptr);
        fseek(fptr, 0, SEEK_SET);
        char* buffer = (char*) malloc(length);
        printf("Length = %d \n", length);
        int i;
        for(i = 0; i < length; i++){
            buffer[i] = fgetc(fptr);
        }      

        send(new_s, buffer , length , 0);        
        fclose(fptr);        
        fputs("FILE SENT\n",stdout);        
      }
      else{
        fputs("File not found\n",stdout);
        message = "File not found";
        send(new_s,message,strlen(message) + 1,0);
      }
      
    }
    close(new_s);
    fputs("connection closed\n", stdout);
  }
}
