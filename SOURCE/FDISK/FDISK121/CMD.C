/*
// Program:  Free FDISK
// Module:  CMD.C
// Module Description:  Command Line Switch Code Module
//                      All functions that process command line entry are
//                      here.
// Written By:  Brian E. Reifsnyder
// Version:  1.2.1
// Copyright:  1998-2002 under the terms of the GNU GPL, Version 2
*/

/*
CATS message store for cmd.c:

$set 2
1 Syntax Error
2 Program Terminated
3 All flags have been cleared
4 Flag
5 has been cleared
6 Invalid partition size specified
7 Primary DOS Partition not found
8 Extended DOS Partition not found
9 Logical drive number is out of range
10 Partition number is out of range
11 New partition type is out of range
12 Source partition number is out of range
13 Destination partition number is out of range
14 Invalid flag number
15 Flag value is out of range
16 Fixed Disk Drive Status
17 Source partition number is out of range
18 Destination partition number is out of range
19 Invalid drive designation
20 no partition deleted
*/

/*
/////////////////////////////////////////////////////////////////////////////
//  DEFINES
/////////////////////////////////////////////////////////////////////////////
*/

#define CMD

/*
/////////////////////////////////////////////////////////////////////////////
//  INCLUDES
/////////////////////////////////////////////////////////////////////////////
*/

#include <conio.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cmd.h"
#include "fdiskio.h"
#include "main.h"
#include "pcompute.h"
#include "pdiskio.h"
#include "userint2.h"
#include "userint1.h"

/*
/////////////////////////////////////////////////////////////////////////////
//  PROTOTYPES
/////////////////////////////////////////////////////////////////////////////
*/

int Get_Options(char *arguments[],int number_of_arguments);

void Command_Line_Clear_Flag();
void Command_Line_Create_Extended_Partition();
void Command_Line_Create_Logical_DOS_Drive();
void Command_Line_Create_Primary_Partition();
void Command_Line_Delete();
void Command_Line_Info();
void Command_Line_Modify();
void Command_Line_Move();
void Command_Line_Set_Flag();
void Command_Line_Status();
void Command_Line_Swap();
void Command_Line_Test_Flag();
void Command_Line_X();
void Shift_Command_Line_Options(int number_of_places);

/*
/////////////////////////////////////////////////////////////////////////////
//  FUNCTIONS
/////////////////////////////////////////////////////////////////////////////
*/

/* /CLEARFLAG command line option */
void Command_Line_Clear_Flag()
{
  int option_count=1;

  if( (0==strcmp(arg[1].choice,"ALL")) && (arg[0].value!=0) )
    {
    printf("\n");
    printf(catgets(cat,2,1,"Syntax Error"));
    printf("\n");
    printf(catgets(cat,2,2,"Program Terminated"));
    printf("\n");
    exit(1);
    }

  if(0==strcmp(arg[1].choice,"ALL"))
    {
    int index=1;

    option_count=2;

    do
      {
      Clear_Flag(index);

      index++;
      }while(index<=64);

    printf("\n");
    printf(catgets(cat,2,3,"All flags have been cleared"));
    printf("\n");
    }
  else
    {
    Clear_Flag((int)arg[0].value);

    printf("\n");
    printf(catgets(cat,2,4,"Flag"));
    printf(" %d ",arg[0].value);
    printf(catgets(cat,2,5,"has been cleared"));
    printf("\n");
    }

  Shift_Command_Line_Options(option_count);
}

/* /EXT command line options */
void Command_Line_Create_Extended_Partition()
{ 
  int maximum_possible_percentage;

  long maximum_partition_size_in_MB;

  Partition_Table *pDrive = &part_table[flags.drive_number-0x80];

  if(arg[0].value<=0)
    {
    printf("\n");
    printf(catgets(cat,2,6,"Invalid partition size specifed"));
    printf("\n");
    printf(catgets(cat,2,2,"Program Terminated"));
    printf("\n");
    exit(9);
    }

  Determine_Free_Space();

  maximum_partition_size_in_MB = Max_Pri_Part_Size_In_MB(EXTENDED);

  maximum_possible_percentage
   = Convert_To_Percentage(maximum_partition_size_in_MB
   ,pDrive->total_hard_disk_size_in_MB);

//   ,pDrive->ext_part_size_in_MB);

  if(arg[0].extra_value==100)
    {
    /* Set limit on percentage. */
    if(arg[0].value > 100) arg[0].value = 100;
    if(arg[0].value > maximum_possible_percentage)
     arg[0].value = maximum_possible_percentage;

    /* Determine partition size. */
    arg[0].value
     = (arg[0].value * maximum_partition_size_in_MB)
     / maximum_possible_percentage;

/*
    arg[0].value = Convert_Percent_To_MB(arg[0].value
     ,(pDrive->total_cyl+1) );
*/
    }

  Create_Primary_Partition(5,arg[0].value);

  Shift_Command_Line_Options(1);
}

