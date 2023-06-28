#ifndef USERINT2_H
#define USERINT2_H

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
void Print_Centered( int y, char *text, int style );

#endif /* USERINT2_H */
