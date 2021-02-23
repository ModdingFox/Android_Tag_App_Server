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

struct player_data_struct///current total 47bytes perplayer
{
	///example for database pid+username+statusflags+tagbackcount+failedcheckincount
	bool locked;
	int PID;
	char USERNAME[33];///max of  32 bytes could optimise with pointers but this will do for now
	char status;//used as a byte for flags contains it,active,addplayers,removeplayers,admin status flags and 3 waste bits
	char pin[2];
	char BTMAC[17];//the mac address of the device
	int Tagged_By_PID;
	int tagback_count;
	double Lat;
	double Lon;
	struct tagque_struct tagque;
}*player_data;

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
