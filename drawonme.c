#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <linux/fb.h>
#include <sys/mman.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define BUFSIZE 4096
#define PORT 8888
#define WIDTH 320
#define HEIGHT 200
#define BYTESPERPIXEL 4

int main(int argc, char* argv[])
{
	int mem_addr = 0;
	int i, count;

	int fbfd = 0;
	struct fb_var_screeninfo vinfo;
	struct fb_fix_screeninfo finfo;
	long int screensize = 0;
	char *fbp = 0;
	
	struct sockaddr_in si_me, si_other;
	int s, slen=sizeof(si_other);
	ssize_t nread;
	char buf[BUFSIZE];

	system("setterm -cursor off");

	fbfd = open("/dev/fb0", O_RDWR);
	if (fbfd == -1) {
		printf("Error: cannot open framebuffer device.\n");
		return(1);
	}
	printf("The framebuffer device opened.\n");

	if (ioctl(fbfd, FBIOGET_VSCREENINFO, &vinfo)) {
		printf("Error reading variable screen info.\n");
		return(1);
	}

	vinfo.bits_per_pixel = BYTESPERPIXEL;
	vinfo.xres = vinfo.xres_virtual = WIDTH;
	vinfo.yres = vinfo.yres_virtual = HEIGHT;

	if (ioctl(fbfd, FBIOPUT_VSCREENINFO, &vinfo)) {
		printf("Error setting variable information.\n");
		return(-1);
	}

	if (ioctl(fbfd, FBIOGET_FSCREENINFO, &finfo)) {
		printf("Error reading fixed screen info.\n");
		return(1);
	}

	printf("Display info %dx%d, %d bpp\n", vinfo.xres, vinfo.yres, vinfo.bits_per_pixel);

	screensize = finfo.smem_len;

	fbp = (char*)mmap(0, screensize, PROT_READ | PROT_WRITE, MAP_SHARED, fbfd, 0);

	if ((int)fbp == -1) {
		printf("Failed to mmap.\n");
		return(-1);
	}

	printf("Opening socket...\n");
	
	if ((s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
		printf("Error opening socket\n");
		return(-1);
	}

	memset((char *) &si_me, 0, sizeof(si_me));
	si_me.sin_family = AF_INET;
	si_me.sin_port = htons(8888);
	si_me.sin_addr.s_addr = htonl(INADDR_ANY);

	if (bind(s, (struct sockaddr *)&si_me, sizeof(si_me)) == -1) {
		printf("Error binding socket\n");
		return(-1);
	}
 	printf("Starting loop\n");
	memset(fbp, 0x00, screensize);

	while (1) {
		nread = recvfrom(s, buf, BUFSIZE, 0, (struct sockaddr *)&si_other, &slen);
		
		// printf("Read %d bytes\n", nread);	
		i = 0;
		while (i < nread) {
			if ((int)(0x80 & buf[i]) == 0) {
				if (i + 1 < nread) {
					mem_addr = (((int)buf[i]) << 8 | ((int)buf[i+1])) % screensize;
				}	
				i = i + 2;
			}
			else {
				count = 0x7F & (int)buf[i];
				i = i + 1;
				if (mem_addr + count < screensize) {
					memcpy(&fbp[mem_addr], &buf[i], count);
				}
				else {
					memcpy(&fbp[mem_addr], &buf[i], screensize - mem_addr - 1);
				}	
				i = i + count;
				mem_addr = (mem_addr + count) % screensize;
			}
		}
	}

	munmap(fbp, screensize);
	close(fbfd);
	close(s);
	
	return 0;
}
