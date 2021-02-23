// TaggConsoleServer.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"///part of a template hehe
#include <iostream>//console access
#include <fstream>//fileaccess
#include <string.h>
#include <stdlib.h>
#include <time.h>
///network stuff
#include <winsock2.h>
#include <ws2tcpip.h>
///thread stuff
#include <windows.h>
#include <tchar.h>
#include <strsafe.h>

#pragma comment(lib,"Ws2_32.lib")

using namespace std;

///prototypes
void Save_Settings_Data();
int Load_Settings_Data();
bool Save_Player_Data();
int Load_Player_Data();

void Write_Log_Data(char *Data);

void Memory_Manager_Thread();
bool Get_Player_Data_Lock(int Thread_Id, int Number_Of_Pids, int *Pids);
bool Release_Player_Data_Lock(int Thread_Id);

int setup_network_connection();
int wsastartup();
int listen_for_connections();

int getfreethread();
bool checkifallthreadsarefree();
bool get_player_data_lock(int pid);
bool release_player_data__lock(int pid);

DWORD WINAPI client_connected(LPVOID threaddata);
void client_loggedin(BYTE command_buffer, short int pid);



int strcmpA(char dataa[], char *datab);
int find_player_by_user(char *username);
bool check_premission_for_operation(int check, int pid);
void Get_New_Settings_Data();

///Start Of File Structs
struct svr_settings_struct{
	bool locked;
	char *port;///you can only guess
	int max_players;//max connections to deal with
	int max_it_players;//just a game check so two more players cant tag each other if already it 
	int max_connections;

	bool AutoScanAllowed;
	bool ScanDetectAllowed;
	bool TagBackControl;
	bool UseGPSforplayerlist;

	char current_directory[MAX_PATH];//pointer to data retrieved for database
	char player_database_file[MAX_PATH];
	char settings_file[MAX_PATH];
}svr_settings;

struct tagque_struct
{
	int currentit;
	int target;
};



struct serverdata_struct
{
	bool locked;
	int current_number_of_players;
	int current_number_of_loggedin_players;
	int pending_tags;
	char current_player_db_checksum[32];
}sessiondata;

struct threads_struct
{
	bool isfree;
	int threadid;
	DWORD dwThreadIdArray;
	HANDLE hThreadArray;
	SOCKET ClientSocket;

	player_data_struct *private_player_data;
	bool request_for_player_data_lock;
	bool player_data_lock_is_held;
	int *pids_for_lock;
	int number_of_pids_to_lock;

	serverdata_struct *private_session_data;
	bool request_for_session_data_lock;
	bool session_data_lock_is_held();

	svr_settings_struct *private_server_settings_data;
	bool request_for_settings_lock;
	bool settings_lock_is_held;

}*threads;

threads_struct memory_manager_thread;

///End Of File Structs

///Network Data
WSADATA wsaData;
SOCKET ListenSocket = INVALID_SOCKET;

struct addrinfo *result = NULL, *ptr = NULL, hints;

///settings and playerdb file resource
std::fstream write_log;

int main(int argc, char** argv)
{
	char *logbuffer;
	GetCurrentDirectoryA(MAX_PATH, svr_settings.current_directory);

	Write_Log_Data("********************************");
	Write_Log_Data("*TaggServer V0.3.8 (RC)        *");
	Write_Log_Data("*Codded By:Royce L Whetstine   *");
	Write_Log_Data("*Email:ModdingFox@gmail.com    *");
	Write_Log_Data("********************************");
	Write_Log_Data("*Bugs:                         *");
	Write_Log_Data("*Player Data Base Save Bug     *");
	Write_Log_Data("*Player Data Lock System Bug   *");
	Write_Log_Data("********************************");
	Write_Log_Data("*Features To Come:             *");
	Write_Log_Data("*GPS Player List System        *");///the updater function is in place but the send playerlist function doesn't use it or check why the list is being sent
	Write_Log_Data("********************************");

	int check = 0;
	
	logbuffer = new char[strlen(svr_settings.current_directory) + strlen("Current Path:") + 1];
	logbuffer[0] = NULL;
	strcat(logbuffer, "Current Path:");
	strcat(logbuffer, svr_settings.current_directory);

	Write_Log_Data(logbuffer);
	delete logbuffer;

	svr_settings.settings_file[0] = NULL;
	svr_settings.player_database_file[0] = NULL;
	strcat(svr_settings.settings_file,svr_settings.current_directory);
	strcat(svr_settings.player_database_file,svr_settings.current_directory);
	strcat(svr_settings.settings_file,"\\Settings.ini");
	strcat(svr_settings.player_database_file,"\\Players.132");


	check = Load_Settings_Data();
	if(check != 0){while(0 == 0){}}
	check = Load_Player_Data();
	if(check != 0){while(0 == 0){}}

	check = wsastartup();
	check = listen_for_connections();
	while(0 == 0){}
}

///*****Start Of File IO Functions
void Save_Settings_Data()
{
	Write_Log_Data("Saveing Settings...");

	std::fstream file_access;

	char *numberbuffer;

	char read_char[2];

	file_access.open(svr_settings.settings_file, std::fstream::out);
	file_access.clear();///if not found clear bad flags and maybe later i will create a default config file

	if (file_access.good() == false)
	{
		file_access.clear();
		file_access.close();
		Write_Log_Data("Could Not Save The Settings.ini The Server Might Not Have Write Premissions For The Directory It Is In Just A Thought");
		return;
	}

	read_char[0] = '\n';
	read_char[1] = '\r';

	file_access.write("AutoScan:", sizeof(char)* 9);
	if (svr_settings.AutoScanAllowed == true){ file_access.write("1", 1); }
	else{ file_access.write("0", 1); }
	file_access.write(read_char, 1);

	file_access.write("ScanDetect:", sizeof(char)* 11);
	if (svr_settings.ScanDetectAllowed == true){ file_access.write("1", 1); }
	else{ file_access.write("0", 1); }
	file_access.write(read_char, 1);

	file_access.write("TagBackControl:", sizeof(char)* 15);
	if (svr_settings.TagBackControl == true){ file_access.write("1", 1); }
	else{ file_access.write("0", 1); }
	file_access.write(read_char, 1);

	file_access.write("GPSBasedList:", sizeof(char)* 13);
	if (svr_settings.UseGPSforplayerlist == true){ file_access.write("1", 1); }
	else{ file_access.write("0", 1); }
	file_access.write(read_char, 1);


	file_access.write("port:", sizeof(char)* 5);
	file_access.write(svr_settings.port, strlen(svr_settings.port));
	file_access.write(read_char, 1);

	numberbuffer = new char[MAX_PATH];
	numberbuffer[0] = NULL;
	file_access.write("max_players:", sizeof(char)* 12);
	itoa(svr_settings.max_players, numberbuffer, 10);
	file_access.write(numberbuffer, strlen(numberbuffer));
	file_access.write(read_char, 1);
	delete numberbuffer;

	numberbuffer = new char[MAX_PATH];
	numberbuffer[0] = NULL;
	file_access.write("max_it_players:", sizeof(char)* 15);
	itoa(svr_settings.max_it_players, numberbuffer, 10);
	file_access.write(numberbuffer, strlen(numberbuffer));
	file_access.write(read_char, 1);
	delete numberbuffer;

	numberbuffer = new char[MAX_PATH];
	numberbuffer[0] = NULL;
	file_access.write("max_connections:", sizeof(char)* 16);
	itoa(svr_settings.max_connections, numberbuffer, 10);
	file_access.write(numberbuffer, strlen(numberbuffer));
	file_access.write(read_char, 1);
	delete numberbuffer;

	file_access.write("endofsettings:", sizeof(char)* 14);

	file_access.close();

	Write_Log_Data("Settings Saved");
}

