/* CSD 304 Computer Networks, Fall 2016
   Lab 2, server
   Team: Aditya , Akhil, Bhishmaraj, Kishore 
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>

#define SERVER_PORT 5432
#define BUF_SIZE 4096


   int main(int argc, char * argv[]){

    struct sockaddr_in sin;
    struct sockaddr_storage client_addr;
    char clientIP[INET_ADDRSTRLEN]; /* For IPv4 addresses */
    socklen_t client_addr_len;
    char buf[BUF_SIZE];
    int len;
    int s;
    char *host;
    struct hostent *hp;
    int udp_buf = 40000;
    int sleep = 100000; 
    if(argc == 2){
     printf("Few arguments ");
     printf("\n Usage : ./*.out (buf_size <= 40000) ( sleep in us) \n");
     exit(1);
   }
   else if(argc == 3){
     udp_buf = atoi(argv[1]);
     sleep = atoi(argv[2]);
   }

   printf("BUF_SIZE = %d SLEEP = %d \n", udp_buf , sleep);
  /* Declarations for file(s) to be sent 
     ...
  */

  /* For inserting delays, use nanosleep()
     struct timespec ... */ 


  /* To get filename from commandline */
  /* if (argc==...) {} */


  /* Create a socket */
     if ((s = socket(PF_INET, SOCK_DGRAM, 0)) < 0) {
      perror("server: socket");
      exit(1);
    }


  /* build address data structure and bind to all local addresses*/
    memset((char *)&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;

  /* If socket IP address specified, bind to it. */
/*  
if(argc == 2) {
    host = argv[1];
    hp = gethostbyname(host);
    if (!hp) {
      fprintf(stderr, "server: unknown host %s\n", host);
      exit(1);
    }
    memcpy((char *)&sin.sin_addr, hp->h_addr, hp->h_length);
  }

  /* Else bind to 0.0.0.0 */
//  else

  sin.sin_addr.s_addr = INADDR_ANY;
  
  sin.sin_port = htons(SERVER_PORT);
  
  if ((bind(s, (struct sockaddr *)&sin, sizeof(sin))) < 0) {
    perror("server: bind");
    exit(1);
  }
  else{
    /* Add code to parse IPv6 addresses */
    inet_ntop(AF_INET, &(sin.sin_addr), clientIP, INET_ADDRSTRLEN);
    printf("Server is listening at address %s:%d\n", clientIP, SERVER_PORT);
  }
  
  printf("Client needs to send \"GET\" to receive the file %s\n", argv[1]);

  
  client_addr_len = sizeof(client_addr);
  
  /* Receive messages from clients*/
  while(len = recvfrom(s, buf, sizeof(buf), 0,
   (struct sockaddr *)&client_addr, &client_addr_len)){

    inet_ntop(client_addr.ss_family,
     &(((struct sockaddr_in *)&client_addr)->sin_addr),
     clientIP, INET_ADDRSTRLEN);

  printf("Server got message from %s: %s [%d bytes]\n", clientIP, buf, len);

    /* Send to client */
    /* Add code to send file if the incoming message is GET */
  
  if(strcmp(buf , "GET\n") == 0){

 /* 	if ((len = sendto(s, buf, sizeof(buf), 0,(struct sockaddr *)&client_addr, client_addr_len)) == -1) {
		  perror("server: sendto");
		  exit(1);
		}*/

     printf("Get recieved \n");
     char* FILE_NAME = "a.txt";
     FILE* fptr = fopen(FILE_NAME , "rb");    	
     fseek(fptr, 0, SEEK_END);
     int length = ftell(fptr);        
     fseek(fptr, 0, SEEK_SET);
     printf("Size of file = %d \n" , length);
     fflush(stdout);

     char* buffer = (char*) malloc(udp_buf);

     int i , ptr = 0;
     memset(buffer, 0, udp_buf );   
     int packet = 1;
     for(i = 0; i < length; i++ , ptr++){

       if(ptr >= udp_buf){
        printf("Sending packet %d , size = %d \n" , (packet++) , udp_buf);
        sendto(s, buffer, udp_buf, 0,(struct sockaddr *)&client_addr, client_addr_len);            
        usleep(sleep);
        ptr = 0;
        memset(buffer, 0, udp_buf );        				
      }

      buffer[ptr] = fgetc(fptr);
    }      
    printf("Sending packet %d , size = %d \n" , (packet++) , ptr);
    sendto(s, buffer, ptr, 0,(struct sockaddr *)&client_addr, client_addr_len);
		//fputs(buffer , stdout);
    printf("Data sent \n");
    free(buffer);
    fclose(fptr);
  }
  strcpy(buf, "BYE");
  sendto(s, buf, 4, 0, 
    (struct sockaddr*)&client_addr, client_addr_len);
  memset(buf, 0, sizeof(buf));
}

  /* Send BYE to signal termination */


}

