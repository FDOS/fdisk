#ifndef KBDINPUT_H
#define KBDINPUT_H

unsigned long Input( int size_of_field, int x_position, int y_position,
                     int type, unsigned long min_range,
                     unsigned long max_range, int return_message,
                     long default_value,
                     unsigned long maximum_possible_percentage,
                     char optional_char_1, char optional_char_2 );

#endif /* KBDINPUT_H */
