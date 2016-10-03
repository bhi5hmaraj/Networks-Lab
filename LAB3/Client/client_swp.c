/* CSD 304 Computer Networks, Fall 2016
   Lab 2, client
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

  typedef File_not_found FNF;
  typedef File_info_and_data FID;
  typedef File_request FR;

  int getType(void* ptr) {
    return ((FR*) ptr)->type;
  }

  char* serialize(void* p ) {   
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
    int udp_buf = 50000;                                                                                                                                                                                                                                                                                                            
    fgets(buf, sizeof(buf), stdin);
    buf[BUF_SIZE-1] = '\0';
    len = strlen(buf) + 1;
    FR* fr = malloc(sizeof(FR));
    fr->type = 0;
    fr->filename_size = len;
    fr->filename = buf;
    send(s, serialize(fr) , payload , 0);	
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
        int toACK = -1;
        if(curr == -1) {
          FID* fid = (FID*) deserialize(buffer);
          fptr = fopen(fid->filename , "wb");        
          if(fid->sequence_number == curr + 1) {
            curr++;
            total += fid->block_size;
            writeToFile(fptr , fid->data , fid->block_size);    
            toACK = curr;      
          }
          else if(fid->sequence_number <= curr) {
            toACK = fid->sequence_number;
          }
        }
        else {
          Data* d = (Data*) deserialize(buffer);
          if(d->sequence_number == curr + 1) {
            curr++;
            total += d->block_size;
            writeToFile(fptr , d->data , d->block_size);
            toACK = curr;
          }
          else if(d->sequence_number <= curr) {
            toACK = d->sequence_number;       
          }
        }

        if(toACK >= 0) {
          ACK* a = malloc(sizeof(ACK));
          a->type = 1;
          a->num_sequences = 1;
          a->sequence_no = &toACK;
          char* ser = serialize(a);
          send(s, ser , payload , 0);
        }

      }
    }

  }
