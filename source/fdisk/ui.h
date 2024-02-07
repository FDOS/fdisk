#ifndef UI_H
#define UI_H

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

int Create_DOS_Partition_Interface( int type );
int Create_Logical_Drive_Interface( void );
int Delete_Logical_Drive_Interface( void );
int Set_Active_Partition_Interface( void );
int Standard_Menu( int menu );

void Ask_User_About_FAT32_Support( void );
int Inform_About_Trimmed_Disk( void );
void Change_Current_Fixed_Disk_Drive( void );
void Delete_Extended_DOS_Partition_Interface( void );
void Delete_N_DOS_Partition_Interface( void );
void Delete_Primary_DOS_Partition_Interface( void );
void Display_Extended_Partition_Information_SS( void );
void Display_Help_Screen( void );
void Display_Or_Modify_Logical_Drive_Information( void );
void Display_Partition_Information( void );
void Display_Primary_Partition_Information_SS( void );
void Interactive_User_Interface( void );
void List_Partition_Types( void );
void Menu_Routine( void );
void Modify_Extended_Partition_Information( int logical_drive_number );
void Modify_Primary_Partition_Information( int logical_drive );
void Position_Cursor( int row, int column );

#endif /* UI_H */
