/* BINTOC is published under the terms of the MIT license.
 *
 * Copyright (C) 2023 Bernd Boeckmann
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#include <stdio.h>
#include <stdlib.h>

#define LINE_ENDING "\r\n"

unsigned char *
read_file( const char *filename, long *filesize )
{
   FILE *f;
   unsigned char *buf;

   f = fopen( filename, "rb" );
   if ( !f ) return NULL;

   fseek( f, 0, SEEK_END );
   *filesize = ftell( f );
   fseek( f, 0, SEEK_SET );

   buf = malloc( *filesize );
   if ( !buf ) return NULL;

   if ( fread( buf, 1, *filesize, f ) != *filesize ) {
      free( buf );
      return NULL;
   }

   fclose( f );
   return buf;
}


int
write_data_to_c_file( const unsigned char *data, long size, 
                      const char *filename, const char *ident )
{
   FILE *f;
   long i;

   f = fopen( filename, "w" );
   if ( !f ) return 0;

   fprintf( f, "const unsigned char %s[%ld] = {" LINE_ENDING, ident, size );

   for ( i = 0; i < size; i++ ) {
      fprintf( f, "%d", data[i] );

      if ( i != size - 1 ) {
         fputs( ",", f );
      }
      if ( i % 16 == 15 ) {
         fputs( LINE_ENDING, f );
      }
   }
   fprintf( f, "};" LINE_ENDING );
   fclose( f );

   return 1;
}


int
main( int argc, char **argv )
{
   unsigned char *data;
   long size;

   (void) argc;

   if ( !argv[1] || !argv[2] || !argv[3] ) {
      return EXIT_FAILURE;
   }

   data = read_file( argv[1], &size );
   if ( !data ) {
      return EXIT_FAILURE;
   }

   if ( !write_data_to_c_file( data, size, argv[2], argv[3] ) ) {
      free( data );
      return EXIT_FAILURE;
   }

   free( data );
	return EXIT_SUCCESS;
}
