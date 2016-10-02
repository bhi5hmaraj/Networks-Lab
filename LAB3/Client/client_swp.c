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
/*
void* deserialize(char* data) { //TODO : Implement deserialization for all types

}
*/
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
    buf[BUF_SIZE-1] = '\0';
    len = strlen(buf) + 1;
    send(s, buf, len, 0);

  /* get reply, display it or store in a file*/ 
  /* Add code to receive unlimited data and either display the data
     or if specified by the user, store it in the specified file. */
  /*
  struct sockaddr_storage server_addr;
  socklen_t client_addr_len;
    client_addr_len = sizeof(client_addr);
    */
    char * FILE_NAME = "a.txt";
  //FILE* fptr = fopen(FILE_NAME , "wb");
    int size = 40000;
    char * buffer = (char*) malloc(size);
    File_request* fr = malloc(sizeof(File_request));
    int total = 0;
    int flag = 0;
    struct timeval start , end;
    gettimeofday(&start , NULL); 
  // while(1){  	

    memset(buffer, 0, size);   
    int length = recv(s , fr , sizeof(File_request) + 3 , 0);
    printf("Recv len = %d \n", length);
    printf("Type = %d \n", fr->type);
    printf("filename_size = %d \n", fr->filename_size);
    printf("str size = %d\n", strlen(fr->filename));
    printf("File name = %s \n", fr->filename);
    total += length;

    // printf("Inside while recv len %d \n" , length);
    // fflush(stdout);
	//fputs(buffer , stdout);
	//fclose(stdout);
  /*	if(length == 4 && (strcmp(buffer , "BYE") == 0) ){
	  	total -= 4;
     	// fflush(fptr);
	  	// fclose(fptr);
  		// printf("COPIED %d \n" , total);
		gettimeofday(&end , NULL);
		long long time = ((end.tv_sec - start.tv_sec) * 1000000LL) + (end.tv_usec - start.tv_usec);
		// printf("Time : %.8fs \n" , ((double)time/(1000000.0)));
		// printf("Data rate = %.2f KB/s \n " , (double)( (total * 1000.0) / time ));
		fflush(stdout);
  		exit(0);
  	}
  	int i;

  	for(i = 0;i < length;i++)
  		fputc(buffer[i] , stdout);
    
    fflush(stdout);
    
    */
    


  // fputs(buf, stdout);

  }
