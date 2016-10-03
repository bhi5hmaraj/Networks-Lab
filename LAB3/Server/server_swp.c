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
#include <poll.h>
#define SERVER_PORT 5432
#define BUF_SIZE 4096
#define udp_buf 30000
#define FR_CONST   2   
#define ACK_CONST  2
#define FID_CONST  10
#define DATA_CONST 5
#define FNF_CONST  2
   FILE* fptr;
   int remain;
   char buffer[udp_buf];

   uint8_t parseByte(char* data) {
    uint8_t bite = data[0];
    return bite;
  }
  uint16_t parseShort(char* data) {
    uint16_t shot = 0;    
    shot |= ((uint8_t)(data[0]));
    shot <<= 8;    
    shot |= ((uint8_t)(data[1]));            
    return shot;
  }
  
  uint32_t parseInt(char* data) {
    uint32_t Int = 0;
    Int |= ((uint8_t)data[0]);
    Int <<= 8;
    Int |= ((uint8_t)data[1]);
    Int <<= 8;
    Int |= ((uint8_t)data[2]);
    Int <<= 8;
    Int |= ((uint8_t)data[3]);
    return Int;
  }
  char* shortToString(uint16_t num) {
    uint16_t mask = 0xFF;
    char* str = malloc(2);
    // printf("inside shortToString mask = %d \n" , mask);
    // printf("num = %d\n", num);
    str[0] = (num >> 8) & mask;
    str[1] = num & mask;
    // printf("str[0] = %d str[1] = %d\n", str[0] , str[1]);
    fflush(stdout);
    return str; 
  }
  char* intToString(uint32_t num) {
    uint32_t mask = 0xFF;
    char* str = malloc(4);
    // printf("num = %d\n", num);
    str[0] = (num >> 24) & mask;
    str[1] = (num >> 16) & mask;
    str[2] = (num >> 8) & mask;
    str[3] = num & mask;
    /*int i = 0;
    for(i=0;i<4;i++)
      printf("i = %d , str[i] = %d \n", i , str[i]);*/
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

// Size of the non-variable fields

  void stringCopy(char* dest , char* src , int length) {
    int i = 0;
    for(;i<length;i++)
      dest[i] = src[i];
  }

  int payload;
  void* deserialize(char* data) { //TODO : Implement deserialization for all types

    switch(data[0]) {
      case 0 : {
        FR* fr = malloc(sizeof(FR));
        fr->type = parseByte(data);
        fr->filename_size = parseByte(data + 1);
        fr->filename = data + 2;
        return (void*) fr;
      }
      case 1 : {
        ACK* a = malloc(sizeof(ACK));
        a->type = parseByte(data);
        a->num_sequences = parseByte(data + 1);
        a->sequence_no = malloc(2 * a->num_sequences);
        int i;
        for(i=0;i<(a->num_sequences);i++)
          a->sequence_no[i] = parseShort(data + 2 + (2*i));
        return (void*)a;
      }
      case 2 : {
        FID* fid = malloc(sizeof(FID));
        fid->type = parseByte(data);
        fid->sequence_number = parseShort(data + 1);
        fid->filename_size = parseByte(data + 3);
        fid->filename = data + 4;
        fid->file_size = parseInt(data + 4 + fid->filename_size);
        fid->block_size = parseShort(data + 8 + fid->filename_size);
        fid->data = data + 10 + fid->filename_size;
        return (void*) fid;
      }
      case 3 : {
        Data* d = malloc(sizeof(Data));
        d->type = parseByte(data);
        d->sequence_number = parseShort(data + 1);
        d->block_size = parseShort(data + 3);
        d->data = data + 5;
        return (void*)d;
      }
      case 4 : {
        FNF* fnf = malloc(sizeof(FNF));
        fnf->type = parseByte(data);
        fnf->filename_size = parseByte(data + 1);
        fnf->filename = data + 2;
        return (void*)fnf;
      }
      default : {
        printf("Undefined Switch case\n"); 
        exit(1);
      }
    }
    return NULL;
  }
char* serialize(void* p ) {   //TODO : Implement serialization for all types
  char * ser ;
  switch(getType(p)) {
    case 0:{        
      FR* fr  = (FR*) p;
      payload = FR_CONST + fr->filename_size ;
      ser = malloc(payload);
      ser[0] = fr->type;
      ser[1] = fr->filename_size;
      memcpy(ser + 2 , fr->filename , ser[1]);      
      return ser;
    }
    case 1: {
      ACK* a = (ACK*) p;
      payload = ACK_CONST + (2 * a->num_sequences);
      ser = malloc(payload);
      ser[0] = a->type;
      ser[1] = a->num_sequences;
      int i;
      for(i=0;i<(a->num_sequences);i++)
        memcpy((ser + 2 + (2*i)) , shortToString(a->sequence_no[i]) , 2);
      return ser;
    }
    case 2: {
      FID* fid = (FID*) p;
      //printf("Inside serialization filename_size = %d block_size = %d file_size = %d \n", fid->filename_size , fid->block_size , fid->file_size);
      payload = FID_CONST + fid->filename_size + fid->block_size;
      ser = malloc(payload);
      ser[0] = fid->type;
      memcpy(ser + 1 , shortToString(fid->sequence_number) , 2);
      ser[3] = fid->filename_size;
      int f_size = ser[3];
      memcpy(ser + 4 , fid->filename , f_size);
      memcpy(ser + 4 + f_size , intToString(fid->file_size) , 4);
      memcpy(ser + 8 + f_size , shortToString(fid->block_size) ,2);
      memcpy(ser + 10 + f_size , fid->data , fid->block_size);
      return ser;
    }
    case 3: {
      Data* d = (Data*) p;
      payload = DATA_CONST + d->block_size;
      ser = malloc(payload);
      ser[0] = d->type;
      memcpy(ser + 1 , shortToString(d->sequence_number) , 2);
      memcpy(ser + 3 , shortToString(d->block_size) , 2);
      memcpy(ser + 5 , d->data , d->block_size);
      return ser;
    }
    case 4 : {
      FNF* fnf = (FNF*) p;
      payload = FNF_CONST + fnf->filename_size;
      ser = malloc(payload);
      ser[0] = fnf->type;
      ser[1] = fnf->filename_size;
      memcpy(ser + 2 , fnf->filename , fnf->filename_size);
      return ser;
    }
    default: {
      printf("Undefined Switch case \n");
      exit(1);
    }
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
 (struct sockaddr *)&client_addr, &client_addr_len)) {

  inet_ntop(client_addr.ss_family,
   &(((struct sockaddr_in *)&client_addr)->sin_addr),
   clientIP, INET_ADDRSTRLEN);

FR* fr = (FR*) deserialize(buf);
printf("Server got message from %s for File : %s", clientIP , fr->filename);
fflush(stdout);
fptr = fopen(fr->filename, "rb");    	
if(fptr != NULL) {
  fseek(fptr, 0, SEEK_END);
  int length = ftell(fptr);        
  fseek(fptr, 0, SEEK_SET);
  remain = length;
  printf("Size of file = %d \n" , length);
  fflush(stdout);  
  int runs = (length / udp_buf) + (length % udp_buf == 0 ? 0 : 1);
  int consume_len = nextBuffer();  
  printf("consume_len = %d\n", consume_len);
  char* ser;
  FID* fid = malloc(sizeof(FID));
  fid->type = 2;
  fid->sequence_number = 0;
  fid->filename_size = fr->filename_size;
  fid->filename = fr->filename;
  fid->file_size = length;
  fid->block_size = consume_len;
  fid->data = buffer;
  ser = serialize(fid);  
  //printf("ser[3] = %d ser[4] = %d \n", ser[3] , ser[4]);
  int i = 0;
  printf("payload = %d \n", payload);
  struct pollfd fd;
  int res;
  fd.fd = s;
  fd.events = POLLIN;
  int curr = 0;
  while(runs--) {
    sendto(s, ser, payload, 0,(struct sockaddr *)&client_addr, client_addr_len);  
    usleep(10000);
    while(1) {
      res = poll(&fd, 1, 1000); // 1000 ms timeout
      if(res == 0) {
        printf("Timeout\n");
        sendto(s, ser, payload, 0,(struct sockaddr *)&client_addr, client_addr_len);  
        usleep(10000);
      }
      else if(res < 0) {
        printf("Error in poll\n");
        exit(1);
      }
      else {
        char * recv_buf = malloc(4);
        recvfrom(s, recv_buf, 4, 0,(struct sockaddr *)&client_addr, &client_addr_len);
        ACK* a = (ACK*) deserialize(recv_buf);
        if(a->sequence_no[0] == curr) {
          printf("ACK received for curr = %d \n", curr);
          curr++;
          break;
        }
      }
      fflush(stdout);
    }
    if(runs == 0)
      break;
    consume_len = nextBuffer();
    printf("consume_len = %d \n", consume_len);
    Data* d = malloc(sizeof(Data));
    d->type = 3;
    d->sequence_number = curr;
    d->block_size = consume_len;
    d->data = buffer;
    ser = serialize(d);  
    // printf("ser[3] = %d ser[4] = %d \n", ser[3] , ser[4]);  
  }
  fclose(fptr);
  printf("Data sent \n");
}
else { // Implement ACK for fnf also
  printf("File not found in the server\n");
  fflush(stdout);
  buf[0] = 4;
  sendto(s, buf, len, 0,(struct sockaddr *)&client_addr, client_addr_len);    
}
    // free(buffer);
strcpy(buf, "BYE");
sendto(s, buf, 4, 0, 
  (struct sockaddr*)&client_addr, client_addr_len);
memset(buf, 0, sizeof(buf));
}

}  