int Load_Settings_Data()///loads player and settings data
{
	Write_Log_Data("Loading Settings...");

	std::fstream file_access;

	char *logbuffer;
	char *numberbuffer; 

	char cmpbuffer[MAX_PATH];
	char read_char[2];
	long buffer_location = 0;
	long file_size = 0;

	cmpbuffer[0] = NULL;//nulls the first char

	logbuffer = new char[strlen("Settings Path:") + strlen(svr_settings.settings_file) + 1];
	logbuffer[0] = NULL;
	strcat(logbuffer, "Settings Path:");
	strcat(logbuffer, svr_settings.settings_file);
	Write_Log_Data(logbuffer);///displays result eventually i would like a  command line option to change the location that the program uses to load the settings.ini
	delete logbuffer;

	file_access.open(svr_settings.settings_file, std::fstream::in);//attempts to open file

	if(file_access.good() == false)///check for filenotfound
	{
		file_access.clear();///if not found clear bad flags and maybe later i will create a default config file
		file_access.close();

		Get_New_Settings_Data();

		file_access.open(svr_settings.settings_file, std::fstream::in);

		if (file_access.good() == false)
		{
			file_access.clear();
			file_access.close();
			Write_Log_Data("An Epic Fail Has Occured Power Off Your Computer And Bang Head On Keyboard Till Your Screen Says QWERTYUIOPASDFGHJKLZXCVBNM EXACTLY!!!");
			return -1;
		}

		file_access.clear();
		file_access.close();

		svr_settings.locked = false;
		return(0);
	}

	Write_Log_Data("Settings File Is Open");
	file_access.seekg(0,ios::end);///set position in file to the end
	file_size = file_access.tellg();///get current position in file
	
	if (file_size == 0)
	{
		file_access.clear();///if not found clear bad flags and maybe later i will create a default config file
		file_access.close();

		Get_New_Settings_Data();

		file_access.open(svr_settings.settings_file, std::fstream::in);

		if (file_access.good() == false)
		{
			file_access.clear();
			file_access.close();
			Write_Log_Data("An Epic Fail Has Occured Power Off Your Computer And Bang Head On Keyboard Till Your Screen Says QWERTYUIOPASDFGHJKLZXCVBNM EXACTLY!!!");
			return -1;
		}

		file_access.clear();
		file_access.close();

		svr_settings.locked = false;
		return(0);
	}

	numberbuffer = new char[MAX_PATH];
	numberbuffer[0] = NULL;
	itoa(file_size, numberbuffer, 10);
	logbuffer = new char[strlen("Current Settings File Size: ") + strlen(numberbuffer) + strlen(" bytes") + 1];
	logbuffer[0] = NULL;
	strcat(logbuffer, "Current Settings File Size: ");
	strcat(logbuffer, numberbuffer);
	strcat(logbuffer, " bytes");
	Write_Log_Data(logbuffer);
	delete logbuffer;
	delete numberbuffer;

	file_access.seekg(0,ios::beg);//jump bact to the begining of file
	file_access.clear();

	while(file_access.eof() == file_access.fail() == file_access.bad() == false)///currentyl opend file is not at end and has no io errors
	{
		file_access.read(read_char,1);///reads a character into the buffer this is not the greatest solution but works fine here i could checked the size of file and loaded line by line and used delimiters to break down the file but i liked this logic better. also could have the file set as a static list of items witch would always be in the same order but thats no fun
		///!!!!!!!!also i do need a handler for settings that are not in the Settings.ini
		if(read_char[0] == ':')///if : if found in the read buffer
		{
			buffer_location = 0;///zeros the current buffer pointer
			if (strcmpA("AutoScan", cmpbuffer) == 0)///only gunna explain this once calls a function to check if static string x is equal to the current string in the cmpbuffer
			{
				while (read_char[0] != '\n' || read_char[0] >= '\r')///loops until a new line is detected
				{
					file_access.read(read_char, 1);///loads a char into the read buffer
					cmpbuffer[buffer_location] = read_char[0];///loade contents of read buffer into the cmpbuffer
					cmpbuffer[(buffer_location + 1)] = NULL;///null terminates the string
					buffer_location++;///incriments the buffer counter
				}
				cmpbuffer[(buffer_location - 1)] = NULL;///null terminates the final result
				if (cmpbuffer[0] == '1'){ svr_settings.AutoScanAllowed = true; }
				else{ svr_settings.AutoScanAllowed = false; }

				logbuffer = new char[strlen("AutoScan Allowed: ") + 4];
				logbuffer[0] = NULL;
				strcat(logbuffer, "AutoScan Allowed: ");
				if (svr_settings.AutoScanAllowed == true){ strcat(logbuffer, "ON"); }
				else{ strcat(logbuffer, "OFF"); }
				Write_Log_Data(logbuffer);
				delete logbuffer;
			}
			else if (strcmpA("ScanDetect", cmpbuffer) == 0)///only gunna explain this once calls a function to check if static string x is equal to the current string in the cmpbuffer
			{
				while (read_char[0] != '\n' || read_char[0] >= '\r')///loops until a new line is detected
				{
					file_access.read(read_char, 1);///loads a char into the read buffer
					cmpbuffer[buffer_location] = read_char[0];///loade contents of read buffer into the cmpbuffer
					cmpbuffer[(buffer_location + 1)] = NULL;///null terminates the string
					buffer_location++;///incriments the buffer counter
				}
				cmpbuffer[(buffer_location - 1)] = NULL;///null terminates the final result
				if (cmpbuffer[0] == '1'){ svr_settings.ScanDetectAllowed = true; }
				else{ svr_settings.ScanDetectAllowed = false; }

				logbuffer = new char[strlen("Scan Detect Allowed: ") + 4];
				logbuffer[0] = NULL;
				strcat(logbuffer, "Scan Detect Allowed: ");
				if (svr_settings.ScanDetectAllowed == true){ strcat(logbuffer, "ON"); }
				else{ strcat(logbuffer, "OFF"); }
				Write_Log_Data(logbuffer);
				delete logbuffer;
			}
			else if (strcmpA("TagBackControl", cmpbuffer) == 0)///only gunna explain this once calls a function to check if static string x is equal to the current string in the cmpbuffer
			{
				while (read_char[0] != '\n' || read_char[0] >= '\r')///loops until a new line is detected
				{
					file_access.read(read_char, 1);///loads a char into the read buffer
					cmpbuffer[buffer_location] = read_char[0];///loade contents of read buffer into the cmpbuffer
					cmpbuffer[(buffer_location + 1)] = NULL;///null terminates the string
					buffer_location++;///incriments the buffer counter
				}
				cmpbuffer[(buffer_location - 1)] = NULL;///null terminates the final result
				if (cmpbuffer[0] == '1'){ svr_settings.TagBackControl = true; }
				else{ svr_settings.TagBackControl = false; }

				logbuffer = new char[strlen("TagBack Control: ") + 4];
				logbuffer[0] = NULL;
				strcat(logbuffer, "TagBack Control: ");
				if (svr_settings.TagBackControl == true){ strcat(logbuffer, "ON"); }
				else{ strcat(logbuffer, "OFF"); }
				Write_Log_Data(logbuffer);
				delete logbuffer;
			}
			else if (strcmpA("GPSBasedList", cmpbuffer) == 0)///only gunna explain this once calls a function to check if static string x is equal to the current string in the cmpbuffer
			{
				while (read_char[0] != '\n' || read_char[0] >= '\r')///loops until a new line is detected
				{
					file_access.read(read_char, 1);///loads a char into the read buffer
					cmpbuffer[buffer_location] = read_char[0];///loade contents of read buffer into the cmpbuffer
					cmpbuffer[(buffer_location + 1)] = NULL;///null terminates the string
					buffer_location++;///incriments the buffer counter
				}
				cmpbuffer[(buffer_location - 1)] = NULL;///null terminates the final result
				if (cmpbuffer[0] == '1'){ svr_settings.UseGPSforplayerlist = true; }
				else{ svr_settings.UseGPSforplayerlist = false; }

				logbuffer = new char[strlen("Use GPS For Scan List: ") + 4];
				logbuffer[0] = NULL;
				strcat(logbuffer, "Use GPS For Scan List: ");
				if (svr_settings.UseGPSforplayerlist == true){ strcat(logbuffer, "ON"); }
				else{ strcat(logbuffer, "OFF"); }
				Write_Log_Data(logbuffer);
				delete logbuffer;
			}
			else if(strcmpA("port", cmpbuffer) == 0)///only gunna explain this once calls a function to check if static string x is equal to the current string in the cmpbuffer
			{
				while(read_char[0] != '\n' || read_char[0] >= '\r')///loops until a new line is detected
				{
					file_access.read(read_char,1);///loads a char into the read buffer
					cmpbuffer[buffer_location] = read_char[0];///loade contents of read buffer into the cmpbuffer
					cmpbuffer[(buffer_location + 1)] = NULL;///null terminates the string
					buffer_location++;///incriments the buffer counter
				}
				cmpbuffer[(buffer_location - 1)] = NULL;///null terminates the final result
				svr_settings.port = new char[strlen(cmpbuffer)];
				svr_settings.port[0] = NULL;
				strcat(svr_settings.port, cmpbuffer);

				logbuffer = new char[strlen("Server Port:") + strlen(svr_settings.port) + 1];
				logbuffer[0] = NULL;
				strcat(logbuffer, "Server Port:");
				strcat(logbuffer, svr_settings.port);
				Write_Log_Data(logbuffer);
				delete logbuffer;
			}
			else if(strcmpA("max_players", cmpbuffer) == 0)
			{
				while(read_char[0] != '\n' || read_char[0] >= '\r')
				{
					file_access.read(read_char,1);
					cmpbuffer[buffer_location] = read_char[0];
					cmpbuffer[(buffer_location + 1)] = NULL;
					buffer_location++;
				}
				cmpbuffer[(buffer_location - 1)] = NULL;
				svr_settings.max_players = atoi(cmpbuffer);

				numberbuffer = new char[MAX_PATH];
				numberbuffer[0] = NULL;
				itoa(svr_settings.max_players, numberbuffer, 10);
				logbuffer = new char[strlen("Max Possible Players:") + strlen(numberbuffer) + 1];
				logbuffer[0] = NULL;
				strcat(logbuffer, "Max Possible Players:");
				strcat(logbuffer, numberbuffer);
				Write_Log_Data(logbuffer);
				delete numberbuffer;
				delete logbuffer;
			}
			else if(strcmpA("max_it_players", cmpbuffer) == 0)
			{
				while(read_char[0] != '\n' || read_char[0] >= '\r')
				{
					file_access.read(read_char,1);
					cmpbuffer[buffer_location] = read_char[0];
					cmpbuffer[(buffer_location + 1)] = NULL;
					buffer_location++;
				}
				cmpbuffer[(buffer_location - 1)] = NULL;
				svr_settings.max_it_players = atoi(cmpbuffer);

				numberbuffer = new char[MAX_PATH];
				numberbuffer[0] = NULL;
				itoa(svr_settings.max_it_players, numberbuffer, 10);
				logbuffer = new char[strlen("Max Possible IT Players:") + strlen(numberbuffer) + 1];
				logbuffer[0] = NULL;
				strcat(logbuffer, "Max Possible IT Players:");
				strcat(logbuffer, numberbuffer);
				Write_Log_Data(logbuffer);
				delete numberbuffer;
				delete logbuffer;
			}
			else if(strcmpA("max_connections", cmpbuffer) == 0)
			{
				while(read_char[0] != '\n' || read_char[0] >= '\r')
				{
					file_access.read(read_char,1);
					cmpbuffer[buffer_location] = read_char[0];
					cmpbuffer[(buffer_location + 1)] = NULL;
					buffer_location++;
				}
				cmpbuffer[(buffer_location - 1)] = NULL;
				svr_settings.max_connections = atoi(cmpbuffer);

				numberbuffer = new char[MAX_PATH];
				numberbuffer[0] = NULL;
				itoa(svr_settings.max_connections, numberbuffer, 10);
				logbuffer = new char[strlen("Max Connections:") + strlen(numberbuffer) + 1];
				logbuffer[0] = NULL;
				strcat(logbuffer, "Max Connections:");
				strcat(logbuffer, numberbuffer);
				Write_Log_Data(logbuffer);
				delete numberbuffer;
				delete logbuffer;

				threads = new threads_struct[svr_settings.max_connections];
				for(int i = 0; i < svr_settings.max_connections; i++)
				{
					threads[i].isfree = true;
					threads[i].threadid = i;
				}
			}
			else if(strcmpA("endofsettings", cmpbuffer) == 0)
			{	
				Write_Log_Data("Settings File Closed");
				file_access.close();///closes the settings file so the fstream or file access resource can be reused
				break;///exits the main loop everything in the settings.ini has been loaded
			}
			buffer_location = 0;///sets the buffer location to zero after its use in the if-else chain
		}
		else///if no : found do this
		{
			cmpbuffer[buffer_location] = read_char[0];///add the contents of the file buffer to the cmpbuffer
			cmpbuffer[(buffer_location + 1)] = NULL;///null terminate the cmpbuffer
			buffer_location++;///incriment the buffer location for next char to be read
		}
	}

	svr_settings.locked = false;
	Write_Log_Data("Settings Loaded");
	return(0);///all data is loaded pass back to caller
}

bool Save_Player_Data()
{
	Write_Log_Data("Saveing Player Data...");

	for (int i = 0; i < svr_settings.max_it_players; i++)
	{
		if (player_data[i].locked == true)
		{
			return false;
		}
	}

	std::fstream file_access;

	char *logbuffer;
	long file_size = 0;
	int pidcount = 0;
	char *data_buffer = new char[sizeof(char)* 33];
	int *int_tf_buffer = new int;

	file_access.open(svr_settings.player_database_file, std::fstream::out);

	if (file_access.good() == false)///check for filenotfound
	{
		file_access.clear();///if not found clear bad flags and maybe later i will create a default config file
		file_access.close();
		Write_Log_Data("Error Accessing Player DataBase For A Save Operation");///oops message hehe
		return false;///back to caller
	}

	*int_tf_buffer = sessiondata.current_number_of_players;
	memcpy(data_buffer, int_tf_buffer, sizeof(int));
	file_access.write(data_buffer, sizeof(int));

	for (int i = 0; i < svr_settings.max_players; i++)
	{
		if (player_data[i].PID >= 0)
		{
			logbuffer = new char[strlen("Saveing User: ") + strlen(player_data[i].USERNAME) + 1];
			logbuffer[0] = NULL;
			strcat(logbuffer, "Saveing User: ");
			strcat(logbuffer, player_data[i].USERNAME);
			Write_Log_Data(logbuffer);
			delete logbuffer;

			*int_tf_buffer = pidcount;
			memcpy(data_buffer, int_tf_buffer, sizeof(int));
			file_access.write(data_buffer, sizeof(int));

			file_access.write(player_data[i].USERNAME, 33);

			data_buffer[0] = player_data[i].status;
			file_access.write(data_buffer, sizeof(char));

			file_access.write(player_data[i].pin, sizeof(char)* 2);

			file_access.write(player_data[i].BTMAC, sizeof(char)* 17);

			*int_tf_buffer = player_data[i].tagback_count;
			memcpy(data_buffer, int_tf_buffer, sizeof(int));
			file_access.write(data_buffer, sizeof(int));

			logbuffer = new char[strlen("Data Saved For User: ") + strlen(player_data[i].USERNAME) + 1];
			logbuffer[0] = NULL;
			strcat(logbuffer, "Data Saved For User: ");
			strcat(logbuffer, player_data[i].USERNAME);
			Write_Log_Data(logbuffer);
			delete logbuffer;

			pidcount++;
		}
	}

	file_access.flush();
	file_access.close();
	Write_Log_Data("Player Data Saved");
	return true;
}