/* /LOG and /LOGO command line options */
void Command_Line_Create_Logical_DOS_Drive()
{
  int maximum_possible_percentage;
  int option_count=1;

//  long ext_part_size;
  long maximum_partition_size_in_MB;

  Partition_Table *pDrive = &part_table[flags.drive_number-0x80];

  if(arg[0].value<=0)
    {
    printf("\n");
    printf(catgets(cat,2,6,"Invalid partition size specifed"));
    printf("\n");
    printf(catgets(cat,2,2,"Program Terminated"));
    printf("\n");
    exit(9);
    }

  Determine_Free_Space();

  maximum_partition_size_in_MB = Max_Log_Part_Size_In_MB();

  maximum_possible_percentage
   = Convert_To_Percentage(maximum_partition_size_in_MB
   ,pDrive->ext_part_size_in_MB);

  if(arg[0].extra_value==100)
    {
    /* Set limit on percentage. */
    if(arg[0].value > 100) arg[0].value = 100;
    if(arg[0].value > maximum_possible_percentage)
     arg[0].value = maximum_possible_percentage;

    /* Determine partition size. */
    arg[0].value
     = (arg[0].value * maximum_partition_size_in_MB)
     / maximum_possible_percentage;



    /* Compute the partition size as a percentage. */

//    arg[0].value = Convert_Percent_To_MB(arg[0].value,ext_part_size);
    }

  if(0!=strcmp(arg[1].choice,"SPEC"))
    {
    /* If no special partition type is defined. */

    Create_Logical_Drive(
     Partition_Type_To_Create(arg[0].value,0),arg[0].value);
    }
  else
    {
    /* If a special partition type is defined. */
    option_count=2;

    Create_Logical_Drive((int)arg[1].value,arg[0].value);
    }

  Shift_Command_Line_Options(option_count);
}

/* /PRI and /PRIO command line options */
void Command_Line_Create_Primary_Partition()
{
  int maximum_possible_percentage;
  int option_count=1;

  long maximum_partition_size_in_MB;

  Partition_Table *pDrive = &part_table[flags.drive_number-0x80];

  if(arg[0].value<=0)
    {
    printf("\n");
    printf(catgets(cat,2,6,"Invalid partition size specifed"));
    printf("\n");
    printf(catgets(cat,2,2,"Program Terminated"));
    printf("\n");
    exit(9);
    }

  Determine_Free_Space();

  maximum_partition_size_in_MB = Max_Pri_Part_Size_In_MB(PRIMARY);

  maximum_possible_percentage
   = Convert_To_Percentage(maximum_partition_size_in_MB
   ,pDrive->total_hard_disk_size_in_MB);


  if(arg[0].extra_value==100)
    {
    /* Set limit on percentage. */
    if(arg[0].value > 100) arg[0].value = 100;
    if(arg[0].value > maximum_possible_percentage)
     arg[0].value = maximum_possible_percentage;

    /* Determine partition size. */
    arg[0].value
     = (arg[0].value * maximum_partition_size_in_MB)
     / maximum_possible_percentage;
    }

  if(0!=strcmp(arg[1].choice,"SPEC"))
    {
    /* If no special partition type is defined. */

    Set_Active_Partition_If_None_Is_Active(
     	Create_Primary_Partition(
     	Partition_Type_To_Create(arg[0].value,0),arg[0].value));
    }
  else
    {
    /* If a special partition type is defined. */
    option_count=2;

    Create_Primary_Partition((int)arg[1].value,arg[0].value);
    }


  Shift_Command_Line_Options(option_count);
}

