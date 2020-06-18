#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include "hwlib.h"
#include "socal/socal.h"
#include "socal/hps.h"
#include "socal/alt_gpio.h"
#include "hps_0.h"

#define HW_REGS_BASE ( 0xFF200000 ) // base address for h2f_lw bridge
#define HW_REGS_SPAN ( 0x0020000 )
#define HW_REGS_MASK ( HW_REGS_SPAN - 1 )

int main() {

	void *virtual_base;
	int fd;
	int loop_count;
	int led_direction;
	int led_mask;
	uint32_t *h2f_lw_led_addr;
	uint32_t dipsw;
	uint32_t i2s;
	//int *h2p_lw_i2s_pio;
	volatile uint32_t *h2f_lw_dipsw_addr;
	volatile uint32_t *h2f_lw_i2s_addr;

	// map the address space for the LED registers into user space so we can interact with them.
	// we'll actually map in the entire CSR span of the HPS since we want to access various registers within that span

	if( ( fd = open( "/dev/mem", ( O_RDWR | O_SYNC ) ) ) == -1 ) {
		printf( "ERROR: could not open \"/dev/mem\"...\n" );
		return( 1 );
	}

	virtual_base = mmap( NULL, HW_REGS_SPAN, ( PROT_READ | PROT_WRITE ), MAP_SHARED, fd, HW_REGS_BASE );

	if( virtual_base == MAP_FAILED ) {
		printf( "ERROR: mmap() failed...\n" );
		close( fd );
		return( 1 );
	}
			
	h2f_lw_led_addr = (uint32_t*)(virtual_base + LED_PIO_BASE);
	h2f_lw_dipsw_addr = (uint32_t*)(virtual_base + DIPSW_PIO_BASE);
	h2f_lw_i2s_addr = (uint32_t*)(virtual_base);
	
	// FILE * f_l;
	// FILE * f_r;
	// uint32_t sample;	
	// int left, right;
/*
	printf("Opening file pointers... \n");

	f_l = fopen("MIC_1_LEFT.dat", "w");
	f_r = fopen("MIC_1_RIGHT.dat", "w");
	int i;	
	printf("Recording starting... \n");
	for (i = 0; i < 30000000; i++) {
		if (h2f_lw_i2s_addr[0]) {
			sample = h2f_lw_i2s_addr[1]; // grab sample now that it is ready
			left = (sample & 0x0000FFFF);
			right = (sample & 0xFFFF0000) >> 16;	
			//printf("%x \n",sample);
			fprintf(f_l, "%hd\n", left); // write sample to a file
			fprintf(f_r, "%hd\n", right);
			while(h2f_lw_i2s_addr[0]) {}; // wait until signal goes back low
		} 
	}
	printf("Done recording... \n");
	
	fclose(f_r);
	fclose(f_l);
*/
	while(1) {
		dipsw = *h2f_lw_dipsw_addr;
		printf("%X \n",dipsw);
	}
	
	if( munmap( virtual_base, HW_REGS_SPAN ) != 0 ) {
		printf( "ERROR: munmap() failed...\n" );
		close( fd );
		return( 1 );
	}

	close( fd );

	return( 0 );
}