int Load_Player_Data()
{
	Write_Log_Data("Loading Player Data...");

	std::fstream file_access;

	char *logbuffer;
	char *numberbuffer;

	char *data_buffer = new char[sizeof(char) * 33];
	int *char_to_int_buffer = new int;
	long buffer_location = 0;
	long file_size = 0;

	logbuffer = new char[strlen("Player Data Path:") + strlen(svr_settings.player_database_file) + 1];
	logbuffer[0] = NULL;
	strcat(logbuffer, "Player Data Path:");
	strcat(logbuffer, svr_settings.player_database_file);
	Write_Log_Data(logbuffer);
	delete logbuffer;

	file_access.open(svr_settings.player_database_file, std::fstream::in);///open the player database
	
	if(file_access.good() == false)///check for any io errors
	{
		file_access.clear();
		file_access.close();
		file_access.open(svr_settings.player_database_file, std::fstream::out);
		file_access.clear();
		file_access.close();
		file_access.open(svr_settings.player_database_file, std::fstream::in);

		if(file_access.good() == false)///check for any io errors
		{
			file_access.clear();
			file_access.close();
			Write_Log_Data("An Error Has Occoured Creating The Player DataBase File");
			return -1;
		}

		logbuffer = new char[strlen("Created New Player DataBase File In: ") + strlen(svr_settings.player_database_file) + 1];
		logbuffer[0] = NULL;
		strcat(logbuffer, "Created New Player DataBase File In: ");
		strcat(logbuffer, svr_settings.player_database_file);
		Write_Log_Data(logbuffer);
		delete logbuffer;
	}

	Write_Log_Data("Player database Is Open");
	file_access.seekg(0,ios::end);
	file_size = file_access.tellg();

	numberbuffer = new char[MAX_PATH];
	numberbuffer[0] = NULL;
	itoa(file_size, numberbuffer, 10);
	logbuffer = new char[strlen("Current Player Database File Size:") + strlen(numberbuffer) + strlen(" bytes") + 1];
	logbuffer[0] = NULL;
	strcat(logbuffer, "Current Player Database File Size:");
	strcat(logbuffer, numberbuffer);
	strcat(logbuffer, " bytes");
	Write_Log_Data(logbuffer);
	delete numberbuffer;
	delete logbuffer;

	file_access.seekg(0,ios::beg);
	file_access.clear();

	player_data = new player_data_struct[svr_settings.max_players];
	memset(player_data,NULL,sizeof(player_data_struct)*svr_settings.max_players);

	for (int i = 0; i < svr_settings.max_players; i++)
	{
		player_data[i].Tagged_By_PID = -1;
		player_data[i].tagback_count = 0;
		player_data[i].status = 0X01;
		player_data[i].pin[0] = 0x12;
		player_data[i].pin[1] = 0x34;
		player_data[i].PID = -1;
		player_data[i].BTMAC[0] = 'F';
		player_data[i].BTMAC[1] = 'F';
		player_data[i].BTMAC[2] = ':';
		player_data[i].BTMAC[3] = 'F';
		player_data[i].BTMAC[4] = 'F';
		player_data[i].BTMAC[5] = ':';
		player_data[i].BTMAC[6] = 'F';
		player_data[i].BTMAC[7] = 'F';
		player_data[i].BTMAC[8] = ':';
		player_data[i].BTMAC[9] = 'F';
		player_data[i].BTMAC[10] = 'F';
		player_data[i].BTMAC[11] = ':';
		player_data[i].BTMAC[12] = 'F';
		player_data[i].BTMAC[13] = 'F';
		player_data[i].BTMAC[14] = ':';
		player_data[i].BTMAC[15] = 'F';
		player_data[i].BTMAC[16] = 'F';
	}

	if(file_size == 0)
	{
		Write_Log_Data("No Player data Found Creating An Admin Account");
		player_data[0].USERNAME[0] = NULL;
		strcat(player_data[0].USERNAME,"Admin");
		player_data[0].Tagged_By_PID = -1;
		player_data[0].locked = false;
		player_data[0].status = 0x1F;
		player_data[0].pin[0] = 0x00;
		player_data[0].pin[1] = 0x00;
		player_data[0].PID = 0;
		player_data[0].tagback_count = 0;
		player_data[0].BTMAC[0] = 'F';
		player_data[0].BTMAC[1] = 'F';
		player_data[0].BTMAC[2] = ':';
		player_data[0].BTMAC[3] = 'F';
		player_data[0].BTMAC[4] = 'F';
		player_data[0].BTMAC[5] = ':';
		player_data[0].BTMAC[6] = 'F';
		player_data[0].BTMAC[7] = 'F';
		player_data[0].BTMAC[8] = ':';
		player_data[0].BTMAC[9] = 'F';
		player_data[0].BTMAC[10] = 'F';
		player_data[0].BTMAC[11] = ':';
		player_data[0].BTMAC[12] = 'F';
		player_data[0].BTMAC[13] = 'F';
		player_data[0].BTMAC[14] = ':';
		player_data[0].BTMAC[15] = 'F';
		player_data[0].BTMAC[16] = 'F';
		player_data[0].BTMAC[17] = NULL;
		sessiondata.current_number_of_players++;
		Write_Log_Data("Login: Admin");
		Write_Log_Data("Pin: 0000");
	}
	else
	{
	file_access.read(data_buffer,4);
	memcpy(char_to_int_buffer,data_buffer,sizeof(int));
	sessiondata.current_number_of_players = *char_to_int_buffer;

	for(int i = 0; i < sessiondata.current_number_of_players;  i++)
	{
			file_access.read(data_buffer,sizeof(int));
			memcpy(char_to_int_buffer,data_buffer,sizeof(int));
			player_data[i].PID = *char_to_int_buffer;

			file_access.read(player_data[i].USERNAME,sizeof(char) * 33);
			file_access.read(data_buffer,sizeof(char) * 1);
			player_data[i].status = data_buffer[0];
			file_access.read(player_data[i].pin,sizeof(char) * 2);
			file_access.read(player_data[i].BTMAC,sizeof(char) * 17);
			
			file_access.read(data_buffer,sizeof(int));
			memcpy(char_to_int_buffer,data_buffer,sizeof(int));
			player_data[i].tagback_count = *char_to_int_buffer;
	}

	numberbuffer = new char[MAX_PATH];
	numberbuffer[0] = NULL;
	itoa(sessiondata.current_number_of_players, numberbuffer, 10);
	logbuffer = new char[strlen("Loaded ") + strlen(numberbuffer) + strlen(" Players") + 1];
	logbuffer[0] = NULL;
	strcat(logbuffer, "Loaded ");
	strcat(logbuffer, numberbuffer);
	strcat(logbuffer, " Players");
	Write_Log_Data(logbuffer);
	delete numberbuffer;
	delete logbuffer;

	Write_Log_Data("Player database is Closed");
	}

	file_access.close();
	
	Write_Log_Data("Players Data Loaded");
	return 0;
}

void Write_Log_Data(char *Data)
{
	char Time[9];
	char month[3];
	char day[3];
	char year[5];
	char hour[3];
	char min[3];
	char sec[3];

	Time[0] = NULL;
	month[0] = NULL;
	day[0] = NULL;
	year[0] = NULL;
	hour[0] = NULL;
	min[0] = NULL;
	sec[0] = NULL;

	time_t now = time(0);
	struct tm currenttime = *localtime(&now);

	itoa(currenttime.tm_mon, month, 10);
	itoa(currenttime.tm_mday, day, 10);
	itoa((1900 + currenttime.tm_year), year, 10);
	itoa(currenttime.tm_hour, hour, 10);
	itoa(currenttime.tm_min, min, 10);
	itoa(currenttime.tm_sec, sec, 10);

	strcat(Time, hour);
	strcat(Time, ":");
	strcat(Time, min);
	strcat(Time, ":");
	strcat(Time, sec);

	char Temp_Log_Path[MAX_PATH];
	Temp_Log_Path[0] = NULL;

	strcat(Temp_Log_Path, svr_settings.current_directory);
	strcat(Temp_Log_Path, "\\");
	strcat(Temp_Log_Path, month);
	strcat(Temp_Log_Path, ".");
	strcat(Temp_Log_Path, day);
	strcat(Temp_Log_Path, ".");
	strcat(Temp_Log_Path, year);
	strcat(Temp_Log_Path, ".log");

	write_log.open(Temp_Log_Path, std::fstream::app);
	write_log.seekp(ios::end);

	cout << Time << " - " << Data << endl;

	write_log.write(Time, strlen(Time));
	write_log.write(" - ", 3);
	write_log.write(Data, strlen(Data));
	write_log.write("\r\n", 2);

	write_log.flush();
	write_log.close();
}
///******End Of File IO Functions

///Start Of Memory Manager Thread Stuff
void Memory_Manager_Thread()
{
	bool loopistorun = true;
	while (loopistorun)
	{
		for (int i = 0; i < svr_settings.max_connections; i++)
		{
			if (threads[i].request_for_player_data_lock)
			{
				if (threads[i].player_data_lock_is_held == false)
				{
					bool requestnotready = false;

					for (int i1 = 0; i1 < threads[i].number_of_pids_to_lock; i1++)
					{
						if (player_data[threads[i].pids_for_lock[i1]].locked){ requestnotready = true; }
					}

					if (requestnotready == false)
					{
						for (int i1 = 0; i1 < threads[i].number_of_pids_to_lock; i1++)
						{
							player_data[threads[i].pids_for_lock[i1]].locked = true;
						}
						threads[i].request_for_player_data_lock = false;
						threads[i].player_data_lock_is_held = true;
					}
				}
				else
				{
					for (int i1 = 0; i1 < threads[i].number_of_pids_to_lock; i1++)
					{
						player_data[threads[i].pids_for_lock[i1]].locked = false;
					}
					
					threads[i].pids_for_lock = 0;
					delete threads[i].pids_for_lock;

					threads[i].request_for_player_data_lock = false;
					threads[i].player_data_lock_is_held = false;
				}
			}
		}
	}
}

bool Get_Player_Data_Lock(int Thread_Id, int Number_Of_Pids,  int *Pids)
{
	if (!threads[Thread_Id].player_data_lock_is_held)
	{
		threads[Thread_Id].private_player_data = new player_data_struct[Number_Of_Pids];
		threads[Thread_Id].number_of_pids_to_lock = Number_Of_Pids;
		threads[Thread_Id].pids_for_lock = new int[Number_Of_Pids];
		for (int i = 0; i < Number_Of_Pids; i++)
		{
			threads[Thread_Id].pids_for_lock[i] = Pids[i];
		}
		threads[Thread_Id].request_for_player_data_lock = true;
		while (!threads[Thread_Id].player_data_lock_is_held){}
		for (int i = 0; i < Number_Of_Pids; i++)
		{
			threads[Thread_Id].private_player_data[i] = player_data[threads[Thread_Id].pids_for_lock[i]];
		}
		return true;
	}
	else
	{
		return false;
	}
}

bool Release_Player_Data_Lock(int Thread_Id)
{
	if (threads[Thread_Id].player_data_lock_is_held)
	{
		for (int i = 0; i < threads[Thread_Id].number_of_pids_to_lock ; i++)
		{
			player_data[threads[Thread_Id].pids_for_lock[i]] = threads[Thread_Id].private_player_data[i];
		}
		delete threads[Thread_Id].private_player_data;

		threads[Thread_Id].request_for_player_data_lock = true;
		while (threads[Thread_Id].player_data_lock_is_held){}
		return true;
	}
	else
	{
		return false;
	}
}

