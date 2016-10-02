#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
  uint16_t parseShort(char* data) {
  uint16_t shot = 0;
  shot |= data[0];
  shot <<= 8;
  shot |= data[1];
  return ntohs(shot);
}
int main(int argc, char const *argv[])
{
	uint16_t bite = 500;
	uint16_t nbite = htons(bite);
	printf("normal = %d htons = %d \n", bite , nbite);	
	uint16_t right = 0xF;
	char data[2];	
	data[1] = (nbite & right);
	nbite >>= 8;
	data[0] = nbite;
	printf("data[0] = %d data[1] = %d\n", data[0] , data[1]);
	printf("parsed data %d\n" , parseShort(data));
	return 0;
}