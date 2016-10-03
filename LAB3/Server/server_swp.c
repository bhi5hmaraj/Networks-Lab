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

// Size of the non-variable fields
#define FR_CONST   2   
#define ACK_CONST  2
#define FID_CONST  10
#define DATA_CONST 5
#define FNF_CONST  2

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
          a->sequence_number[i] = parseShort(data + 2 + (2*i));
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
      ser = malloc(length);
      ser[0] = fr->type;
      ser[1] = fr->filename_size;
      strcpy(ser + 2 , fr->filename);      
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
        strcpy((ser + 2 + (2*i)) , shortToString(a->sequence_no[i]));
      return ser;
    }
    case 2: {
      FID* fid = (FID*) p;
      payload = FID_CONST + fid->filename_size + fid->block_size;
      ser = malloc(payload);
      ser[0] = fid->type;
      strcpy(ser + 1 , shortToString(fid->sequence_number));
      ser[3] = fid->filename_size;
      strcpy(ser + 4 , fid->filename);
      strcpy(ser + 4 + fid->filename_size , intToString(fid->file_size));
      strcpy(ser + 8 + fid->filename_size , shortToString(fid->block_size));
      strcpy(ser + 10 + fid->filename_size , fid->data);
      return ser;
    }
    case 3: {
      Data* d = (Data*) p;
      payload = DATA_CONST + d->block_size;
      ser = malloc(payload);
      ser[0] = d->type;
      strcpy(ser + 1 , shortToString(d->sequence_number));
      strcpy(ser + 3 , shortToString(d->block_size));
      strcpy(ser + 5 , d->data);
      return ser;
    }
    case 4 : {
      FNF* fnf = (FNF*) p;
      payload = FNF_CONST + fnf->filename_size;
      ser = malloc(payload);
      ser[0] = fnf->type;
      ser[1] = fnf->filename_size;
      strcpy(ser + 2 , fnf->filename);
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
 (struct sockaddr *)&client_addr, &client_addr_len)){

  inet_ntop(client_addr.ss_family,
   &(((struct sockaddr_in *)&client_addr)->sin_addr),
   clientIP, INET_ADDRSTRLEN);

FR* fr = (FR*) deserialize(buf);
printf("Server got message from %s for File : %s \n", clientIP , fr->filename);

fptr = fopen(fr->filename , "rb");    	
if(fptr) {
  fseek(fptr, 0, SEEK_END);
  int length = ftell(fptr);        
  fseek(fptr, 0, SEEK_SET);
  remain = length;
  printf("Size of file = %d \n" , length);
  fflush(stdout);  
  int runs = (length / udp_buf) + (length % udp_buf == 0 ? 0 : 1);
  int consume_len = nextBuffer();
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
  int curr = fid->sequence_number;
  while(runs--) {
    sendto(s, ser, payload, 0,(struct sockaddr *)&client_addr, client_addr_len);  
    while(1) {
      struct pollfd fd;
      int res;
      fd.fd = s;
      fd.events = POLLIN;
      res = poll(&fd, 1, 500); // 1000 ms timeout
      if(res == 0) 
        sendto(s, ser, payload, 0,(struct sockaddr *)&client_addr, client_addr_len);  
      else if(res < 0) {
        printf("Error in poll\n");
        exit(1);
      }
      else {
        char * recv_buf = malloc(4);
        recvfrom(s, recv_buf, 4, 0,(struct sockaddr *)&client_addr, &client_addr_len);
        ACK* a = (ACK*) deserialize(recv_buf);
        if(a->sequence_no[0] == curr) {
          curr++;
          break;
        }
      }
    }
    if(runs == 0)
      break;
    consume_len = nextBuffer();
    Data* d = malloc(sizeof(Data));
    d->type = 3;
    d->sequence_number = curr;
    d->block_size = consume_len;
    d->data = buffer;
    ser = serialize(d);    
  }
  printf("Data sent \n");
}
else { // Implement ACK for fnf also
  buf[0] = 4;
  sendto(s, buffer, len, 0,(struct sockaddr *)&client_addr, client_addr_len);    
}
    // free(buffer);
  fclose(fptr);
}
   strcpy(buf, "BYE");
   sendto(s, buf, 4, 0, 
    (struct sockaddr*)&client_addr, client_addr_len);
    memset(buf, 0, sizeof(buf));
}  