///Start Of Networking Functions
int wsastartup()
{
	char *logbuffer;
	char *numberbuffer;

	Write_Log_Data("Sever Using WINSOCK");
	int iResult;
	iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
	if(iResult != 0)
	{
		numberbuffer = new char[MAX_PATH];
		numberbuffer[0] = NULL;
		itoa(iResult, numberbuffer, 10);
		logbuffer = new char[strlen("WSAStartup Failed: ") + strlen(numberbuffer) + 1];
		logbuffer[0] = NULL;
		strcat(logbuffer, "WSAStartup Failed: ");
		strcat(logbuffer, numberbuffer);
		Write_Log_Data(logbuffer);
		delete numberbuffer;
		delete logbuffer;

		return 1;
	}
	Write_Log_Data("WSA Started");

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;

	iResult = getaddrinfo(NULL, svr_settings.port, &hints, &result);
	if(iResult != 0)
	{
		numberbuffer = new char[MAX_PATH];
		numberbuffer[0] = NULL;
		itoa(iResult, numberbuffer, 10);
		logbuffer = new char[strlen("getaddrinfo failed: ") + strlen(numberbuffer) + 1];
		logbuffer[0] = NULL;
		strcat(logbuffer, "getaddrinfo failed : ");
		strcat(logbuffer, numberbuffer);
		Write_Log_Data(logbuffer);
		delete numberbuffer;
		delete logbuffer;

		WSACleanup();
		return 2;
	}
	Write_Log_Data("Got Local Address Info");

	ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	if(ListenSocket == INVALID_SOCKET)
	{
		numberbuffer = new char[MAX_PATH];
		numberbuffer[0] = NULL;
		itoa(WSAGetLastError(), numberbuffer, 10);
		logbuffer = new char[strlen("Error Getting Socket: ") + strlen(numberbuffer) + 1];
		logbuffer[0] = NULL;
		strcat(logbuffer, "Error Getting Socket: ");
		strcat(logbuffer, numberbuffer);
		Write_Log_Data(logbuffer);
		delete numberbuffer;
		delete logbuffer;

		freeaddrinfo(result);
		WSACleanup();
		return 3;
	}
	Write_Log_Data("Got A Socket");

	iResult = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
	if(iResult == SOCKET_ERROR)
	{
		numberbuffer = new char[MAX_PATH];
		numberbuffer[0] = NULL;
		itoa(WSAGetLastError(), numberbuffer, 10);
		logbuffer = new char[strlen("Error on binding to socket: ") + strlen(numberbuffer) + 1];
		logbuffer[0] = NULL;
		strcat(logbuffer, "Error on binding to socket: ");
		strcat(logbuffer, numberbuffer);
		Write_Log_Data(logbuffer);
		delete numberbuffer;
		delete logbuffer;

		freeaddrinfo(result);
		closesocket(ListenSocket);
		WSACleanup();
		return 4;
	}
	Write_Log_Data("Socket Bind Success");
	freeaddrinfo(result);

	if(listen(ListenSocket, svr_settings.max_connections) == SOCKET_ERROR)
	{
		numberbuffer = new char[MAX_PATH];
		numberbuffer[0] = NULL;
		itoa(WSAGetLastError(), numberbuffer, 10);
		logbuffer = new char[strlen("Cannot Listen For Connections: ") + strlen(numberbuffer) + 1];
		logbuffer[0] = NULL;
		strcat(logbuffer, "Cannot Listen For Connections: ");
		strcat(logbuffer, numberbuffer);
		Write_Log_Data(logbuffer);
		delete numberbuffer;
		delete logbuffer;

		closesocket(ListenSocket);
		WSACleanup();
		return 5;
	}

	logbuffer = new char[strlen("Listening For Connections On Port:") + strlen(svr_settings.port) + 1];
	logbuffer[0] = NULL;
	strcat(logbuffer, "Listening For Connections On Port:");
	strcat(logbuffer, svr_settings.port);
	Write_Log_Data(logbuffer);
	delete logbuffer;

	return 0;
}

int listen_for_connections()
{
	char *logbuffer;
	char *numberbuffer;

	int error = 0;
	int thread = 0;

	time_t now = time(0);

	struct tm timestruct_lastupdate;
	timestruct_lastupdate = *localtime(&now);
	
	struct tm timestruct_now;

	while(ListenSocket != INVALID_SOCKET)
	{
		now = time(0);
		timestruct_now = *localtime(&now);

		if (timestruct_now.tm_year - timestruct_lastupdate.tm_year >= 1 || timestruct_now.tm_mon - timestruct_lastupdate.tm_mon >= 1 || timestruct_now.tm_mday - timestruct_lastupdate.tm_mday == 1)
		{
			if (checkifallthreadsarefree() == false){ if (Save_Player_Data() == true){ timestruct_lastupdate = *localtime(&now); } }
		}

		thread = getfreethread();
		threads[thread].isfree = false;
		threads[thread].ClientSocket = INVALID_SOCKET;
		threads[thread].ClientSocket = accept(ListenSocket,NULL,NULL);
		if(threads[thread].ClientSocket == INVALID_SOCKET)
		{
			numberbuffer = new char[MAX_PATH];
			numberbuffer[0] = NULL;
			itoa(WSAGetLastError(), numberbuffer, 10);
			logbuffer = new char[strlen("Cound Not Accept Connection: ") + strlen(numberbuffer) + 1];
			logbuffer[0] = NULL;
			strcat(logbuffer, "Cound Not Accept Connection: ");
			strcat(logbuffer, numberbuffer);
			Write_Log_Data(logbuffer);
			delete numberbuffer;
			delete logbuffer;

			closesocket(ListenSocket);
			WSACleanup();
			return 1;
		}
		threads[thread].hThreadArray = CreateThread(NULL,0,client_connected,(LPVOID) &threads[thread],0,&threads[thread].dwThreadIdArray);///create new thread pass the client socket and return to listening for a connection
		//cout << "Client Connected On Thread: " << thread << endl;
		if(error != 0){return 2;}
	}
	return 0;
}
///End Of Networking Functions

///Start Of Thread Management
int getfreethread()
{
	bool found = false;
	int id = NULL;
	while(found == false)
	{
		for(int i = 0; i < svr_settings.max_connections; i++)
		{
			if(threads[i].isfree == true)
			{
				id = i;
				found = true;
				break;
			}
		}
	}
	return id;
}

bool checkifallthreadsarefree()
{

	for (int i = 0; i < svr_settings.max_connections; i++)
	{
		if (threads[i].isfree == false)
		{
			return false;
		}
	}
	return true;
}
///End Of Thread Management

/*Start Of Client Connect Net Menu*/
DWORD WINAPI client_connected(LPVOID threaddataaddress)
{
	threads_struct *threaddata = (threads_struct*)threaddataaddress;
	bool disconnect = false;
	
	SOCKET ClientSocket = threaddata[0].ClientSocket;
	char *data_buffer;
	data_buffer = new char[2];

	while(disconnect == false){
		data_buffer[0] = NULL;
		recv(ClientSocket,data_buffer,1,0);
		data_buffer[1] = NULL;

		switch(data_buffer[0])
		{
		case 0x00:
			//cout << "Client Disconnect On Thread: " threaddata[0].threadid << endl;
			disconnect = true;
			break;
		case 0x01://get pid by user
			pid_request(ClientSocket);
			break;
		case 0x02://get status byte(for current signed in user)
			player_status_reuqest(ClientSocket);
			break;
		case 0x03:
			player_list_request(ClientSocket);
			break;
		case 0x04:
			tag_request(ClientSocket);
			break;
		case 0x05:
			tag_confirm_request(ClientSocket);
			break;
		case 0x06:
			add_player(ClientSocket);
			break;
		case 0x07:
			remove_player(ClientSocket);
			break;
		case 0x08:
			set_player_pin(ClientSocket);
			break;
		case 0x09:
			set_player_status_and_premissions(ClientSocket);
			break;
		case 0x0A:
			Save_Player_Data();
			break;
		case 0x0B:
			update_bt_mac(ClientSocket);
			break;
		case 0x0C:
			Get_User_By_Pid(ClientSocket);
			break;
		case 0x0D:
			Update_Player_Lat_Lon(ClientSocket);
			break;
		case 0x0E:
			Player_BT_Off(ClientSocket);
			break;
		case 0x0F:
			Get_Server_Settings(ClientSocket);
			break;
		default:
			//cout << "Client Disconnect On Thread: " << threaddata[0].threadid << endl;
			disconnect = true;
			break;
		}
	}
	shutdown(ClientSocket,SD_BOTH);
	closesocket(ClientSocket);
	///WSACleanup();
	delete[] data_buffer;
	threaddata[0].isfree = true;
	return 0;
}
/*End Of Client Connect Net Menu*/

/*Start of Network routines*/
void Get_Server_Settings(SOCKET ClientSocket)
{
	char *settings = new char[1];
	settings[0] = 0x00;

	if (svr_settings.AutoScanAllowed == true){ settings[0] = settings[0] | 0x01; }
	if (svr_settings.ScanDetectAllowed == true){ settings[0] = settings[0] | 0x02; }
	if (svr_settings.TagBackControl == true){ settings[0] = settings[0] | 0x04; }
	if (svr_settings.UseGPSforplayerlist == true){ settings[0] = settings[0] | 0x08; }

	send(ClientSocket, settings, 1, 0);

	Write_Log_Data("Sent Server Settings Data");
}

void Player_BT_Off(SOCKET ClientSocket)
{
	//the app will check if 900000ms has passed since boot and if so send this request
	//the app will also send this if bt turns off
	//if the app cannot reach the server it will store a flag that states that nexttime it connects it will send this request
	char *logbuffer;

	char *data_buffer = new char[sizeof(int)+2];
	int *piddata = new int;


	recv(ClientSocket, data_buffer, sizeof(int)+2, 0);
	memcpy(piddata, data_buffer, sizeof(int));

	while (player_data[*piddata].locked){}
	player_data[*piddata].locked = true;

	if (player_data[*piddata].pin[0] == data_buffer[sizeof(int)] && player_data[*piddata].pin[1] == data_buffer[sizeof(int)+1])
	{
		logbuffer = new char[strlen("Tagback Count +1: ") + strlen(player_data[*piddata].USERNAME) + 1];
		logbuffer[0] = NULL;
		strcat(logbuffer, "Tagback Count +1: ");
		strcat(logbuffer, player_data[*piddata].USERNAME);
		Write_Log_Data(logbuffer);
		delete logbuffer;

		player_data[*piddata].tagback_count++;

		data_buffer[0] = 0x00;
		send(ClientSocket, data_buffer, sizeof(char), 0);
	}
	else
	{
		logbuffer = new char[strlen("Could Not Auth Player For Tagback Count +1: ") + strlen(player_data[*piddata].USERNAME) + 1];
		logbuffer[0] = NULL;
		strcat(logbuffer, "Could Not Auth Player For Tagback Count +1: ");
		strcat(logbuffer, player_data[*piddata].USERNAME);
		Write_Log_Data(logbuffer);
		delete logbuffer;

		data_buffer[0] = 0x01;
		send(ClientSocket, data_buffer, 1, 0);
	}
	player_data[*piddata].locked = false;

	delete[] data_buffer;
	delete piddata;
}

