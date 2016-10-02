#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#define SERVER_PORT 5400
#define MAX_LINE 256


int main(int argc, char * argv[]){
  FILE *fp;
  struct hostent *hp;
  struct sockaddr_in sin;
  char *host;
  char buf[MAX_LINE];
  int s;
  int len;
  if (argc==2) {
    host = argv[1];
  }
  else {
    fprintf(stderr, "usage: simplex-talk host\n");
    exit(1);
  }
  /* translate host name into peer's IP address */
  hp = gethostbyname(host);
  if (!hp) {
    fprintf(stderr, "simplex-talk: unknown host: %s\n", host);
    exit(1);
  }
  else
    printf("Client's remote host: %s\n", argv[1]);
  /* build address data structure */
  bzero((char *)&sin, sizeof(sin));
  sin.sin_family = AF_INET;
  bcopy(hp->h_addr, (char *)&sin.sin_addr, hp->h_length);
  sin.sin_port = htons(SERVER_PORT);
  /* active open */
  if ((s = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
    perror("simplex-talk: socket");
    exit(1);
  }
  else
    printf("Client created socket.\n");

  if (connect(s, (struct sockaddr *)&sin, sizeof(sin)) < 0)
  {
    perror("simplex-talk: connect");
    close(s);
    exit(1);
  }
  else
    printf("Client connected.\n");
  /* main loop: get and send lines of text */
  recv(s, buf, sizeof(buf), 0);
  fputs(buf,stdout);
  char file[MAX_LINE + 1];
  while (fgets(buf, sizeof(buf), stdin)) {

    buf[MAX_LINE-1] = '\0';    
    strcpy(file , buf);
    len = strlen(buf) + 1;
    if (strcmp(buf, "Bye\n") == 0) {
      printf("Connection closed \n");
      close(s);
      exit(0);
    }

      send(s, buf, len, 0); 
      recv(s, buf, sizeof(buf), 0);
      printf("%s\n", buf);
      fflush(stdout);
      if(strcmp(buf,"OK") == 0) {
        FILE* fptr = fopen(file , "wb");           
        int i;   
            // char filebuf[10000];   // Creating 1MB is causing issues
          
            char* filebuf = (char*) malloc(50000);  //Buffer of size 50KB
            // printf("Size of filebuf = %d \n", sizeof(filebuf));

            if(filebuf == NULL){
              printf("Not enough space \n");
              fflush(stdout);
              close(s);
              exit(1);
            }
            //printf("Im here \n" );
            //fflush(stdout);
            int length = recv(s , filebuf , 50000 , 0);             
            //printf("I want to be here \n" );
            //fflush(stdout);
            
            //printf("Length = %d \n", length);
            for (i = 0; i < length; ++i){
             fputc(filebuf[i] , fptr);
           }
           fflush(fptr);                            
           fclose(fptr);
           free(filebuf);
           fputs("FILE RECIEVED \n" , stdout);            
         
       }
     }   
   }