/* /DELETE command line option */
void Command_Line_Delete()
{
  Partition_Table *pDrive = &part_table[flags.drive_number-0x80];

  /* Delete the primary partition */
  if(0==strcmp(arg[1].choice,"PRI"))
    {
    if (arg[1].value != 0)			/* specified what to delete */
      {
      if (arg[1].value < 1 || arg[1].value >4)
	{
	printf("primary partition # (%ld) must be 1..4\n",(long)arg[1].value); exit(9);}

	Delete_Primary_Partition((int)(arg[1].value - 1));
		}
	else
	  {							/* no number given, delete 'the' partition */
	  int index,found,count;

	  for (count = 0, index = 0; index < 4; index++)
	    {
	    if(IsRecognizedFatPartition(pDrive->pri_part[index].num_type))
	      {
	      count++;
	      found = index;
	      }
	    }
	    if (count == 0)
	     printf("no partition to delete found\n");	/* but continue */
	    else
	      if (count > 1)
		{
		printf("%d primary partitions found, you must specify number to delete\n",count); exit(9);
		}
	      else
		{
		Delete_Primary_Partition(found);
		}
	    }
	} /* end PRI */

  /* Delete the extended partition */
  if(0==strcmp(arg[1].choice,"EXT"))
    {
    int index=3;

    do
      {
      if( ( (flags.version==FOUR)
       || (flags.version==FIVE)
       || (flags.version==SIX) )
       && (pDrive->pri_part[index].num_type==5) )
	{
	Delete_Primary_Partition(index);
	break;
	}
      if( ( (flags.version==W95)
       || (flags.version==W95B)
       || (flags.version==W98) )
       && ( (pDrive->pri_part[index].num_type==5)
       || (pDrive->pri_part[index].num_type==15) ) )
        {
        Delete_Primary_Partition(index);
        break;
        }

      index--;
      }while(index>=0);

    if(index<0)
      {
      printf("\nExtended DOS Partition not found...no partition deleted.\n");
      exit(9);
      }
    }

  /* Delete a Logical DOS Drive */
  if(0==strcmp(arg[1].choice,"LOG"))
    {
    if( (arg[1].value>=1) && (arg[1].value<=23) )
      {
      Delete_Logical_Drive( (int)(arg[1].value-1) );
      }
    else 
      {
      printf("\nLogical drive number  (%d) is out of range...Operation Terminated\n",arg[1].value);
      exit(9);
      }
    }

  /* Delete the partition by the number of the partition */
  if(0==strcmp(arg[1].choice,"NUM"))
    {
    if( (arg[1].value>=1) && (arg[1].value<=4) )
      {
      Delete_Primary_Partition( (int)(arg[1].value-1) );
      }
    else if( (arg[1].value>=5) && (arg[1].value<=28) )
      {
      Delete_Logical_Drive( (int)(arg[1].value-5) );
      }
    else  
      {
      printf("\nPartition number is out of range...Operation Terminated\n");
      exit(9);
      }
    }

  Shift_Command_Line_Options(2);
}

/* /INFO command line option */
void Command_Line_Info()
{
  int option_count=1;

  if(0==strcmp(arg[1].choice,"TECH"))
    {
    option_count=2;

    flags.extended_options_flag=TRUE;
    }

  Display_CL_Partition_Table();

  Shift_Command_Line_Options(option_count);
}

/* /MODIFY command line option */
void Command_Line_Modify()
{
/*
  if((arg[0].value<1) || (arg[0].value>4))
    {
    printf("\nPrimary partition number is out of range...Operation Terminated.\n");
    exit(9);
    }
*/
  if((arg[0].extra_value<=0) || (arg[0].extra_value>255))
    {
    printf("\nNew partition type is out of range...Operation Terminated.\n");
    exit(9);
    }

  Modify_Partition_Type((int)(arg[0].value-1),arg[0].extra_value);

  Shift_Command_Line_Options(1);
}

/* /MOVE command line option */
void Command_Line_Move()
{
  if((arg[0].value<1) || (arg[0].value>4))
    {
    printf("\Source partition number is out of range...Operation Terminated.\n");
    exit(9);
    }

  if((arg[0].extra_value<1) || (arg[0].extra_value>4))
    {
    printf("\Destination partition number is out of range...Operation Terminated.\n");
    exit(9);
    }

  Primary_Partition_Slot_Transfer(MOVE,(int)arg[0].value,arg[0].extra_value);

  Shift_Command_Line_Options(1);
}

/* /SETFLAG command line option */
void Command_Line_Set_Flag()
{
  if( (arg[0].value<1) || (arg[0].value>64) )
    {
    printf("\nInvalid flag number...Operation Terminated.\n");

    exit(9);
    }

  if(arg[0].extra_value==0) arg[0].extra_value=1;

  if( (arg[0].extra_value<1) || (arg[0].extra_value>64) )
    {
    printf("\nFlag value is out of range...Operation Terminated.\n");

    exit(9);
    }

  Set_Flag((int)arg[0].value,arg[0].extra_value);

  printf("\nFlag %d has been set to ",arg[0].value);
  printf("%d.\n",arg[0].extra_value);

  Shift_Command_Line_Options(1);
}

/* /STATUS command line option */
void Command_Line_Status()
{
  flags.monochrome=TRUE;
  textcolor(7);
  Clear_Screen(0);
  Print_Centered(1,"Fixed Disk Drive Status",0);
  Display_All_Drives();

  Shift_Command_Line_Options(1);
}