void Update_Player_Lat_Lon(SOCKET ClientSocket)
{
	//log not yet implimented function may not ever be used

	char *data_buffer = new char[sizeof(int)+ 2 + sizeof(double) * 2];
	int *piddata = new int;
	double *Lat = new double;
	double *Lon = new double;


	recv(ClientSocket, data_buffer, sizeof(int)+ 2 + sizeof(double)* 2, 0);
	memcpy(piddata, data_buffer, sizeof(int));
	memcpy(Lat, data_buffer + sizeof(int) + 2, sizeof(double));
	memcpy(Lon, data_buffer + sizeof(int)+ 2 + sizeof(double), sizeof(double));

	while (player_data[*piddata].locked){}
	player_data[*piddata].locked = true;

	cout << "Update Lat,Lon: " << player_data[*piddata].USERNAME;
	if (player_data[*piddata].pin[0] == data_buffer[sizeof(int)] && player_data[*piddata].pin[1] == data_buffer[sizeof(int)+1])
	{
		player_data[*piddata].Lat = *Lat;
		player_data[*piddata].Lon = *Lon;

		cout << player_data[*piddata].USERNAME << "Lat,Lon Update Success" << endl;
		player_data[*piddata].locked = false;
		*piddata = 0;
		memcpy(data_buffer, piddata, sizeof(int));
		send(ClientSocket, data_buffer, sizeof(int), 0);
	}
	else
	{
		cout << player_data[*piddata].USERNAME << "Lat,Lon Update Couldn't Auth Player" << endl;
		player_data[*piddata].locked = false;
		*piddata = -1;
		memcpy(data_buffer, piddata, sizeof(int));
		send(ClientSocket, data_buffer, sizeof(int), 0);
	}

	delete[] data_buffer;
	delete piddata;
}

void Get_User_By_Pid(SOCKET ClientSocket)
{
	char *logbuffer;

	char *data_buffer = new char[33];
	int *piddata = new int;
	
	recv(ClientSocket, data_buffer, sizeof(int) * 2 + 2, 0);
	memcpy(piddata, data_buffer + sizeof(int) + 2, sizeof(int));

	while (player_data[*piddata].locked){}
	player_data[*piddata].locked = true;

	send(ClientSocket, player_data[*piddata].USERNAME, 33, 0);

	logbuffer = new char[strlen("Request For Pid For User: ") + strlen(player_data[*piddata].USERNAME) + 1];
	logbuffer[0] = NULL;
	strcat(logbuffer, "Request For Pid For User: ");
	strcat(logbuffer, player_data[*piddata].USERNAME);
	Write_Log_Data(logbuffer);
	delete logbuffer;

	player_data[*piddata].locked = false;

	delete[] data_buffer;
	delete piddata;
}

void update_bt_mac(SOCKET ClientSocket)
{
	char *logbuffer;

	char *data_buffer = new char[sizeof(int) + 19];
	int *piddata = new int;


	recv(ClientSocket,data_buffer,sizeof(int) + 19,0);
	memcpy(piddata,data_buffer,sizeof(int));

	while (player_data[*piddata].locked){}
	player_data[*piddata].locked = true;

	if(player_data[*piddata].pin[0] == data_buffer[sizeof(int)] && player_data[*piddata].pin[1] == data_buffer[sizeof(int) + 1])
	{
		logbuffer = new char[strlen("Set BT Mac For: ") + strlen(player_data[*piddata].USERNAME) + strlen(" - Old Mac: ") + strlen(player_data[*piddata].BTMAC) + strlen(" - New Mac: ") + strlen(player_data[*piddata].BTMAC) + 1];
		logbuffer[0] = NULL;
		strcat(logbuffer, "Set BT Mac For: ");
		strcat(logbuffer, player_data[*piddata].USERNAME);
		strcat(logbuffer, " - Old Mac: ");
		strcat(logbuffer, player_data[*piddata].BTMAC);
		strcat(logbuffer, " - New Mac: ");

		for(int i = 0; i < 17; i++)
		{
		player_data[*piddata].BTMAC[i] = data_buffer[6 + i];
		}

		strcat(logbuffer, player_data[*piddata].BTMAC);
		Write_Log_Data(logbuffer);
		delete logbuffer;

		player_data[*piddata].locked = false;
		*piddata = 0;
		memcpy(data_buffer,piddata,sizeof(int));
		send(ClientSocket,data_buffer,sizeof(int),0);
	}
	else
	{
		logbuffer = new char[strlen("Auth Fail For Mac Change: ") + strlen(player_data[*piddata].USERNAME) + +1];
		logbuffer[0] = NULL;
		strcat(logbuffer, "Auth Fail For Mac Change: ");
		strcat(logbuffer, player_data[*piddata].USERNAME);
		Write_Log_Data(logbuffer);
		delete logbuffer;

			player_data[*piddata].locked = false;
			*piddata = -1;
			memcpy(data_buffer,piddata,sizeof(int));
			send(ClientSocket,data_buffer,sizeof(int),0);
	}
	delete[] data_buffer;
	delete piddata;
}
void player_status_reuqest(SOCKET ClientSocket)///this function is correct as a single send / recv function
{
	char *logbuffer;

	char *data_buffer = new char[sizeof(int) + 2];
	int *piddata = new int;


	recv(ClientSocket,data_buffer,sizeof(int) + 2,0);
	memcpy(piddata,data_buffer,sizeof(int));

	while (player_data[*piddata].locked){}
	player_data[*piddata].locked = true;

	logbuffer = new char[strlen("Request For Status Byte By: ") + strlen(player_data[*piddata].USERNAME) + strlen(" Auth Success/Fail: ") + sizeof(char)+1];
	logbuffer[0] = NULL;
	strcat(logbuffer, "Request For Status Byte By: ");
	strcat(logbuffer, player_data[*piddata].USERNAME);
	strcat(logbuffer, " Auth Success/Fail: ");

	if(player_data[*piddata].pin[0] == data_buffer[sizeof(int)] && player_data[*piddata].pin[1] == data_buffer[sizeof(int) + 1])
	{
		strcat(logbuffer, "S");
		Write_Log_Data(logbuffer);
		delete logbuffer;

		data_buffer[0] = player_data[*piddata].status | 0x20;
		send(ClientSocket,data_buffer,sizeof(char),0);
	}
	else
	{
		strcat(logbuffer, "F");
		Write_Log_Data(logbuffer);
		delete logbuffer;

		data_buffer[0] = 0x40; 
		send(ClientSocket,data_buffer,1,0);
	}
	player_data[*piddata].locked = false;

	delete[] data_buffer;
	delete piddata;
}

void player_list_request(SOCKET ClientSocket)///this function is not correct as a single send / recv function
{
	char *logbuffer;

	char *data_buffer = new char[32];
	int *piddata = new int;
	int *sessioncount = new int;
	data_buffer[0] = NULL;

	recv(ClientSocket,data_buffer, sizeof(int) + 2,0);
	memcpy(piddata,data_buffer, sizeof(int));

	while (player_data[*piddata].locked){}
	player_data[*piddata].locked = true;

	// 0active 1it 2add 3remove 4admin
	if(player_data[*piddata].pin[0] == data_buffer[sizeof(int)] && player_data[*piddata].pin[1] == data_buffer[sizeof(int) + 1])
	{
		logbuffer = new char[strlen("Requet For Player List From: ") + strlen(player_data[*piddata].USERNAME) + 1];
		logbuffer[0] = NULL;
		strcat(logbuffer, "Requet For Player List From: ");
		strcat(logbuffer, player_data[*piddata].USERNAME);
		Write_Log_Data(logbuffer);
		delete logbuffer;

		if(check_premission_for_operation(0, *piddata) == true && (check_premission_for_operation(1, *piddata) == true || check_premission_for_operation(4, *piddata) == true || check_premission_for_operation(3, *piddata) == true))
		{
			logbuffer = new char[strlen("Sending Player List To ") + strlen(player_data[*piddata].USERNAME) + 1];
			logbuffer[0] = NULL;
			strcat(logbuffer, "Sending Player List To ");
			strcat(logbuffer, player_data[*piddata].USERNAME);
			Write_Log_Data(logbuffer);
			delete logbuffer;

			player_data[*piddata].locked = false;
			*sessioncount = sessiondata.current_number_of_players;
			memcpy(data_buffer,sessioncount,sizeof(int));
			send(ClientSocket,data_buffer,sizeof(int),0);
			for(int i = 0; i < svr_settings.max_players; i++)
			{

				while (player_data[i].locked){}
				player_data[i].locked = true;

				if(player_data[i].PID != -1)
				{
				*sessioncount = player_data[i].PID;
				memcpy(data_buffer,sessioncount,sizeof(int));
				send(ClientSocket,data_buffer,sizeof(int),0);
				send(ClientSocket,player_data[i].USERNAME,32,0);
				data_buffer[0] = player_data[i].status;
				send(ClientSocket,data_buffer,1,0);
				send(ClientSocket,player_data[i].BTMAC,17,0);
				}

				player_data[i].locked = false;
			}

			logbuffer = new char[strlen("Sent Player List To ") + strlen(player_data[*piddata].USERNAME) + 1];
			logbuffer[0] = NULL;
			strcat(logbuffer, "Sent Player List To ");
			strcat(logbuffer, player_data[*piddata].USERNAME);
			Write_Log_Data(logbuffer);
			delete logbuffer;

			player_data[*piddata].locked = false;
		}
		else
		{
			logbuffer = new char[strlen(player_data[*piddata].USERNAME) + strlen(": Doesn't Have Premission To Get A Player List") + 1];
			logbuffer[0] = NULL;
			strcat(logbuffer, player_data[*piddata].USERNAME);
			strcat(logbuffer, ": Doesn't Have Premission To Get A Player List");
			Write_Log_Data(logbuffer);
			delete logbuffer;

			player_data[*piddata].locked = false;
			*piddata = -2;
			memcpy(data_buffer,piddata,sizeof(int));
			send(ClientSocket,data_buffer,sizeof(int),0);
		}
	}
	else
	{
		logbuffer = new char[strlen(player_data[*piddata].USERNAME) + strlen(": Couldn't Auth Player") + 1];
		logbuffer[0] = NULL;
		strcat(logbuffer, player_data[*piddata].USERNAME);
		strcat(logbuffer, ": Couldn't Auth Player");
		Write_Log_Data(logbuffer);
		delete logbuffer;

		player_data[*piddata].locked = false;
		*piddata = -1;
		memcpy(data_buffer,piddata,sizeof(int));
		send(ClientSocket,data_buffer,sizeof(int),0);
	}
	delete sessioncount;
	delete[] data_buffer;
	delete piddata;
}

void pid_request(SOCKET ClientSocket)///this function is correct as a single send / recv function
{
	char *logbuffer;
	char *numberbuffer;

	char *data_buffer = new char[32];
	int *piddata = new int;

	data_buffer[0] = NULL;

	recv(ClientSocket,data_buffer,32,0);

	numberbuffer = new char[MAX_PATH];
	numberbuffer[0] = NULL;

	*piddata = find_player_by_user(data_buffer);
	if (*piddata == -1){ strcat(numberbuffer, "Not Found"); }
	else{ itoa(*piddata, numberbuffer, 10);}

	logbuffer = new char[strlen("Request For Pid From User ") + strlen(data_buffer) + strlen(" Pid Is:") + strlen(numberbuffer) + 1];
	logbuffer[0] = NULL;
	strcat(logbuffer, "Request For Pid From User ");
	strcat(logbuffer, data_buffer);
	strcat(logbuffer, " Pid Is:");
	strcat(logbuffer, numberbuffer);
	Write_Log_Data(logbuffer);
	delete logbuffer;

	memcpy(data_buffer,piddata,sizeof(int));

	send(ClientSocket,data_buffer,sizeof(int),0);

	delete[] data_buffer;
	delete piddata;
}

