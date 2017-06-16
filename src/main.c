#include <stdio.h>
#include "h264motion.h"

int main(int argc, char * argv[])
{
	if(argc == 2)
		ffmpeg_init(argv[1]);

	return 0;
}