/* /SWAP command line option */
void Command_Line_Swap()
{
  if((arg[0].value<1) || (arg[0].value>4))
    {
    printf("\nSource partition number is out of range...Operation Terminated.\n");
    exit(9);
    }

  if((arg[0].extra_value<1) || (arg[0].extra_value>4))
    {
    printf("\nDestination partition number is out of range...Operation Terminated.\n");
    exit(9);
    }

  Primary_Partition_Slot_Transfer(SWAP,(int)arg[0].value,arg[0].extra_value);

  Shift_Command_Line_Options(1);
}


/* /TESTFLAG command line option */
void Command_Line_Test_Flag()
{
  int flag;

  flag=Test_Flag((int)arg[0].value);

  if(arg[0].extra_value>0)
    {
    /* If testing for a particular value, return a true or false answer. */
    /* The error level returned will be 20 for false and 21 for true.    */

    if(flag==arg[0].extra_value)
      {
      printf("\nFlag %d is set to ",arg[0].value);
      printf("%d.\n",arg[0].extra_value);

      exit(21);
      }
    else
      {
      printf("\nFlag %d is not set to ",arg[0].value);
      printf("%d.\n",arg[0].extra_value);

      exit(20);
      }
    }
  else
    {
    /* If not testing the flag for a particular value, return the value */
    /* the flag is set to.  The error level returned will be the value  */
    /* of the flag + 30.                                                */

    printf("\nFlag %d is set to ",arg[0].value);
    printf("%d.\n",flag);

    exit( (30+flag) );
    }
}

/* /X command line option */
void Command_Line_X()
{
  int index;

  /* Ask the user if FAT32 is desired. */
  if( (flags.version==W95B) || (flags.version==W98) )
   Ask_User_About_FAT32_Support();

  flags.use_extended_int_13=FALSE;
  index=0;
  do
    {
    part_table[index].ext_int_13=FALSE;
    index++;
    }while(index<8);

  Read_Partition_Tables();
  Interactive_User_Interface();
}

/* Get the command line options */
int Get_Options(char *argv[],int argc)
{
  char *argptr;
  int i;
  int number_of_options = 0;

  flags.drive_number=0x80;
  
  argc--, argv++;		/* absorb program name */
  

  for (i = 0; i < 20; i++)
	{
  	strcpy(arg[i].choice,"");
  	arg[i].value = 0;
  	arg[i].extra_value = 0;
  	}

  
  for ( ; argc > 0; number_of_options++,argv++, argc--)
  	{
	if (number_of_options >= 20)
	  break;

	/* Limit the possible number of options to 20 to prevent an overflow of */
	/* the arg[] structure.                                                 */


	argptr = *argv;


    if(1==strlen(argptr))
      {
      if (!isdigit(*argptr))
			{ printf("<%s> should be a digit; terminated\n",argptr); exit(9);}

      flags.drive_number=(argptr[0]-'0')+127;
      flags.using_default_drive_number=FALSE;
      number_of_options--;
      continue;
      }

    if (*argptr != '-' && *argptr != '/')
			{ printf("<%s> should start with '-' or '/'; terminated\n",argptr); exit(9);}

    argptr++;			/* skip -/ */

				/* parse commandline
					/ARGUMENT:number,number   */
    for (i = 0; ; argptr++,i++)
	{
	if (!isalpha(*argptr) && *argptr != '_')
		break;

	if (i < sizeof(arg[0].choice)-1)
		{
		arg[number_of_options].choice[i]   = toupper(*argptr);
		arg[number_of_options].choice[i+1] = 0;
			}
	}

	if (*argptr == 0)		/* done */
		continue;

	if (*argptr != ':')
			{ printf("<%s> ':' expected; terminated\n",argptr); exit(9);}

	argptr++;

	arg[number_of_options].value = atol(argptr);

    while(isdigit(*argptr))	/* skip number */
	argptr++;


	if (*argptr == 0)		/* done */
		continue;

	if (*argptr != ',')
			{ printf("<%s> ',' expected; terminated\n",argptr); exit(9);}

	argptr++;

	arg[number_of_options].extra_value = (int)atol(argptr);

    while(isdigit(*argptr))	/* skip number */
	argptr++;

	if (*argptr != 0)		/* done */
			{ printf("<%s> expected end of string; terminated\n",argptr); exit(9);}
	}


  /* check to make sure the drive is a legitimate number */
  if( (flags.drive_number < 0x80)
   || (flags.drive_number > flags.maximum_drive_number) )
    {
    printf("\nInvalid drive designation...Operation Terminated.\n");
    exit(5);
    }


  return(number_of_options);
}

void Shift_Command_Line_Options(int number_of_places)
{
  int index;

  for (index=0;index < 20 - number_of_places; index++)
      {
      strcpy(arg[index].choice, arg[index+number_of_places].choice);
      arg[index].value=arg[index+number_of_places].value;
      arg[index].extra_value=arg[index+number_of_places].extra_value;
      }

  number_of_command_line_options-=number_of_places;
}