void tag_request(SOCKET ClientSocket)///this function is not correct needs only 1 send and recv
{
	char *logbuffer;

	char *data_buffer = new char[sizeof(int)+6];
	int *piddata = new int;
	int *targetpid = new int;

	recv(ClientSocket, data_buffer, sizeof(int) + 6,0);
	memcpy(piddata,data_buffer,sizeof(int));
	memcpy(targetpid, data_buffer + 6,sizeof(int));

	while (player_data[*piddata].locked){}
	player_data[*piddata].locked = true;

	if(player_data[*piddata].pin[0] == data_buffer[sizeof(int)] && player_data[*piddata].pin[1] == data_buffer[sizeof(int) + 1])
	{
		logbuffer = new char[strlen("Got Tag Request From : ") + strlen(player_data[*piddata].USERNAME) + 1];
		logbuffer[0] = NULL;
		strcat(logbuffer, "Got Tag Request From: ");
		strcat(logbuffer, player_data[*piddata].USERNAME);
		Write_Log_Data(logbuffer);
		delete logbuffer;

		if(check_premission_for_operation(0,*piddata) == true && check_premission_for_operation(1,*piddata) == true)
		{
			if (player_data[*piddata].Tagged_By_PID == *targetpid)
			{
				while (player_data[*targetpid].locked){}
				player_data[*targetpid].locked = true;

				if (player_data[*targetpid].tagback_count <= 0)
				{
					player_data[*piddata].locked = false;
					player_data[*targetpid].locked = false;

					*piddata = 1;
					memcpy(data_buffer, piddata, sizeof(int));
					send(ClientSocket, data_buffer, sizeof(int), 0);
					delete targetpid;
					delete[] data_buffer;
					delete piddata;
					return;
				}

				player_data[*targetpid].locked = false;
			}

			player_data[*piddata].tagque.target = *targetpid;//need to fix this so multipul its can exist

			player_data[*piddata].locked = false;

			while (player_data[*piddata].tagque.currentit != *piddata)
			{
				//need to create a time out to kill thread if no confirm ever recieved
			}//need to fix this so multipul its can exist

			while (player_data[*piddata].locked){}
			player_data[*piddata].locked = true;

			while (player_data[*targetpid].locked){}
			player_data[*targetpid].locked = true;

			player_data[*piddata].status = player_data[*piddata].status & 0xFD;
			player_data[*targetpid].status = player_data[*targetpid].status | 0x02;

			logbuffer = new char[strlen(player_data[*piddata].USERNAME) + strlen(": Is No Longer It") + 1];
			logbuffer[0] = NULL;
			strcat(logbuffer, player_data[*piddata].USERNAME);
			strcat(logbuffer, ": Is No Longer It");
			Write_Log_Data(logbuffer);
			delete logbuffer;

			player_data[*piddata].locked = false;
			player_data[*targetpid].locked = false;

			*piddata = 0;
			memcpy(data_buffer,piddata,sizeof(int));
			send(ClientSocket,data_buffer,sizeof(int),0);

		}
		else
		{
			logbuffer = new char[strlen(player_data[*piddata].USERNAME) + strlen(": Is Not It Or Is Not Active. Tag Request Failed") + 1];
			logbuffer[0] = NULL;
			strcat(logbuffer, player_data[*piddata].USERNAME);
			strcat(logbuffer, ": Is Not It Or Is Not Active. Tag Request Failed");
			Write_Log_Data(logbuffer);
			delete logbuffer;

			player_data[*piddata].locked = false;
			*piddata = -2;
			memcpy(data_buffer,piddata,sizeof(int));
			send(ClientSocket,data_buffer,sizeof(int),0);
		}
	}
	else
	{
		logbuffer = new char[strlen(player_data[*piddata].USERNAME) + strlen(": Couldn't Auth Player") + 1];
		logbuffer[0] = NULL;
		strcat(logbuffer, player_data[*piddata].USERNAME);
		strcat(logbuffer, ": Couldn't Auth Player");
		Write_Log_Data(logbuffer);
		delete logbuffer;

		player_data[*piddata].locked = false;
		*piddata = -1;
		memcpy(data_buffer,piddata,sizeof(int));
		send(ClientSocket,data_buffer,sizeof(int),0);
	}

	delete targetpid;
	delete[] data_buffer;
	delete piddata;
}

void tag_confirm_request(SOCKET ClientSocket)///this function is not correct needs only 1 send and recv
{
	char *logbuffer;

	char *data_buffer = new char[sizeof(int) + 6];
	int *piddata = new int;
	int *currentit = new int;

	recv(ClientSocket,data_buffer,sizeof(int) + 6,0);
	memcpy(piddata,data_buffer,sizeof(int));
	memcpy(currentit, data_buffer + 6,sizeof(int));

	while (player_data[*piddata].locked){}
	player_data[*piddata].locked = true;

	if(player_data[*piddata].pin[0] == data_buffer[sizeof(int)] && player_data[*piddata].pin[1] == data_buffer[sizeof(int) + 1])
	{
		if(check_premission_for_operation(0,*piddata) == true && check_premission_for_operation(1,*piddata) == false)
		{
				while (player_data[*currentit].locked){}
				player_data[*currentit].locked = true;

				if (player_data[*currentit].Tagged_By_PID == *piddata)
				{
					if (player_data[*piddata].tagback_count <= 0)
					{
						player_data[*currentit].locked = false;
						player_data[*piddata].locked = false;
						*piddata = 1;
						memcpy(data_buffer, piddata, sizeof(int));
						send(ClientSocket, data_buffer, sizeof(int), 0);
						delete[] data_buffer;
						delete piddata;
						delete currentit;
						return;
					}
					else
					{
						player_data[*piddata].tagback_count--;
					}
				}


				player_data[*currentit].tagque.currentit = *currentit;

				player_data[*currentit].locked = false;

				logbuffer = new char[strlen("Got A Tag Confirm From: ") + strlen(player_data[*piddata].USERNAME) + 1];
				logbuffer[0] = NULL;
				strcat(logbuffer, "Got A Tag Confirm From: ");
				strcat(logbuffer, player_data[*piddata].USERNAME);
				Write_Log_Data(logbuffer);
				delete logbuffer;

				player_data[*piddata].Tagged_By_PID = *currentit;

				player_data[*piddata].locked = false;
				*piddata = 0;
				memcpy(data_buffer,piddata,sizeof(int));
				send(ClientSocket,data_buffer,sizeof(int),0);
		}
		else
		{
			logbuffer = new char[strlen(player_data[*piddata].USERNAME) + strlen(": Is It Or Is Not Active. Tag Confirm Request Failed") + 1];
			logbuffer[0] = NULL;
			strcat(logbuffer, player_data[*piddata].USERNAME);
			strcat(logbuffer, ": Is It Or Is Not Active. Tag Confirm Request Failed");
			Write_Log_Data(logbuffer);
			delete logbuffer;

			player_data[*piddata].locked = false;
			*piddata = -2;
			memcpy(data_buffer,piddata,sizeof(int));
			send(ClientSocket,data_buffer,sizeof(int),0);
		}
	}
	else
	{
		logbuffer = new char[strlen(player_data[*piddata].USERNAME) + strlen(": Couldn't Auth Player") + 1];
		logbuffer[0] = NULL;
		strcat(logbuffer, player_data[*piddata].USERNAME);
		strcat(logbuffer, ": Couldn't Auth Player");
		Write_Log_Data(logbuffer);
		delete logbuffer;

		player_data[*piddata].locked = false;
		*piddata = -1;
		memcpy(data_buffer,piddata,sizeof(int));
		send(ClientSocket,data_buffer,sizeof(int),0);
	}

	delete[] data_buffer;
	delete piddata;
	delete currentit;
}

void add_player(SOCKET ClientSocket)///this function is correct as a single send / recv function
{
	char *logbuffer;

	char *data_buffer = new char[sizeof(int) + (sizeof(char) * 34)];
	int *piddata = new int;

	recv(ClientSocket,data_buffer,sizeof(int) + (sizeof(char) * 34),0);
	memcpy(piddata,data_buffer,sizeof(int));
	data_buffer[sizeof(int) + (sizeof(char) * 33)] = NULL;

	while (player_data[*piddata].locked){}
	player_data[*piddata].locked = true;

	if(player_data[*piddata].pin[0] == data_buffer[sizeof(int)] && player_data[*piddata].pin[1] == data_buffer[sizeof(int) + 1])
	{
		if(check_premission_for_operation(2,*piddata) == true || check_premission_for_operation(4,*piddata) == true)
		{
			player_data[*piddata].locked = false;

			if(sessiondata.current_number_of_players < svr_settings.max_players)
			{
				if(find_player_by_user(data_buffer + sizeof(int) + (sizeof(char) * 2)) == -1)
				{
					for(int i = 0; i < svr_settings.max_players; i++)
					{
						while (player_data[i].locked){}
						player_data[i].locked = true;

						if(player_data[i].PID == -1)
						{
							sessiondata.current_number_of_players++;

							for(int iA = 0; iA < 32; iA++)
							{
								player_data[i].USERNAME[iA] = data_buffer[sizeof(int) + (sizeof(char) * 2) + iA];
								if(data_buffer[sizeof(int) + (sizeof(char) * 2) + iA] == NULL){break;}
							}
							player_data[i].tagback_count = 0;
							player_data[i].status = 0X01;
							player_data[i].pin[0] = 0x12;
							player_data[i].pin[1] = 0x34;
							player_data[i].PID = i;
							//player_data[i].last_check_in = current time and date
							player_data[i].BTMAC[0] = 'F';
							player_data[i].BTMAC[1] = 'F';
							player_data[i].BTMAC[2] = ':';
							player_data[i].BTMAC[3] = 'F';
							player_data[i].BTMAC[4] = 'F';
							player_data[i].BTMAC[5] = ':';
							player_data[i].BTMAC[6] = 'F';
							player_data[i].BTMAC[7] = 'F';
							player_data[i].BTMAC[8] = ':';
							player_data[i].BTMAC[9] = 'F';
							player_data[i].BTMAC[10] = 'F';
							player_data[i].BTMAC[11] = ':';
							player_data[i].BTMAC[12] = 'F';
							player_data[i].BTMAC[13] = 'F';
							player_data[i].BTMAC[14] = ':';
							player_data[i].BTMAC[15] = 'F';
							player_data[i].BTMAC[16] = 'F';

							logbuffer = new char[strlen(player_data[*piddata].USERNAME) + strlen(" Added New Player:") + strlen(data_buffer + sizeof(int)+(sizeof(char)* 2)) + 1];
							logbuffer[0] = NULL;
							strcat(logbuffer, player_data[*piddata].USERNAME);
							strcat(logbuffer, " Added New Player:");
							strcat(logbuffer, data_buffer + sizeof(int)+(sizeof(char)* 2));
							Write_Log_Data(logbuffer);
							delete logbuffer;

							player_data[i].locked = false;

							*piddata = 0;
							memcpy(data_buffer,piddata,sizeof(int));
							send(ClientSocket,data_buffer,sizeof(int),0);
							break;
						}

						player_data[i].locked = false;
					}
				}
				else
				{
					while (player_data[*piddata].locked){}
					player_data[*piddata].locked = true;

					logbuffer = new char[strlen(player_data[*piddata].USERNAME) + strlen(": Player: ") + strlen(data_buffer + sizeof(int)+(sizeof(char)* 2)) + strlen(" Wasn't Added As The User Is Currently In Use") + 1];
					logbuffer[0] = NULL;
					strcat(logbuffer, player_data[*piddata].USERNAME);
					strcat(logbuffer, ": Player: ");
					strcat(logbuffer, data_buffer + sizeof(int)+(sizeof(char)* 2));
					strcat(logbuffer, " Wasn't Added As The User Is Currently In Use");
					Write_Log_Data(logbuffer);
					delete logbuffer;
					
					player_data[*piddata].locked = false;
					
					*piddata = -4;
					memcpy(data_buffer,piddata,sizeof(int));
					send(ClientSocket,data_buffer,sizeof(int),0);
				}

			}
			else
			{
				while (player_data[*piddata].locked){}
				player_data[*piddata].locked = true;

				logbuffer = new char[strlen(player_data[*piddata].USERNAME) + strlen(": At Max Users Could Not Add Player") + 1];
				logbuffer[0] = NULL;
				strcat(logbuffer, player_data[*piddata].USERNAME);
				strcat(logbuffer, ": At Max Users Could Not Add Player");
				Write_Log_Data(logbuffer);
				delete logbuffer;

				player_data[*piddata].locked = false;

				*piddata = -3;
				memcpy(data_buffer,piddata,sizeof(int));
				send(ClientSocket,data_buffer,sizeof(int),0);
			}
		}
		else
		{			
			logbuffer = new char[strlen(player_data[*piddata].USERNAME) + strlen(": Doesn't Have Premission To Add Players") + 1];
			logbuffer[0] = NULL;
			strcat(logbuffer, player_data[*piddata].USERNAME);
			strcat(logbuffer, ": Doesn't Have Premission To Add Players");
			Write_Log_Data(logbuffer);
			delete logbuffer;

			player_data[*piddata].locked = false;

			*piddata = -2;
			memcpy(data_buffer,piddata,sizeof(int));
			send(ClientSocket,data_buffer,sizeof(int),0);
		}
	}
	else
	{
		logbuffer = new char[strlen(player_data[*piddata].USERNAME) + strlen(": Couldn't Auth Player") + 1];
		logbuffer[0] = NULL;
		strcat(logbuffer, player_data[*piddata].USERNAME);
		strcat(logbuffer, ": Couldn't Auth Player");
		Write_Log_Data(logbuffer);
		delete logbuffer;

		player_data[*piddata].locked = false;

		*piddata = -1;
		memcpy(data_buffer,piddata,sizeof(int));
		send(ClientSocket,data_buffer,sizeof(int),0);
	}
}

