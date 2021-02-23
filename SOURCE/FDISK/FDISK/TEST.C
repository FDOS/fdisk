#include <conio.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string.h>   
#include <stdio.h>   
#include <dos.h>   
    
typedef unsigned long ulong;
typedef unsigned int  uint;
typedef unsigned char uchar;
    
struct driveParameter { 
	ulong max_cylinder;
	uint heads;
	uint sectors;
	ulong max_sector;
	};

getDriveParameters(int hd, struct driveParameter *dp)
{               
    union REGS ir;
        
    memset(dp,0, sizeof(*dp));    
    
    ir.h.ah = 0x08;
    ir.h.dl = hd | 0x80;
    int86(0x13, &ir, &ir);
    
    if (ir.x.cflag)
    	return ir.h.ah;
    	
	dp->max_cylinder = ((ir.h.cl >> 6) << 8) |  ir.h.cl; 
	dp->heads     = ir.h.dh;
	dp->sectors   = ir.h.cl & 0x3f;

	return 0;	
}

#define READ  2
#define WRITE 3
    
    
int ReadWrite_Physical_Sectors_CHS(int drive, int cylinder, int head, int sector, int number_of_sectors, void *sector_buffer, int read_or_write)
{
  int error_code;                   

  error_code=biosdisk(read_or_write, drive | 0x80, (int)head, (int)cylinder, (int)sector, number_of_sectors, sector_buffer);

  return(error_code);
}
                      
struct part_entry {
	char bootable;
	char chs_start[3];                      
	char partitiontype;
	char chs_end[3];                      
	ulong start_sect;
	ulong num_sect;
	};

void dump_MBR(char *buffer)
{    
	int i;                     
	struct part_entry* P;
	
	for ( i = 0; i < 4; i++)		
		{
		P = (struct part_entry*)(buffer+0x1be + 16*i);

		if (P->partitiontype) 		
		   printf("%d: 0x%02x %s sect %8lu-%-8lu size %8lu\n", i,
								P->partitiontype,
								P->bootable == 0x80 ? "A" : " ",
								P->start_sect, 
								P->start_sect + P->num_sect-1,
								P->num_sect);
		}	
	printf("\n");
}

	

void set_partition_size(int partno, ulong sectorcount, int HD)
{
	char buffer[4096];
	struct part_entry *P;
	int i, error;
	                                                                                              
	                                                                                              
	if ((error = ReadWrite_Physical_Sectors_CHS(HD, 0,0,1,1,buffer, READ)) != 0)
		{
		printf("error %u reading HD %u\n", error, HD);
		exit(1);
		}                

	// dump_MBR(buffer);	
	

	if (partno >= 0 && partno <= 3)
		{
		P = (struct part_entry*)(buffer+0x1be + 16*partno);
		
		P->num_sect = sectorcount;
		}
	else
		printf("partno (%u) must be 0..3\n", partno), exit(1);		

	dump_MBR(buffer);	
	printf("\n");

	if ((error = ReadWrite_Physical_Sectors_CHS(HD, 0,0,1,1,buffer, WRITE)) != 0)
		{
		printf("error %u writing HD %u\n", error, HD);
		exit(1);
		}                

	exit(0);		

}

check_partition_table(int HD)
{

	char buffer[4096];
	struct part_entry *Pi, *Pj;
	int i, j, error;
	
				asm int 3;

	
	if ((error = ReadWrite_Physical_Sectors_CHS(HD, 0,0,1,1,buffer, READ)) != 0)
		{
		printf("error %u reading HD %u\n", error, HD);
		exit(1);
		}                

	dump_MBR(buffer);
	printf("\n");	

										// check for partition overlap

	for ( i = 0; i < 4; i++)		
		for ( j = 0; j < 4; j++)		
		{                    
		if (i == j)
			continue;
			
		Pi = (struct part_entry*)(buffer+0x1be + 16*i);
		Pj = (struct part_entry*)(buffer+0x1be + 16*j);
		
		if (Pi->start_sect               <= Pj->start_sect  && 
			Pi->start_sect + Pi->num_sect > Pj->start_sect)
			{
			printf("Houston, we have a problem: %d and %d overlap\n",i,j);
			dump_MBR(buffer);
             
             
			{
			struct driveParameter dP;
             
			getDriveParameters(HD, &dP);
			printf(" heads %u sectors %u sectors_per_cyl %u\n", dP.heads, dP.sectors, (dP.heads+1)*dP.sectors);
			}
			
			exit(1);
			}
		}	

	exit(0);		
	
	return 0;

}


int main(int argc, char *argv[])        
{         
	int i, cnt;       
	int HD = 0;   
	uint partno; ulong sectorcount;
	

	argc--, argv++;

	     
	for (i = 0; i < argc; i++)
		strupr(argv[i]);

	if (memcmp(argv[0], "/SETSIZE", 8) == 0)		/*  /SETSIZE:partno:sectorcount:harddisk */
		{         
		if (argc >= 4 &&
			sscanf( argv[1], "%u",  &partno) == 1 &&
			sscanf( argv[2], "%lu", &sectorcount) == 1 &&
			sscanf( argv[3], "%u",  &HD) == 1)
			{
			
			printf("SETSIZE:partition %u sectors %lu disk %u \n", partno, sectorcount, HD);

			set_partition_size(partno, sectorcount, HD);

			exit(0);
			}
		else {
			printf("/SETSIZE:partion(0..3):sectorcount:harddisk(0..7)\n");

			exit(1);			
			}					
		exit(0);
		}   

	else if (memcmp(argv[0], "/CHECK", 6) == 0)		/*  /CHECK harddisk */
		{         
		if (argc >= 2 &&
			sscanf( argv[1], "%u",  &HD) == 1)
			{
			check_partition_table(HD);

			exit(0);
			}
		else {
			printf("/CHECK harddisk(0..7)\n");

			exit(1);			
			}					
		exit(0);
		}   




	printf("?? %u:%u:%lu %u\n", partno, sectorcount, HD, cnt);
		
	printf("usage: /SETSIZE partion(0..3) sectorcount harddisk(0..7)\n");
	printf("       /CHECK   harddisk(0..7)\n");

	return 0;
}
                                                                                                  

