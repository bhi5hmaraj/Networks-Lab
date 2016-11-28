/* CSD 304 Computer Networks, Fall 2016
   Lab 4, Sender
   Team: Bhishmaraj , Kishore , Aditya , Akhil
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <dirent.h>

#define MC_PORT 5253
#define TCP_PORT 5451
#define BUF_SIZE 15000
#define MAX_PENDING 10
#define STATION_CONST 14

// double speed = ((BUF_SIZE * (8.0 / 1000.0)) / (SLEEP / 1000000.0)); // in kbps (kilo bits per sec)

   char* intToIPAddress(uint32_t IP) {
    uint8_t* ptr = (uint8_t*) &IP;
    char* str = malloc(36); // 32 + 3 + 1
    sprintf(str , "%u.%u.%u.%u" ,  *(ptr + 3), *(ptr + 2) , *(ptr + 1) ,*ptr );
    str[35] = '\0';
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
 char* shortToString(uint16_t num) {
   uint16_t mask = 0xFF;
   char* str = malloc(2);
   str[0] = (num >> 8) & mask;
   str[1] = num & mask;
   fflush(stdout);
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

 typedef struct {
  uint8_t  station_number;
  uint8_t  station_name_size;
  char*    station_name;
  uint32_t multicast_address;
  uint16_t data_port;
  uint16_t info_port;
  uint32_t bit_rate;
}station_info;

typedef struct {
 uint8_t type;
 uint8_t site_name_size;
 char*   site_name;
 uint8_t site_desc_size;
 char*   site_desc;
 uint8_t station_count;
 station_info** station_list;
}site_info;

site_info* deserialize(char* ser) {


 site_info* si = malloc(sizeof(site_info));
 si->type = *(ser++);
 si->site_name_size = *(ser++);
 si->site_name = ser; ser += si->site_name_size;
 si->site_desc_size = *(ser++);
 si->site_desc = ser; ser += si->site_desc_size;
 si->station_count = *(ser++);
 int len = si->station_count;
 int i;
 si->station_list = (station_info**) malloc(sizeof(station_info*) * len);
 for(i=0;i<len;i++) {
   si->station_list[i] = (station_info*) malloc(sizeof(station_info));
   (si->station_list[i])->station_number = *(ser++);
   (si->station_list[i])->station_name_size = *(ser++);
   (si->station_list[i])->station_name = ser; ser += (si->station_list[i])->station_name_size;
   (si->station_list[i])->multicast_address = parseInt(ser); ser += 4;
   (si->station_list[i])->data_port = parseShort(ser); ser += 2;
   (si->station_list[i])->info_port = parseShort(ser); ser += 2;
   (si->station_list[i])->bit_rate  = parseInt(ser); ser += 4; 
 }

 return si;
}

int totalSize;

char* serializeSiteInfo(site_info* si ) {
  char* ser;
  totalSize = 1 + 1 + si->site_name_size + 1 + si->site_desc_size + 1;
  int i , len = (si->station_count);
  for(i = 0;i < len;i++) 
    totalSize += (STATION_CONST + (si->station_list[i])->station_name_size);

  ser = malloc(totalSize);
  if(ser == NULL) {
    printf("Not enough memory \n");
    exit(1);
  }
  char * ans = ser;
  char * start = ser;
  *(ser++) = si->type;
  *(ser++) = si->site_name_size;
  memcpy(ser , si->site_name , si->site_name_size);
  ser += si->site_name_size;
  *(ser++) = si->site_desc_size;
  memcpy(ser , si->site_desc , si->site_desc_size);
  ser += si->site_desc_size;

  *(ser++) = si->station_count;

  for(i = 0;i < len;i++) {
    *(ser++) = (si->station_list[i])->station_number;
    *(ser++) = (si->station_list[i])->station_name_size;
    memcpy(ser , (si->station_list[i])->station_name , (si->station_list[i])->station_name_size); ser += (si->station_list[i])->station_name_size;
    memcpy(ser , intToString((si->station_list[i])->multicast_address) , 4); ser += 4;
    memcpy(ser , shortToString((si->station_list[i])->data_port) , 2); ser += 2;
    memcpy(ser , shortToString((si->station_list[i])->info_port) , 2); ser += 2;
    memcpy(ser , intToString((si->station_list[i])->bit_rate) , 4); ser += 4;
  }
  return ans;
}

uint32_t IPAddressToInt(char* str) {
  int digit[4];
  uint32_t ans = 0;
  sscanf(str , "%d.%d.%d.%d" , digit , digit + 1 , digit + 2 , digit + 3);
  int i;
  for(i=0;i<4;i++) {
    ans |= digit[i];
    if(i < 3)
      ans <<= 8;
  }
  //printf("Input address = %s converted address = %u \n", str , ans);
  //fflush(stdout);
  return ans;
}

  // st no. , st name , MC ADDR , DATA PORT , INFO PORT , bitrate
station_info* createStation(uint8_t station_number , char* station_name ,uint32_t multicast_address , uint16_t data_port , uint16_t info_port , uint32_t bit_rate ) {

  station_info* sti = (station_info*) malloc(sizeof(station_info));
  sti->station_number = station_number;
  sti->station_name_size = strlen(station_name) + 1;
  sti->station_name = station_name;
  sti->multicast_address = multicast_address;
  sti->data_port = data_port;
  sti->info_port = info_port;
  sti->bit_rate = bit_rate;

  return sti;
}

int main(int argc, char * argv[]){
    /* Multicast specific */
  char *mcast_addr; /* multicast address */
  mcast_addr = "230.192.1.10\0";
  site_info* si = (site_info*) malloc(sizeof(site_info));
  si->type = 10;
  si->site_name = "SNU Radio\0";
  si->site_name_size = strlen(si->site_name) + 1;
  si->site_desc = "Description\0";
  si->site_desc_size = strlen(si->site_desc) + 1;

  si->station_count = 4;
  si->station_list = (station_info**) malloc(sizeof(station_info*) * si->station_count);    
  si->station_list[0] = createStation(
    0,
    "Suryan\0",
    IPAddressToInt(mcast_addr),
    MC_PORT,
        0 /* TODO */ ,
        1800 /* TODO */
    );

  si->station_list[1] = createStation(
    1,
    "Hello\0",
    IPAddressToInt(mcast_addr),
    MC_PORT + 1,
      0 /* TODO */ ,
      400 /* TODO */
    );
  si->station_list[2] = createStation (
    2,
    "Camera\0",
    IPAddressToInt(mcast_addr),
    MC_PORT + 2,
    0,
    380
    );
  si->station_list[3] = createStation (
    3,
    "Internet_Radio\0",
    IPAddressToInt(mcast_addr),
    MC_PORT + 3,
    0,
    450 // 430 for Illayaraja
    );
  int i , len = si->station_count;

  for(i=0;i<len;i++) {

    if(fork() == 0) {
      // printf("PID %d , tcp = %d udp = %d \n", getpid() , TCP_PORT , (si->station_list[i])->data_port );      
      int bitRate = (si->station_list[i])->bit_rate; // kbps
      double bufSize = (BUF_SIZE * 8.0) / 1000.0;
      double sleepRequired = (bufSize / (double) bitRate) * 1000000.0;
      int sleep = ((int)sleepRequired);
      double speed = ((BUF_SIZE * (8.0 / 1000.0)) / (sleep / 1000000.0)); // in kbps (kilo bits per sec)

      printf("\nStation = %s created at %s:%d sleepRequired = %lf Bit Rate of song = %d , Bit Rate of Transmission = %lf \n\n", (si->station_list[i])->station_name , mcast_addr , (si->station_list[i])->data_port , sleepRequired, bitRate , speed);
      fflush(stdout);
      int s; /* socket descriptor */
      struct sockaddr_in sin; /* socket struct */
      char buf[BUF_SIZE];
      if ((s = socket(PF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("server: socket");
        exit(1);
      }

          /* build address data structure */
      memset((char *)&sin, 0, sizeof(sin));
      sin.sin_family = AF_INET;
      sin.sin_addr.s_addr = inet_addr(mcast_addr);
      sin.sin_port = htons((si->station_list[i])->data_port);

      if(strcmp((si->station_list[i])->station_name , "Internet_Radio") == 0) {

        if(fork() == 0) {
          // Suryan http://104.238.193.114:7077/;stream.mp3
          // Illayaraja http://66.55.145.43:7446/;
          system("ffmpeg -i 'http://66.55.145.43:7446/;' -f mpegts -y ./Internet_Radio/pipe");
          return 0;
        }        

        FILE* fptr = fopen("./Internet_Radio/pipe" , "rb");
        if(fptr == NULL) {
          printf("Pipe not opened\n");
          fflush(stdout);
          exit(1);
        }
        else {
          printf("Internet Radio Pipe opened\n");
          fflush(stdout);
        }

        while(1) {
          memset(buf, 0, BUF_SIZE);   
          fread(buf , 1 , BUF_SIZE , fptr);
          sendto(s, buf, BUF_SIZE, 0,(struct sockaddr *)&sin, sizeof(sin));
          usleep(sleep);
        }

      }
      else if(strcmp((si->station_list[i])->station_name , "Camera") == 0) {

        if(fork() == 0) {
          system("ffmpeg -f v4l2 -framerate 25 -video_size 480x480 -i /dev/video0 -f mpegts -y ./Camera/pipe");
          return 0;
        }        
        //usleep(100000);
        FILE* fptr = fopen("./Camera/pipe" , "rb");
        if(fptr == NULL) {
          printf("Camera Pipe not opened\n");
          fflush(stdout);
          exit(1);
        }
        else {
          printf("Camera Pipe opened\n");
          fflush(stdout);
        }

        while(1) {
          memset(buf, 0, BUF_SIZE);   
          fread(buf , 1 , BUF_SIZE , fptr);
          sendto(s, buf, BUF_SIZE, 0,(struct sockaddr *)&sin, sizeof(sin));
          usleep(sleep);
        }

      }
      else {

        DIR *directory;
        struct dirent* file;
        FILE * fptr;
        char path[100];
        char file_path[100];
        memset(path , 0 , 100);
        sprintf(path , "./%s/" , (si->station_list[i])->station_name);
        directory = opendir(path);
        if(directory == NULL) {
          printf("Error in directory creation \n");
          exit(1);
        }
        else {
          while ((file=readdir(directory)) != NULL) {
          //printf("d_name = %s\n", file->d_name);
            if(file->d_name[0] != '.') {
              memset(file_path , 0 ,100);
              strcpy(file_path , path);
              strcat(file_path , file->d_name);
              fptr = fopen(file_path , "rb");
              if(fptr != NULL)
                printf("File = %s opened sucessfully\n" , file_path);
              else {
                printf("File not opened\n");
                fflush(stdout);
                exit(1);
              }
              fflush(stdout);

              fseek(fptr, 0, SEEK_END);
              int length = ftell(fptr);
              fseek(fptr, 0, SEEK_SET);
              int remain = length;
              int packet = 1;
              while(remain > 0) {
                memset(buf, 0, BUF_SIZE);   
                int to = BUF_SIZE < remain ? BUF_SIZE : remain;
                fread(buf , 1 , to , fptr);
                remain -= to;
                sendto(s, buf, to, 0,(struct sockaddr *)&sin, sizeof(sin));
                usleep(sleep);
              }
              printf("File = %s streamed sucessfully \n", file_path);
              fclose(fptr);
            }
          }

          closedir(directory);

        }

        close(s);
        return 0;    

      }

    }

  }

  struct sockaddr_in sin;
  /*char buf[MAX_LINE];*/
  int store;
  int s, new_s;
  char str[INET_ADDRSTRLEN];

  bzero((char *)&sin, sizeof(sin));
  sin.sin_family = AF_INET;
  sin.sin_addr.s_addr = INADDR_ANY;
  sin.sin_port = htons(TCP_PORT);
  /* setup passive open */
  if ((s = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
    perror("simplex-talk: socket");
    exit(1);
  }

  inet_ntop(AF_INET, &(sin.sin_addr), str, INET_ADDRSTRLEN);
  printf("Server is using address %s and port %d.\n", str, TCP_PORT);

  if ((bind(s, (struct sockaddr *)&sin, sizeof(sin))) < 0) {
    perror("simplex-talk: bind");
    exit(1);
  }
  else
    printf("Server bind done.\n");

  listen(s, MAX_PENDING);
  /* wait for connection, then receive and print text */
  char* payload = serializeSiteInfo(si);
/*  si = deserialize(payload);
  printf("After deserialize \n");
  int station_len = si->station_count;
  printf("site desc = %s\n", si->site_desc);
  printf("site name = %s\n", si->site_name);
  printf("Station cnt = %d\n", si->station_count);
  printf("Num of stations = %d \n", station_len);
  fflush(stdout);
  for(i=0;i<station_len;i++) {
    printf("Station Number : %d\n", (si->station_list[i])->station_number);
    printf("Station Name : %s\n", (si->station_list[i])->station_name);
    printf("Multicast Address : %s\n", intToIPAddress((si->station_list[i])->multicast_address));
    printf("Data Port : %d\n", (si->station_list[i])->data_port);
    printf("Info Port : %d\n", (si->station_list[i])->info_port);
    printf("Bit Rate : %d\n", (si->station_list[i])->bit_rate);
    fflush(stdout);
  }
  printf("Size of payload = %d \n", totalSize);
*/
  while(1) {
    if ((new_s = accept(s, (struct sockaddr *)&sin, &store)) < 0) {
      perror("simplex-talk: accept");
      exit(1);
    }
    printf("Server Connected to Client \n");
    send(new_s , payload , totalSize , 0);
  }
  close(s); // added new 
  return 0;
}