#include <stdio.h>
#include <sqlite3.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <dirent.h>

// home directory/tracker_databases
#define DB_DIRECTORY "tracker_databases"

//Purpose of program: track daily advancements in user-specified directions of life. Score based system.

//functions
void print_help() {
	printf("To use this utility you need to create database first\n");
	printf("To create new database use --newdb with name you want as your database name\n");
	printf("To list all databases use --databases\n");
	printf("To open and do operations on your database use --open with your database name\n");
}

//Function to list the contents of a directory
void list_directory_contents(const char *path) {
    struct dirent *entry;
    DIR *dp = opendir(path);

    if (dp == NULL) {
        perror("opendir");
        return;
    }

    printf("Contents of directory '%s':\n", path);
    while ((entry = readdir(dp)) != NULL) {
        // Skip the current (.) and parent (..) directories
        if (entry->d_name[0] != '.') {
            printf(" - %s\n", entry->d_name);
        }
    }

    closedir(dp);
}

int check_db_folder(const char *home_dir, const char *db_dir_path) {
	// Get the home directory
    	if (home_dir == NULL) {
        	fprintf(stderr, "Could not determine home directory.\n");
        	return -1; // Error
    	}
    	// Check if the directory exists
    	if (access(db_dir_path, F_OK) == 0) { //F_OK: Checks for the existence of the file.
        	printf("Directory already exists: %s\n", db_dir_path);
        	return 1; // Directory exists
    	} else {
        	// Try to create the directory
        	if (mkdir(db_dir_path, 0777) == 0) {
            	printf("Directory created: %s\n", db_dir_path);
            	return 0; // Directory created successfully
        } else {
            perror("Error creating directory");
            return -1; // Error creating directory
        }
    }
}

void list_dbs(const char *home_dir) {
    	// Construct the path to the tracker_databases directory
    	char db_dir_path[512]; //already pointer 
	//snprintf function in this context is used to safely create a string representing path
    	snprintf(db_dir_path, sizeof(db_dir_path), "%s/%s", home_dir, DB_DIRECTORY);
	check_db_folder(home_dir, db_dir_path);
	list_directory_contents(db_dir_path);
}
// Function to create a new SQLite database
int create_database(const char *db_name) {
    	sqlite3 *db;
    	int rc;
	char *home_dir = getenv("HOME");

    	// Construct the full path for the database
    	char db_path[512];
    	snprintf(db_path, sizeof(db_path), "%s/%s", home_dir, db_name);

    	// Try to open or create the database
    	rc = sqlite3_open(db_path, &db);
    	if (rc) {
        	fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        	return -1; // Error opening database
    	}
    
    	printf("Database created/opened successfully: %s\n", db_path);

    	// Close the database
    	sqlite3_close(db);
    	return 0; // Success
}

void open_db() {
}


// Take arguments as input for CRUD operations
// --help 
// --databases (list all)
// --open (db name)
// --show (all, specific)
// --newdb (database name.sqlite)
// --create (new metric) 
// --delete (metric, or database)
// --modify (name of metric, description, or name)




// first argument in argv[] is always the same of the program
int main(int argc, char *argv[]) {
	// Boolean values 1 or 0
	int show_all = 0, new = 0, create = 0, delete = 0, modify = 0;
	if(argc > 4) {
		fprintf(stderr, "Too many arguments, please use --help for help");
		return -1; // Error
	}
	// Parse command-line arguments
	/*
    		0 if the strings are equal.
    		A negative value if str1 is less than str2.
    		A positive value if str1 is greater than str2.
		We compare strings
	*/
	if(strcmp(argv[1], "--help") == 0) {
		print_help();
	} else if(strcmp(argv[1], "--open") == 0) {
		open_db(argv[2]);	
	} else if(strcmp(argv[1], "--databases") == 0) {
		list_dbs(home_dir);	
	} else if(strcmp(argv[1], "--newdb") == 0) {
		create_database(argv[2]);	
	} else {
		fprintf(stderr, "Wrong arguments, please use --help to see more");
	}
	return 0;
}
