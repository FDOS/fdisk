#include <stdio.h>
#include "ansicon.h"
#include "printf.h"

int  main( void )
{
	con_init( 1 );
	con_print( "\33[1;5m\tThis\t\tshould blink!\33[22;25m\nAnd this NOT!\n");
	return 0;	
}
