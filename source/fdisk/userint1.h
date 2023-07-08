#ifndef USERINT1_H
#define USERINT1_H

#define TEXT_ATTR_NORMAL 0x07

void Clear_Screen( int type );
void Print_At( int column, int row, const char *, ... );
void Color_Print( const char *text );
void Color_Printf( const char *format, ... );
void Color_Print_At( int column, int row, const char *, ... );
void Normal_Print_At( int column, int row, const char *, ... );
void BlinkPrintAt( int column, int row, const char *format, ... );
void Position_Cursor( int column, int row );

int Standard_Menu( int menu );
void Warn_Incompatible_Ext( void );
/*void Display_Label( void );*/
void Interactive_User_Interface( void );
void Menu_Routine( void );
void Exit_Screen( void );

#endif /* USERINT1_H */