void remove_player(SOCKET ClientSocket)///this function is correct as a single send / recv function
{
	char *logbuffer;

	char *data_buffer = new char[(sizeof(int) * 2) + (sizeof(char) * 2)];
	int *pid = new int;
	int *piddata = new int;
	
	recv(ClientSocket,data_buffer,(sizeof(int) * 2) + (sizeof(char) * 2),0);
	memcpy(piddata,data_buffer,sizeof(int));

	while (player_data[*piddata].locked){}
	player_data[*piddata].locked = true;

	if(player_data[*piddata].pin[0] == data_buffer[sizeof(int)] && player_data[*piddata].pin[1] == data_buffer[sizeof(int) + 1])
	{
		if(check_premission_for_operation(3, *piddata) == true)
		{
			player_data[*piddata].locked = false;

			memcpy(pid,data_buffer + sizeof(int) + (sizeof(char) * 2),sizeof(int));

			while (player_data[*pid].locked){}
			player_data[*pid].locked = true;

			if(*pid < svr_settings.max_players && pid >= 0)
			{
				logbuffer = new char[strlen("Removeing ") + strlen(player_data[*pid].USERNAME) + strlen(" From The Game") + 1];
				logbuffer[0] = NULL;
				strcat(logbuffer, "Removeing ");
				strcat(logbuffer, player_data[*pid].USERNAME);
				strcat(logbuffer, " From The Game");
				Write_Log_Data(logbuffer);
				delete logbuffer;

			player_data[*pid].PID = -1;
			player_data[*pid].USERNAME[0] = NULL;
			player_data[*pid].tagback_count = 0;
			player_data[*pid].status = NULL;
			player_data[*pid].pin[0] = player_data[*pid].pin[1] = NULL;

			logbuffer = new char[strlen(player_data[*piddata].USERNAME) + strlen(": Removed Pid ") + strlen(" From The Game") + 1];
			logbuffer[0] = NULL;
			strcat(logbuffer, player_data[*piddata].USERNAME);
			strcat(logbuffer, ": Removed Pid ");
			strcat(logbuffer, " From The Game");
			Write_Log_Data(logbuffer);
			delete logbuffer;

			player_data[*pid].locked = false;

			*pid = 0;
			memcpy(data_buffer,pid,sizeof(int));
			sessiondata.current_number_of_players--;
			send(ClientSocket,data_buffer,sizeof(int),0);
			}
			else
			{
				logbuffer = new char[strlen(player_data[*piddata].USERNAME) + strlen(": Couldn't Delete Player Pid Is Out Of Range") + 1];
				logbuffer[0] = NULL;
				strcat(logbuffer, player_data[*piddata].USERNAME);
				strcat(logbuffer, ": Couldn't Delete Player Pid Is Out Of Range");
				Write_Log_Data(logbuffer);
				delete logbuffer;

				player_data[*pid].locked = false;

				*pid = -3;
				memcpy(data_buffer,pid,sizeof(int));
				send(ClientSocket,data_buffer,sizeof(int),0);
			}
		}
		else
		{
			logbuffer = new char[strlen(player_data[*piddata].USERNAME) + strlen(": Doesn't Have Premission To Remove Players") + 1];
			logbuffer[0] = NULL;
			strcat(logbuffer, player_data[*piddata].USERNAME);
			strcat(logbuffer, ": Doesn't Have Premission To Remove Players");
			Write_Log_Data(logbuffer);
			delete logbuffer;

			player_data[*piddata].locked = false;
			*pid = -2;
			memcpy(data_buffer,pid,sizeof(int));
			send(ClientSocket,data_buffer,sizeof(int),0);
		}
	}
	else
	{
		logbuffer = new char[strlen(player_data[*piddata].USERNAME) + strlen(": Couldn't Auth Player") + 1];
		logbuffer[0] = NULL;
		strcat(logbuffer, player_data[*piddata].USERNAME);
		strcat(logbuffer, ": Couldn't Auth Player");
		Write_Log_Data(logbuffer);
		delete logbuffer;

		player_data[*piddata].locked = false;
		*pid = -1;
		memcpy(data_buffer,pid,sizeof(int));
		send(ClientSocket,data_buffer,sizeof(int),0);
	}
}

void set_player_pin(SOCKET ClientSocket)///this function is correct as a single send / recv function
{
	char *logbuffer;

	char *data_buffer = new char[(sizeof(int) * 2) + (sizeof(char) * 4)];
	int *pid = new int;
	int *targetpid = new int;

	recv(ClientSocket,data_buffer,(sizeof(int) * 2) + (sizeof(char) * 4),0);
	memcpy(pid,data_buffer,sizeof(int));
	memcpy(targetpid,data_buffer + sizeof(int) + (sizeof(char) * 2),sizeof(int));

	while (player_data[*pid].locked){}
	player_data[*pid].locked = true;

	if(player_data[*pid].pin[0] == data_buffer[sizeof(int)] && player_data[*pid].pin[1] == data_buffer[sizeof(int) + 1] && *pid == *targetpid || player_data[*pid].pin[0] == data_buffer[sizeof(int)] && player_data[*pid].pin[1] == data_buffer[sizeof(int) + 1 ] && check_premission_for_operation(4,*pid))
	{
		player_data[*pid].locked = false;

		while (player_data[*targetpid].locked){}
		player_data[*targetpid].locked = true;

		player_data[*targetpid].pin[0] = data_buffer[(sizeof(int) * 2) + (sizeof(char) * 2)];
		player_data[*targetpid].pin[1] = data_buffer[(sizeof(int) * 2) + (sizeof(char) * 3)];

		logbuffer = new char[strlen("Set ") + strlen(player_data[*targetpid].USERNAME) + strlen("'s Pin") + 1];
		logbuffer[0] = NULL;
		strcat(logbuffer, "Set ");
		strcat(logbuffer, player_data[*targetpid].USERNAME);
		strcat(logbuffer, "'s Pin");
		Write_Log_Data(logbuffer);
		delete logbuffer;

		player_data[*targetpid].locked = false;

		*pid = 0;
		memcpy(data_buffer,pid,sizeof(int));
		send(ClientSocket,data_buffer,sizeof(int),0);
	}
	else
	{
		logbuffer = new char[strlen(player_data[*pid].USERNAME) + strlen(": Couldn't Auth Player or Premissions Not Avaliable For Pin Change") + 1];
		logbuffer[0] = NULL;
		strcat(logbuffer, player_data[*pid].USERNAME);
		strcat(logbuffer, ": Couldn't Auth Player or Premissions Not Avaliable For Pin Change");
		Write_Log_Data(logbuffer);
		delete logbuffer;

		player_data[*pid].locked = false;

		*pid = -1;
		memcpy(data_buffer,pid,sizeof(int));
		send(ClientSocket,data_buffer,sizeof(int),0);
	}

	delete[] data_buffer;
	delete pid;
	delete targetpid;
}

