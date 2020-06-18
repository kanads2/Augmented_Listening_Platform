#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <inttypes.h>
#include "hwlib.h"
#include "socal.h"
#include "hps.h"
#include "hps_0.h"
#include "alt_types.h"

//#define BUF_SIZE 380000							// Buffer size ~5 seconds
#define HW_REGS_BASE ( ALT_STM_OFST )
#define HW_REGS_SPAN ( 0x04000000 )
#define HW_REGS_MASK ( HW_REGS_SPAN - 1 )
#define DDR_BASE ( 0x00000000 )
#define DDR_SPAN ( 0x3FFFFFFF )
//#define RECORDING_LENGTH 0x001D4C00
#define HALF_BUF 0x000EA600
//#define BUF_SIZE 0x001D4C00
#define SAMPLING_RATE 48000
#define START_ADDRESS 0x01000000
//48000 sampling rate
void grabData(alt_u32* mem, FILE * left, FILE * right, unsigned int start, unsigned int end);
int main() {
	int RECORDING_LENGTH;
	int BUF_SIZE;
	void *virtual_base;
	void *mem_base;
	int fd;
	int choice = -1;
	unsigned int buffer_difference;
	int rec_length = 0;
	int d2 = 0;
	alt_u32 *h2p_lw_audio_addr;

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

	mem_base =  mmap( NULL, DDR_SPAN, ( PROT_READ | PROT_WRITE ), MAP_SHARED, fd, DDR_BASE );

	if( virtual_base == MAP_FAILED ) {
		printf( "ERROR: DDR mmap()  failed...\n" );
		close( fd );
		return( 1 );
	}

	h2p_lw_audio_addr = ((alt_u32) virtual_base) + ( ( alt_u32  )( ALT_LWFPGASLVS_OFST + AVALON_MIRCOPHONE_SYSTEM_0_BASE ) & ( alt_u32)( HW_REGS_MASK ) );


	printf("Enter recording length in seconds:");
	scanf("%d", &rec_length);

	RECORDING_LENGTH = 48000*rec_length;
	BUF_SIZE = RECORDING_LENGTH;

	TOP: alt_write_word(h2p_lw_audio_addr + 1, START_ADDRESS);
	alt_write_word(h2p_lw_audio_addr + 2, RECORDING_LENGTH);
	alt_write_word(h2p_lw_audio_addr, 0x0);

	//rec_length = RECORDING_LENGTH / SAMPLING_RATE;
	buffer_difference = (int)rec_length % 40;


	printf("Audio set to record for %d seconds\n\n", rec_length);

	alt_u32* ddr3 = (alt_u32) mem_base + START_ADDRESS;
	alt_u32* ddr3_2 = (alt_u32) mem_base + START_ADDRESS + (BUF_SIZE * 4);
	alt_u32* ddr3_3 = (alt_u32) mem_base + START_ADDRESS + ((BUF_SIZE * 4) * 2);
	alt_u32* ddr3_4 = (alt_u32) mem_base + START_ADDRESS + ((BUF_SIZE * 4) * 3);
	alt_u32* ddr3_5 = (alt_u32) mem_base + START_ADDRESS + ((BUF_SIZE * 4) * 4);

	FILE * f5_l;
	FILE * f5_r;

	f5_l = fopen("LINE_IN_LEFT.dat", "w");
	f5_r = fopen("LINE_IN_RIGHT.dat", "w");
	if (f5_l == NULL || f5_r == NULL)
	{
		printf("ERROR: OUT.dat not found.\n");
	}

	//printf("DDR3 TEST: %lX\n", alt_read_word(ddr3));
	printf("ENTER 1 TO START RECORDING: ");
	scanf("%d", &d2);
	alt_write_word(h2p_lw_audio_addr, 0x1);
	// int d2;
	// scanf("Press enter to continue: %d", &d2);

	while (alt_read_word(h2p_lw_audio_addr + 2) == 0x0){
		if (alt_read_word(h2p_lw_audio_addr + 3) == 0x1){
			grabData(ddr3_5, f5_l, f5_r, 0x0, HALF_BUF);
			while(alt_read_word(h2p_lw_audio_addr + 3) == 0x1){alt_write_word(h2p_lw_audio_addr + 7, 0x0);}
		}
		if(alt_read_word(h2p_lw_audio_addr + 4) == 0x1){
			grabData(ddr3_5, f5_l, f5_r, HALF_BUF, BUF_SIZE);
			while(alt_read_word(h2p_lw_audio_addr + 4) == 0x1){alt_write_word(h2p_lw_audio_addr + 7, 0x1);}
		}
	}
	alt_write_word(h2p_lw_audio_addr, 0x0);
	if (buffer_difference <= 20){
		grabData(ddr3_5, f5_l, f5_r, 0x0, (HALF_BUF - (buffer_difference * SAMPLING_RATE)));
	} else {
		grabData(ddr3_5, f5_l, f5_r, HALF_BUF, (BUF_SIZE - (buffer_difference * SAMPLING_RATE)));
	}
	fclose(f5_l);	
	fclose(f5_r);
	// printf("After DMA write: %lX\n", alt_read_word(ddr3));
	// printf("After DMA write: %lX\n", alt_read_word(ddr3 + 0x0001));
	// printf("After DMA write: %lX\n", alt_read_word(ddr3 + 0x0002));
	// printf("After DMA write: %lX\n", alt_read_word(ddr3 + 0x0003));

	// alt_write_word(h2p_lw_audio_addr, 0x00000000);

	while (1)
	{
		printf("Enter 2 to save data\nEnter 1 to Re-record (Overwrites data)\nEnter 0 to exit without saving data\n");
		printf("INPUT: ");
		scanf("%d", &choice);
		if (choice == 2)
		{
			FILE * f_l;
			FILE * f_r;
			unsigned int i;
			uint16_t left, right;
			uint32_t fir_left, fir_right;
			///////////////////////////////////////////////////////////////////////////////////////////////////////
			f_l = fopen("MIC_1_LEFT.dat", "w");
			f_r = fopen("MIC_1_RIGHT.dat", "w");

			if (f_l == NULL || f_r == NULL)
			{
				printf("ERROR: OUT.dat not found.\n");
			}

			// Writing to the file

			for(i = 0x00000000; i < BUF_SIZE; i = i + 0x00000001)
			{
				left = (256*(alt_read_word(ddr3 + i) >> 16)) & 0x0000FFFF;
				right = (256*alt_read_word(ddr3 + i)) & 0x0000FFFF;
				fprintf(f_l, "%hd\n", left);
				fprintf(f_r, "%hd\n", right);
			}

			fclose(f_l);
			fclose(f_r); // Closing file
			///////////////////////////////////////////////////////////////////////////////////////////////////////
			FILE * f2_l;
			FILE * f2_r;

			f2_l = fopen("MIC_2_LEFT.dat", "w");
			f2_r = fopen("MIC_2_RIGHT.dat", "w");

			if (f2_l == NULL || f2_r == NULL)
			{
				printf("ERROR: OUT.dat not found.\n");
			}

			// Writing to the file

			for(i = 0x00000000; i < BUF_SIZE; i = i + 0x00000001)
			{
				left = (alt_read_word(ddr3_2 + i) >> 16) & 0x0000FFFF;
				right = alt_read_word(ddr3_2 + i) & 0x0000FFFF;
				fprintf(f2_l, "%hd\n", left);
				fprintf(f2_r, "%hd\n", right);
			}

			fclose(f2_l);	
			fclose(f2_r); // Closing file
			///////////////////////////////////////////////////////////////////////////////////////////////////////
			
			// FILE * f3_l;
			
			// f3_l = fopen("FPGA_FIR_LEFT.dat", "w");

			// if (f3_l == NULL)
			// {
			// 	printf("ERROR: OUT.dat not found.\n");
			// }

			// // Writing to the file

			// for(i = 0x00000000; i < BUF_SIZE; i = i + 0x00000001)
			// {
			// 	fir_left = alt_read_word(ddr3_3 + i) & 0xFFFFFFFF;
			// 	fprintf(f3_l, "%d\n", fir_left);
			// }

			// fclose(f3_l);	// Closing file

			FILE * f3_l;
			FILE * f3_r;

			f3_l = fopen("MIC_3_LEFT.dat", "w");
			f3_r = fopen("MIC_3_RIGHT.dat", "w");

			if (f3_l == NULL || f3_r == NULL)
			{
				printf("ERROR: OUT.dat not found.\n");
			}

			// Writing to the file

			for(i = 0x00000000; i < BUF_SIZE; i = i + 0x00000001)
			{
				left = (alt_read_word(ddr3_3 + i) >> 16) & 0x0000FFFF;
				right = alt_read_word(ddr3_3 + i) & 0x0000FFFF;
				fprintf(f3_l, "%hd\n", left);
				fprintf(f3_r, "%hd\n", right);
			}

			fclose(f3_l);	
			fclose(f3_r); // Closing file

			///////////////////////////////////////////////////////////////////////////////////////////////////////
			
			// FILE * f4_r;

			// f4_r = fopen("FPGA_FIR_RIGHT.dat", "w");
			// if (f4_r == NULL)
			// {
			// 	printf("ERROR: OUT.dat not found.\n");
			// }

			// // Writing to the file

			// for(i = 0x00000000; i < BUF_SIZE; i = i + 0x00000001)
			// {
			// 	fir_right = alt_read_word(ddr3_4 + i) & 0xFFFFFFFF;
			// 	fprintf(f4_r, "%d\n", fir_right);
			// }

			// fclose(f4_r);	// Closing file

			FILE * f4_l;
			FILE * f4_r;

			f4_l = fopen("MIC_4_LEFT.dat", "w");
			f4_r = fopen("MIC_4_RIGHT.dat", "w");

			if (f4_l == NULL || f4_r == NULL)
			{
				printf("ERROR: OUT.dat not found.\n");
			}

			// Writing to the file

			for(i = 0x00000000; i < BUF_SIZE; i = i + 0x00000001)
			{
				left = (alt_read_word(ddr3_4 + i) >> 16) & 0x0000FFFF;
				right = alt_read_word(ddr3_4 + i) & 0x0000FFFF;
				fprintf(f4_l, "%hd\n", left);
				fprintf(f4_r, "%hd\n", right);
			}

			fclose(f4_l);	
			fclose(f4_r); // Closing file

			///////////////////////////////////////////////////////////////////////////////////////////////////////
			/*			
			FILE * f5_l;
			FILE * f5_r;

			f5_l = fopen("LINE_IN_LEFT.dat", "w");
			f5_r = fopen("LINE_IN_RIGHT.dat", "w");
			if (f5_l == NULL || f5_r == NULL)
			{
				printf("ERROR: OUT.dat not found.\n");
			}

			// Writing to the file

			for(i = 0x00000000; i < BUF_SIZE; i = i + 0x00000001)
			{
				left = (alt_read_word(ddr3_5 + i) >> 16) & 0x0000FFFF;
				right = alt_read_word(ddr3_5 + i) & 0x0000FFFF;
				fprintf(f5_l, "%hd\n", left);
				fprintf(f5_r, "%hd\n", right);
			}

			fclose(f5_l);	
			fclose(f5_r); // Closing file
			*/
			///////////////////////////////////////////////////////////////////////////////////////////////////////

			printf("Wrote to files successfully\n");
			alt_write_word(h2p_lw_audio_addr + 3, 0x1);
			alt_write_word(h2p_lw_audio_addr + 3, 0x0);
			break;
		}
		else if (choice == 1){
			alt_write_word(h2p_lw_audio_addr + 3, 0x1);
			alt_write_word(h2p_lw_audio_addr + 3, 0x0);
			goto TOP;
			break;
		}
		else{
			break;
		}
	}

	if( munmap( virtual_base, HW_REGS_SPAN ) != 0 ) {
		printf( "ERROR: munmap() failed...\n" );
		close( fd );
		return( 1 );
	}

	if( munmap( mem_base, DDR_SPAN ) != 0 ) {
		printf( "ERROR: munmap() failed...\n" );
		close( fd );
		return( 1 );
	}

	close( fd );

	return( 0 );
}

void grabData(alt_u32* mem, FILE * left, FILE * right, unsigned int start, unsigned int end){
	uint16_t L, R;
	unsigned int i;
	printf("grabbing lower\n");	// so go until half way and then upper half will start indexing in the upper half of the bufffer weeeeeeeeeeeee taco bell
	for(i = start; i < end; i = i + 0x00000001)
	{
		L = (alt_read_word(mem + i) >> 16) & 0x0000FFFF;
		R = alt_read_word(mem + i) & 0x0000FFFF;
		fprintf(left, "%hd\n", L);
		fprintf(right, "%hd\n", R);
	}
}
