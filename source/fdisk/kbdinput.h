#ifndef KBDINPUT_H
#define KBDINPUT_H

/* Definitions for the input routine */
enum kbdinput_type {
   YN = 0,
   NUM = 1,
   NUMP = 2,
   ESC = 3,
   ESCR = 4,
   ESCE = 5,
   ESCC = 6,
   CHAR = 7,
   NONE = 8,
   CHARNUM = 9,
   NUMCHAR = 10,
   NUMYN = 11
};

unsigned long Input( int size_of_field, int x_position, int y_position,
                     enum kbdinput_type type, unsigned long min_range,
                     unsigned long max_range, int return_message,
                     long default_value,
                     unsigned long maximum_possible_percentage,
                     char optional_char_1, char optional_char_2 );

#endif /* KBDINPUT_H */