void set_player_status_and_premissions(SOCKET ClientSocket)///this function is correct as a single send / recv function
{
	char *logbuffer;

	char *data_buffer = new char[(sizeof(int) * 2) + (sizeof(char) * 3)];
	int *piddata = new int;
	int *intstore = new int;

	recv(ClientSocket,data_buffer,(sizeof(int) * 2) + (sizeof(char) * 3),0);
	memcpy(piddata,data_buffer,sizeof(int));

	if(player_data[*piddata].pin[0] == data_buffer[sizeof(int)] && player_data[*piddata].pin[1] == data_buffer[sizeof(int) + 1])
	{
		if(check_premission_for_operation(0, *piddata) == true && check_premission_for_operation(4, *piddata) == true)
		{
			memcpy(intstore,data_buffer + sizeof(int) + (sizeof(char) * 2),sizeof(int));
			bool current = check_premission_for_operation(data_buffer[(sizeof(int) * 2) + (sizeof(char) * 2)], *intstore);// 0active 1it 2add 3remove 4admin
			switch(data_buffer[(sizeof(int) * 2) + (sizeof(char) * 2)])
			{
			case 0x00://toggle active status
				if(current == false)
				{
					player_data[*intstore].status = player_data[*intstore].status | 0x01;

					logbuffer = new char[strlen(player_data[*piddata].USERNAME) + strlen(": Set ") + strlen(player_data[*intstore].USERNAME) + strlen("'s Active Status to Enabled") + 1];
					logbuffer[0] = NULL;
					strcat(logbuffer, player_data[*piddata].USERNAME);
					strcat(logbuffer, ": Set ");
					strcat(logbuffer, player_data[*intstore].USERNAME);
					strcat(logbuffer, "'s Active Status to Enabled");
					Write_Log_Data(logbuffer);
					delete logbuffer;

					*piddata = 1;
					memcpy(data_buffer,piddata,sizeof(int));
				}
				else
				{
					player_data[*intstore].status = player_data[*intstore].status & 0xFE;

					logbuffer = new char[strlen(player_data[*piddata].USERNAME) + strlen(": Set ") + strlen(player_data[*intstore].USERNAME) + strlen("'s Active Status to Disabled") + 1];
					logbuffer[0] = NULL;
					strcat(logbuffer, player_data[*piddata].USERNAME);
					strcat(logbuffer, ": Set ");
					strcat(logbuffer, player_data[*intstore].USERNAME);
					strcat(logbuffer, "'s Active Status to Disabled");
					Write_Log_Data(logbuffer);
					delete logbuffer;

					*piddata = 0;
					memcpy(data_buffer,piddata,sizeof(int));
				}
				break;
			case 0x01://toggle it status
				if(current == false)
				{
					player_data[*intstore].status = player_data[*intstore].status | 0x02;

					logbuffer = new char[strlen(player_data[*piddata].USERNAME) + strlen(": Set ") + strlen(player_data[*intstore].USERNAME) + strlen("'s It Status to Enabled") + 1];
					logbuffer[0] = NULL;
					strcat(logbuffer, player_data[*piddata].USERNAME);
					strcat(logbuffer, ": Set ");
					strcat(logbuffer, player_data[*intstore].USERNAME);
					strcat(logbuffer, "'s It Status to Enabled");
					Write_Log_Data(logbuffer);
					delete logbuffer;

					*piddata = 1;
					memcpy(data_buffer,piddata,sizeof(int));
				}
				else
				{
					player_data[*intstore].status = player_data[*intstore].status & 0xFD;

					logbuffer = new char[strlen(player_data[*piddata].USERNAME) + strlen(": Set ") + strlen(player_data[*intstore].USERNAME) + strlen("'s It Status to Disabled") + 1];
					logbuffer[0] = NULL;
					strcat(logbuffer, player_data[*piddata].USERNAME);
					strcat(logbuffer, ": Set ");
					strcat(logbuffer, player_data[*intstore].USERNAME);
					strcat(logbuffer, "'s It Status to Disabled");
					Write_Log_Data(logbuffer);
					delete logbuffer;

					*piddata = 0;
					memcpy(data_buffer,piddata,sizeof(int));
				}
				break;
			case 0x02://toggel add player
				if(current == false)
				{
					player_data[*intstore].status = player_data[*intstore].status | 0x04;

					logbuffer = new char[strlen(player_data[*piddata].USERNAME) + strlen(": Set ") + strlen(player_data[*intstore].USERNAME) + strlen("'s Add Player Status to Enabled") + 1];
					logbuffer[0] = NULL;
					strcat(logbuffer, player_data[*piddata].USERNAME);
					strcat(logbuffer, ": Set ");
					strcat(logbuffer, player_data[*intstore].USERNAME);
					strcat(logbuffer, "'s Add Player Status to Enabled");
					Write_Log_Data(logbuffer);
					delete logbuffer;

					*piddata = 1;
					memcpy(data_buffer,piddata,sizeof(int));
				}
				else
				{
					player_data[*intstore].status = player_data[*intstore].status & 0xFB;

					logbuffer = new char[strlen(player_data[*piddata].USERNAME) + strlen(": Set ") + strlen(player_data[*intstore].USERNAME) + strlen("'s Add Player Status to Disabled") + 1];
					logbuffer[0] = NULL;
					strcat(logbuffer, player_data[*piddata].USERNAME);
					strcat(logbuffer, ": Set ");
					strcat(logbuffer, player_data[*intstore].USERNAME);
					strcat(logbuffer, "'s Add Player Status to Disabled");
					Write_Log_Data(logbuffer);
					delete logbuffer;

					*piddata = 0;
					memcpy(data_buffer,piddata,sizeof(int));
				}
				break;
			case 0x03://toggle remove player
				if(current == false)
				{
					player_data[*intstore].status = player_data[*intstore].status | 0x08;

					logbuffer = new char[strlen(player_data[*piddata].USERNAME) + strlen(": Set ") + strlen(player_data[*intstore].USERNAME) + strlen("'s Remove Player Status to Enabled") + 1];
					logbuffer[0] = NULL;
					strcat(logbuffer, player_data[*piddata].USERNAME);
					strcat(logbuffer, ": Set ");
					strcat(logbuffer, player_data[*intstore].USERNAME);
					strcat(logbuffer, "'s Remove Player Status to Enabled");
					Write_Log_Data(logbuffer);
					delete logbuffer;

					*piddata = 1;
					memcpy(data_buffer,piddata,sizeof(int));
				}
				else
				{
					player_data[*intstore].status = player_data[*intstore].status & 0xF7;

					logbuffer = new char[strlen(player_data[*piddata].USERNAME) + strlen(": Set ") + strlen(player_data[*intstore].USERNAME) + strlen("'s Remove Player Status to Disabled") + 1];
					logbuffer[0] = NULL;
					strcat(logbuffer, player_data[*piddata].USERNAME);
					strcat(logbuffer, ": Set ");
					strcat(logbuffer, player_data[*intstore].USERNAME);
					strcat(logbuffer, "'s Remove Player Status to Disabled");
					Write_Log_Data(logbuffer);
					delete logbuffer;

					*piddata = 0;
					memcpy(data_buffer,piddata,sizeof(int));
				}
				break;
			case 0x04://toggle admin
				if(current == false)
				{
					player_data[*intstore].status = player_data[*intstore].status | 0x10;

					logbuffer = new char[strlen(player_data[*piddata].USERNAME) + strlen(": Set ") + strlen(player_data[*intstore].USERNAME) + strlen("'s Admin Status to Enabled") + 1];
					logbuffer[0] = NULL;
					strcat(logbuffer, player_data[*piddata].USERNAME);
					strcat(logbuffer, ": Set ");
					strcat(logbuffer, player_data[*intstore].USERNAME);
					strcat(logbuffer, "'s Admin Status to Enabled");
					Write_Log_Data(logbuffer);
					delete logbuffer;

					*piddata = 1;
					memcpy(data_buffer,piddata,sizeof(int));
				}
				else
				{
					player_data[*intstore].status = player_data[*intstore].status & 0xEF;

					logbuffer = new char[strlen(player_data[*piddata].USERNAME) + strlen(": Set ") + strlen(player_data[*intstore].USERNAME) + strlen("'s Admin Status to Disabled") + 1];
					logbuffer[0] = NULL;
					strcat(logbuffer, player_data[*piddata].USERNAME);
					strcat(logbuffer, ": Set ");
					strcat(logbuffer, player_data[*intstore].USERNAME);
					strcat(logbuffer, "'s Admin Status to Disabled");
					Write_Log_Data(logbuffer);
					delete logbuffer;

					*piddata = 0;
					memcpy(data_buffer,piddata,sizeof(int));
				}
				break;
			default:
				Write_Log_Data("Invalid Command Issued To Set Satus");
				break;
			}
			send(ClientSocket,data_buffer,sizeof(int),0);
		}
		else
		{
			logbuffer = new char[strlen(player_data[*piddata].USERNAME) + strlen(": Doesn't Have Admin Premissions") + 1];
			logbuffer[0] = NULL;
			strcat(logbuffer, player_data[*piddata].USERNAME);
			strcat(logbuffer, ": Doesn't Have Admin Premissions");
			Write_Log_Data(logbuffer);
			delete logbuffer;

			*piddata = -2;
			memcpy(data_buffer,piddata,sizeof(int));
			send(ClientSocket,data_buffer,sizeof(int),0);
		}
	}
	else
	{
		logbuffer = new char[strlen(player_data[*piddata].USERNAME) + strlen(": Couldn't Auth Player") + 1];
		logbuffer[0] = NULL;
		strcat(logbuffer, player_data[*piddata].USERNAME);
		strcat(logbuffer, ": Couldn't Auth Player");
		Write_Log_Data(logbuffer);
		delete logbuffer;

		*piddata = -1;
		memcpy(data_buffer,piddata,sizeof(int));
		send(ClientSocket,data_buffer,sizeof(int),0);
	}
}
///End of Network Admin Functions
///End Of NetworkFunctions


///****Support Functions
int strcmpA(char dataa[], char *datab)
{
	if(strlen(dataa) == strlen(datab))
	{
		for(int i = 0; i <= strlen(dataa); i++)
		{
			if(dataa[i] != datab[i]){return 1;}
		}
		return 0;
	}
	else
	{
		return 1;
	}
}

int find_player_by_user(char *username)
{
	int user_pid = -1;
	bool diff = false;
	for(int i = 0; i < svr_settings.max_players; i++)
	{
		if(strlen(username) == strlen(player_data[i].USERNAME))
		{
			user_pid = i;
			diff = false;
			for(int i1 = 0; i1 <= strlen(player_data[i].USERNAME); i1++)
			{
				if(username[i1] != player_data[i].USERNAME[i1]){diff = true;}
				if(diff == true)
				{
					break;
				}
			}
			if(diff == false)
			{
				return user_pid;
			}
		}
	}
	return -1;
}

bool check_premission_for_operation(int check, int pid)
{
	char temp;
	char premissions = player_data[pid].status;

	//takes a bool stating the flags need to check as true
	//checks those valuse against the players premissions
	//returns true if player has the premissions
	//returns false if any fails
	switch(check)
	{
	case 0:
		temp = 0x01;
		premissions = premissions & temp;
		if(premissions != temp)
		{
			return false;
		}
		break;
	case 1:
		premissions = player_data[pid].status;
		temp = 0x02;
		premissions = premissions & temp;
		if(premissions != temp)
		{
			return false;
		}
		break;
	case 2:
		premissions = player_data[pid].status;
		temp = 0x04;
		premissions = premissions & temp;
		if(premissions != temp)
		{
			return false;
		}
		break;
	case 3:
		premissions = player_data[pid].status;
		temp = 0x08;
		premissions = premissions & temp;
		if(premissions != temp)
		{
			return false;
		}
		break;
	case 4:
		premissions = player_data[pid].status;
		temp = 0x10;
		premissions = premissions & temp;
		if(premissions != temp)
		{
			return false;
		}
		break;
	}
	return true;
}

void Get_New_Settings_Data()
{
	char cmpbuffer[MAX_PATH];

	Write_Log_Data("No Settings.ini was found time to make one lol");
	cout << "AutoScan Allowed[Y/N]:";
	cin >> cmpbuffer;
	if (cmpbuffer[0] == 'y' || cmpbuffer[0] == 'Y'){ svr_settings.AutoScanAllowed = true; }
	else{ svr_settings.AutoScanAllowed = false; }

	cout << "Scan Detect Allowed[Y/N]:";
	cin >> cmpbuffer;
	if (cmpbuffer[0] == 'y' || cmpbuffer[0] == 'Y'){ svr_settings.ScanDetectAllowed = true; }
	else{ svr_settings.ScanDetectAllowed = false; }

	cout << "TagBack Control[Y/N]:";
	cin >> cmpbuffer;
	if (cmpbuffer[0] == 'y' || cmpbuffer[0] == 'Y'){ svr_settings.TagBackControl = true; }
	else{ svr_settings.TagBackControl = false; }

	cout << "Use GPS For Scan List[Y/N]:";
	cin >> cmpbuffer;
	if (cmpbuffer[0] == 'y' || cmpbuffer[0] == 'Y'){ svr_settings.UseGPSforplayerlist = true; }
	else{ svr_settings.UseGPSforplayerlist = false; }

	cout << "Server Port:";
	cin >> cmpbuffer;
	svr_settings.port = new char[strlen(cmpbuffer) + 1];
	svr_settings.port[0] = NULL;
	strcat(svr_settings.port, cmpbuffer);

	cout << "Max Possible Players:";
	cin >> cmpbuffer;
	svr_settings.max_players = atoi(cmpbuffer);

	cout << "Max Possible IT Players:";
	cin >> cmpbuffer;
	svr_settings.max_it_players = atoi(cmpbuffer);

	cout << "Max Connections:";
	cin >> cmpbuffer;
	svr_settings.max_connections = atoi(cmpbuffer);

	threads = new threads_struct[svr_settings.max_connections];
	for (int i = 0; i < svr_settings.max_connections; i++)
	{
		threads[i].isfree = true;
		threads[i].threadid = i;
	}

	Save_Settings_Data();

	Write_Log_Data("Settings.ini Created Moveing On :-D");
}

bool get_player_data_lock(int pid)
{
	if (pid < 0 || pid > svr_settings.max_it_players){ return false; }
	while (player_data[pid].locked == true){}
	player_data[pid].locked = true;
	return true;
}

bool release_player_data__lock(int pid)
{
	if (pid < 0 || pid > svr_settings.max_it_players){ return false; }
	if (player_data[pid].locked == false){ return false; }
	player_data[pid].locked = false;
}
