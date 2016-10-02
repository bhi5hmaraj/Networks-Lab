#include <sys/types.h>
#include <unistd.h>

int main(int argc, char const *argv[])
{
	system("cat music.mp3 | vlc -");
	return 0;
}