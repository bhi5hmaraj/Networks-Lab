/* CSD 304 Computer Networks, Fall 2016
   Lab 3, client
   Team: 
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>
#define SERVER_PORT 5432
#define BUF_SIZE 4096
#define FR_CONST   2   
#define ACK_CONST  2
#define FID_CONST  10
#define DATA_CONST 5
#define FNF_CONST  2
#define udp_buf 45000

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

  char* shortToString(uint16_t num) {
    uint16_t mask = 0xFF;
    char* str = malloc(2);
    str[0] = (num >> 8) & mask;
    str[1] = num & mask;
    return str; 
  }
  char* intToString(uint32_t num) {
    uint32_t mask = 0xFF;
    char* str = malloc(4);
    str[0] = (num >> 24) & mask;
    str[1] = (num >> 16) & mask;
    str[2] = (num >> 8) & mask;
    str[3] = num & mask;
    return str; 
  }

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

  typedef File_not_found FNF;
  typedef File_info_and_data FID;
  typedef File_request FR;

  int getType(void* ptr) {
    return ((FR*) ptr)->type;
  }
  int payload;
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
      // printf("Inside serialization filename_size = %d block_size = %d file_size = %d \n", fid->filename_size , fid->block_size , fid->file_size);
      payload = FID_CONST + fid->filename_size + fid->block_size;
      ser = malloc(payload);
      ser[0] = fid->type;
      memcpy(ser + 1 , shortToString(fid->sequence_number) , 2);
      ser[3] = fid->filename_size;
      int f_size = ser[3];
      memcpy(ser + 4 , fid->filename , f_size);
      /*
      char * temp = intToString(fid->file_size);
      int i;
      for(i=0;i<4;i++){
        printf("i = %d , temp[i] = %d \n", i , temp[i]);
        ser[4 + f_size + i] = temp[i];
      }
      temp = shortToString(fid->block_size);
      for(i = 0;i<2;i++){
        printf("i = %d , temp[i] = %d \n", i , temp[i]);
        ser[8 + f_size + i] = temp[i];      
      }
      */

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
  void* deserialize(char* data) { 

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
        //printf("Inside deserialize data[3] = %d data[4] = %d \n", data[3] , data[4]);
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

  void writeToFile(FILE* fptr , char* buffer , int len) {
    int i;
    
    for(i=0;i<len;i++)
      fputc(buffer[i] , fptr);

    fflush(fptr);
  }

  int main(int argc, char * argv[]){

    FILE *fp;
    struct hostent *hp;
    struct sockaddr_in sin;
    char *host;
    char buf[BUF_SIZE];
    int s;
    int len;

    if ((argc==2)||(argc == 3)) {
      host = argv[1];
    }
    else {
      fprintf(stderr, "usage: client serverIP [download_filename(optional)]\n");
      exit(1);
    }

    if(argc == 3) {
      fp = fopen(argv[2], "w");
      if (fp == NULL) {
        fprintf(stderr, "Error opening output file\n");
        exit(1);
      }
    }

  /* translate host name into peer's IP address */
    hp = gethostbyname(host);
    if (!hp) {
      fprintf(stderr, "client: unknown host: %s\n", host);
      exit(1);
    }
    else
      printf("Host %s found!\n", argv[1]);

  /* build address data structure */
    memset((char *)&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;
    memcpy((char *)&sin.sin_addr, hp->h_addr, hp->h_length);
    sin.sin_port = htons(SERVER_PORT);


  /* create socket */
    if ((s = socket(PF_INET, SOCK_DGRAM, 0)) < 0) {
      perror("client: socket");
      exit(1);
    }

  /* active open */
    if (connect(s, (struct sockaddr *)&sin, sizeof(sin)) < 0) {

      perror("client: connect");
      close(s);
      exit(1);
    }
    else{

    // printf("Client connected to %s:%d.\n", argv[1], SERVER_PORT);
    // printf("To play the music, pipe the downlaod file to a player, e.g., ALSA, SOX, VLC: cat recvd_file.wav | vlc -\n"); 
    }



  /* send message to server */        
                                                                                                                                                                                                                                                                                                          
    fgets(buf, sizeof(buf), stdin);    
    buf[strlen(buf) - 1] = '\0';
    FR* fr = malloc(sizeof(FR));
    fr->type = 0;
    fr->filename_size = strlen(buf);
    fr->filename = buf;    
    char* str = serialize(fr);
    send(s, str , payload , 0);	
    int total = 0;
    char buffer[udp_buf];
    int curr = -1;
    FILE* fptr = NULL;
    while(1){    
      memset(buffer, 0, udp_buf);   
      int length = recv(s , buffer , udp_buf , 0);        
      printf("Recv len = %d \n", length);     
      if(length == 4 && (strcmp(buffer , "BYE") == 0) ){
        printf("Bye recieved\n");
        if(fptr) {
          fflush(fptr);
          fclose(fptr);
          printf("COPIED %d \n" , total);
        }
        else {
          printf("FNF \n");
        }
        fflush(stdout);
        return 0;
      }                   
      else if(buffer[0] == 4) {
        printf("File not found in server \n");      
      }
      else {
        int flag = 0;
        uint16_t toACK = 0;
        if(curr == -1) {
          FID* fid = (FID*) deserialize(buffer);
          fptr = fopen(fid->filename , "wb");        
          if(fid->sequence_number == curr + 1) {
            curr++;
            printf("block size = %d \n", fid->block_size);
            fflush(stdout);
            total += fid->block_size;
            //printf("block_size = %d\n", fid->block_size);
            writeToFile(fptr , fid->data , fid->block_size);    
            toACK = curr;    
            flag = 1;  
          }
          else if(fid->sequence_number <= curr) {
            toACK = fid->sequence_number;
            flag = 1;
          }
        }
        else {
          Data* d = (Data*) deserialize(buffer);
          if(d->sequence_number == curr + 1) {
            curr++;
            total += d->block_size;
            printf("block_size = %d\n", d->block_size);
            writeToFile(fptr , d->data , d->block_size);
            toACK = curr;
            flag = 1;
          }
          else if(d->sequence_number <= curr) {
            toACK = d->sequence_number;     
            flag = 1;  
          }
        }

        if(flag) {
          ACK* a = malloc(sizeof(ACK));
          a->type = 1;
          a->num_sequences = 1;
          a->sequence_no = &toACK;
          printf("ACKed = %d\n", a->sequence_no[0]);
          char* ser = serialize(a);
          send(s, ser , payload , 0);
        }

      }
  }
}

