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
#define udp_buf 40000

   FILE* fptr;
   int remain;
   char buffer[udp_buf];

   uint8_t parseByte(char* data) {
    uint8_t bite = data[0];
    return bite;
  }
  uint16_t parseShort(char* data) {
    uint16_t shot = 0;
    shot |= data[0];
    shot <<= 8;
    shot |= data[1];
    return shot;
  }
  
  uint32_t parseInt(char* data) {
    uint32_t Int = 0;
    Int |= data[0];
    Int <<= 8;
    Int |= data[1];
    Int <<= 8;
    Int |= data[2];
    Int <<= 8;
    Int |= data[3];
    return Int;
  }
  char* shortToString(uint16_t num) {
    uint16_t mask = 0xF;
    char* str = malloc(2);
    str[0] = (num >> 8) & mask;
    str[1] = num & mask;
    return str; 
  }
 char* intToString(uint16_t num) {
    uint32_t mask = 0xF;
    char* str = malloc(4);
    str[0] = (num >> 24) & mask;
    str[1] = (num >> 16) & mask;
    str[2] = (num >> 8) & mask;
    str[4] = num & mask;
    return str; 
  }


  typedef struct {
    uint8_t type;
    uint8_t filename_size;
    char* filename;
  }File_request;

  

  typedef struct {
    uint8_t type;
    uint8_t num_sequences;
    uint16_t* sequence_no;
  }ACK;

  typedef struct {
    uint8_t type;
    uint16_t sequence_number;
    uint8_t filename_size;
    char* filename;
    uint32_t file_size;
    uint16_t block_size;
    char* data;
  }File_info_and_data;

  

  typedef struct {
    uint8_t type;
    uint16_t sequence_number;
    uint16_t block_size;
    char* data;
  }Data;

  typedef struct  {
    uint8_t type;
    uint8_t filename_size;
    char* filename;
  }File_not_found;

typedef File_not_found FNF;
typedef File_info_and_data FID;
typedef File_request FR;

char* serialize(void* p ) {   //TODO : Implement serialization for all types
    char * ser ;
    switch(getType(p)) {
      case 0:
        ser = malloc(sizeof(p) + ((FR*)p)->filename_size);
        ser[0] = ((FR*)p)->type;
        ser[1] = ((FR*)p)->filename_size;
        strcpy(ser + 2 , ((FR*)p)->filename);
        return ser;
        default:
        printf("Undefined \n");
        return NULL;
    }
    return NULL;
}
int getType(void* ptr) {
    return ((FR*) ptr)->type;
}
  int nextBuffer() {
    int i;
    memset(buffer, 0, udp_buf);   
    int to = udp_buf < remain ? udp_buf : remain;
    for(i=0;i<to;i++)
      buffer[i] = fgetc(fptr);
    remain -= i;
    return i;
  }


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
    
    // int sleep = 100000; 
    if(argc == 2){
     printf("Few arguments ");
     printf("\n Usage : ./*.out (buf_size <= 40000) ( sleep in us) \n");
     exit(1);
   }
   else if(argc == 3){
     // udp_buf = atoi(argv[1]);
     // sleep = atoi(argv[2]);
   }


  /* Create a socket */
   if ((s = socket(PF_INET, SOCK_DGRAM, 0)) < 0) {
    perror("server: socket");
    exit(1);
  }


  /* build address data structure and bind to all local addresses*/
  memset((char *)&sin, 0, sizeof(sin));
  sin.sin_family = AF_INET;

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
/*
     printf("Get recieved \n");
     char* FILE_NAME = "a.txt";
     fptr = fopen(FILE_NAME , "rb");    	
     fseek(fptr, 0, SEEK_END);
     int length = ftell(fptr);        
     fseek(fptr, 0, SEEK_SET);
     remain = length;
     printf("Size of file = %d \n" , length);
     fflush(stdout);  
     int packet = 1;
     int runs = (length / udp_buf) + (length % udp_buf == 0 ? 0 : 1);
     while(runs--) {
      int len = nextBuffer();
      sendto(s, buffer, len, 0,(struct sockaddr *)&client_addr, client_addr_len);  

    }
    */
    File_request* fr = malloc(sizeof(File_request) + 3);
    fr->filename_size = 3;
    fr->filename = "a.t";
    // printf("sizeof struct before = %d \n", strlen(fr->filename));
    fr->type = 1;          
    char* ser = serialize(fr );
    int i;
    for(i=0;i<5;i++){
      printf("i = %d , ser[i] = %d\n", i , ser[i]);
    }
    //strcpy(fr->filename , str);
    printf("After copy = %s \n", fr->filename);    
    // printf("sizeof struct after = %d \n", sizeof(File_request));
    sendto(s, fr, sizeof(File_request) + 3, 0,(struct sockaddr *)&client_addr, client_addr_len);    
		//fputs(buffer , stdout);
    printf("Data sent \n");
    // free(buffer);
    // fclose(fptr);
  }
 /* strcpy(buf, "BYE");
  sendto(s, buf, 4, 0, 
    (struct sockaddr*)&client_addr, client_addr_len);
  memset(buf, 0, sizeof(buf));*/
}

  /* Send BYE to signal termination */


}

