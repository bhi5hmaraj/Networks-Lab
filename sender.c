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

#define MC_PORT 5433
#define TCP_PORT 5453
#define BUF_SIZE 40000
#define MAX_PENDING 10
#define STATION_CONST 14

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

 char* serializeSiteInfo(site_info* si ) {
  char* ser;
      // int stationConst = 1 + 1 + 4 + 2 + 2 + 4;
  int totalSize = 1 + 1 + si->site_name_size + 1 + si->site_desc_size + 1;
  printf("totalSize = %d\n", totalSize);
  int i , len = (si->station_count);
  for(i = 0;i < len;i++) 
    totalSize += (STATION_CONST + (si->station_list[i])->station_name_size);

  ser = malloc(totalSize);
  char * ans = ser;
  *(ser++) = si->type;
  *(ser++) = si->site_name_size;
  memcpy(ser , si->site_name , si->site_name_size);
  ser += si->site_name_size;
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
  return ser;
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
  printf("Input address = %s converted address = %u \n", str , ans);
  fflush(stdout);
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
  si->site_name = "SNU Internet Radio\0";
  si->site_name_size = strlen(si->site_name) + 1;
  si->site_desc = "This is a holder for the site Description\0";
  si->site_desc_size = strlen(si->site_desc) + 1;

  si->station_count = 2;
  si->station_list = (station_info**) malloc(sizeof(station_info*) * si->station_count);    
  si->station_list[0] = createStation(
    0,
    "Suryan\0",
    IPAddressToInt(mcast_addr),
    MC_PORT,
        0 /* TODO */ ,
        320 /* TODO */
    );

  si->station_list[1] = createStation(
    1,
    "Hello\0",
    IPAddressToInt(mcast_addr),
    MC_PORT + 1,
      0 /* TODO */ ,
      320 /* TODO */
    );

  int i , len = si->station_count;
  for(i=0;i<len;i++) {

    if(fork() == 0) {

      printf("Station = %s created at %s:%d\n", (si->station_list[i])->station_name , mcast_addr , (si->station_list[i])->data_port);
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
      DIR *directory;
      struct dirent* file;
      FILE * fptr;
      char* path = malloc(100);
      memset(path , 0 , 100);
      sprintf(path , "./%s/" , (si->station_list[i])->station_name);
      directory = opendir(path);
      if(directory == NULL) {
        printf("Error in directory creation \n");
        exit(1);
      }
      else {
        char* file_path = malloc(100);
        while ((file=readdir(directory)) != NULL) {
          memset(file_path , 0 ,100);
          strcpy(file_path , path);
          strcat(file_path , file->d_name);
          fptr = fopen(file_path , "rb");
          if(fptr != NULL)
            printf("File = %s opened sucessfully\n" , file_path);
          else {
            printf("File not opened\n");
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
            sendto(s, buf, BUF_SIZE, 0,(struct sockaddr *)&sin, sizeof(sin));
            printf("Sent Packet = %d\n", (packet++));
            fflush(stdout);
            usleep(100000);
          }
          fclose(fptr);
        }
        closedir(directory);

      }
      close(s);
      return 0;    
    }

  }

  struct sockaddr_in sin;
  /*char buf[MAX_LINE];*/
  int len;
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
  while(1) {

    if ((new_s = accept(s, (struct sockaddr *)&sin, &len)) < 0) {
      perror("simplex-talk: accept");
      exit(1);
    }
    printf("Server Listening.\n");

    char * message = malloc(200);
    int bitrate = 320; // TODO : automatically find bitrate
    sprintf(message , "Hello\nSite Name : SNU Internet Radio\nSite Description : NULL\nStation Count : 1\nStation Number : 1\nStation Name : Hello FM\nMulticast address : %s\nMulticast UDP Port : %d\nInfo Port : %d\nBitrate :%d\n ", mcast_addr , MC_PORT , 0 , bitrate);
    send(new_s,message,strlen(message),0);

  }
  return 0;
}