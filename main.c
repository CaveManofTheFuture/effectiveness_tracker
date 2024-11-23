#include <stdio.h>
#include <ctype.h>
#include <sqlite3.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <dirent.h>

// home directory/tracker_databases
#define DB_DIRECTORY "tracker_databases"
#define MAX_INPUT_LENGTH 50

//Purpose of program: track daily advancements in user-specified directions of life. Score based system.

//functions
void print_help() {
	printf("To use this utility you need to create database first\n");
	printf("To create new database use --newdb with name you want as your database name\n");
	printf("To list all databases use --databases\n");
	printf("To open and do operations on your database use --open with your database name\n");
}

//Function to list the contents of a directory
int list_directory_contents(const char *path) {
    struct dirent *entry;
    DIR *dp = opendir(path);

    if (dp == NULL) {
        perror("opendir");
        return -1;
    }

    printf("Contents of directory '%s':\n", path);
    while ((entry = readdir(dp)) != NULL) {
        // Skip the current (.) and parent (..) directories
        if (entry->d_name[0] != '.') {
            printf(" - %s\n", entry->d_name);
        }
    }

    closedir(dp);
    return 0;
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

int list_dbs() {
	char *home_dir = getenv("HOME");
    	// Construct the path to the tracker_databases directory
    	char db_dir_path[512]; //already pointer 
	//snprintf function in this context is used to safely create a string representing path
    	snprintf(db_dir_path, sizeof(db_dir_path), "%s/%s", home_dir, DB_DIRECTORY);
	//error handling
	int checkdb_result = check_db_folder(home_dir, db_dir_path);
    	if (checkdb_result != 0) {
        // Check if the result is 1 (specific failure) or any other non-zero value
        	if (checkdb_result == 1) {
            		fprintf(stderr, "Directory already exists\n");
        	} else {
            		fprintf(stderr, "An error occurred while creating the database: %d\n", checkdb_result);
			return -1;
        	}
    	}
		
	//error handling
    	if (list_directory_contents(db_dir_path) != 0) {
            	fprintf(stderr, "An error occurred while creating the database\n");
		return -1;
    	}
		
	return 0;
}
// Function to create a new SQLite database
int create_database(const char *db_name) {
	char *home_dir = getenv("HOME");

    	char db_full_name[512]; //already pointer 
    	snprintf(db_full_name, sizeof(db_full_name), "%s/%s/%s.db", home_dir, DB_DIRECTORY, db_name);
    	// Construct the path to the tracker_databases directory
	sqlite3 *db;
    	char *err_msg = 0;

    	// Open the database (creates it if it doesn't exist)
    	int exit = sqlite3_open(db_full_name, &db);
    	if (exit != SQLITE_OK) {
        	fprintf(stderr, "Error opening database: %s\n", sqlite3_errmsg(db));
        	return exit; // Return the error code
    	}

    	// SQL statement to create the Habits table
    	const char *sql = "CREATE TABLE IF NOT EXISTS Habits ("
                      	  "ID INTEGER PRIMARY KEY AUTOINCREMENT, "
                          "Name TEXT NOT NULL, "
                          "Description TEXT, "
                          "TotalScore INTEGER DEFAULT 0);";

    	// Execute the SQL statement
    	exit = sqlite3_exec(db, sql, 0, 0, &err_msg);
    	if (exit != SQLITE_OK) {
        	fprintf(stderr, "Error creating table: %s\n", err_msg);
        	sqlite3_free(err_msg);
        	sqlite3_close(db);
        	return exit; // Return the error code
    	}

    	printf("Database and table created successfully!\n");

    	// Close the database
    	sqlite3_close(db);
    	return 0; // Success
}
/*
int is_valid_input(const char *input) {
    // Check if the input is empty
    if (input[0] == '\0') {
        return -1; // Invalid
    }

    // Check each character to ensure it's alphabetic
    for (int i = 0; input[i] != '\0'; i++) {
        if (!isalpha(input[i])) {
            return -1; // Invalid
        }
    }
    return 0; // Valid
}
*/

int add_metric(sqlite3 *db, const char *name, const char *description) {
    const char *sql = "INSERT INTO Habits (Name, Description) VALUES (?, ?);";
    sqlite3_stmt *stmt;

    int exit = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
    if (exit != SQLITE_OK) {
        fprintf(stderr, "Error preparing statement: %s\n", sqlite3_errmsg(db));
        return exit;
    }

    // Bind parameters
    sqlite3_bind_text(stmt, 1, name, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, description, -1, SQLITE_STATIC);

    exit = sqlite3_step(stmt);
    if (exit != SQLITE_DONE) {
        fprintf(stderr, "Error inserting data: %s\n", sqlite3_errmsg(db));
    } else {
        printf("Habit added successfully!\n");
    }

    sqlite3_finalize(stmt);
    return exit;
}

void view_habits(sqlite3 *db) {
    const char *sql = "SELECT * FROM Habits;";
    sqlite3_stmt *stmt;

    int exit = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
    if (exit != SQLITE_OK) {
        fprintf(stderr, "Error preparing statement: %s\n", sqlite3_errmsg(db));
        return;
    }

    printf("ID\tName\t\tDescription\t\tTotalScore\n");
    printf("-----------------------------------------------------\n");

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int id = sqlite3_column_int(stmt, 0);
        const char *name = (const char *)sqlite3_column_text(stmt, 1);
        const char *description = (const char *)sqlite3_column_text(stmt, 2);
        int total_score = sqlite3_column_int(stmt, 3);

        printf("%d\t%s\t%s\t%d\n", id, name, description, total_score);
    }

    sqlite3_finalize(stmt);
}


int option_one(sqlite3 *db ) {
	//taking input from the user	
	char name[MAX_INPUT_LENGTH];
	int name_bool = 0; 
	char description[MAX_INPUT_LENGTH];
	printf("Enter a name for metric (only alphabetic characters are allowed): \n");	 
    	// Use fgets to safely read input
	scanf("%s", name);
	printf("Enter a description for metric (only alphabetic characters are allowed): \n");	 
	scanf("%s", description);
	add_metric(db, name, description);
	return 0;

	
}

int option_two(sqlite3 *db ) {
	//taking input from the user	
	view_habits(db);
	return 0;
}


int open_db(const char *db_name) {
	char *home_dir = getenv("HOME");
    	char db_full_name[512]; //already pointer 
    	snprintf(db_full_name, sizeof(db_full_name), "%s/%s/%s", home_dir, DB_DIRECTORY, db_name);
	sqlite3 *db;
    	// Open the database (creates it if it doesn't exist)
    	int try_db = sqlite3_open(db_full_name, &db);
    	if (try_db!= SQLITE_OK) {
        	fprintf(stderr, "Error opening database: %s\n", sqlite3_errmsg(db));
        	return try_db; // Return the error code
    	}
	int choice;
	while (1) { // Infinite loop
        	// Display options to the user
        	printf("Menu:\n");
        	printf("1. Add metric\n");
        	printf("2. View all metrics\n");
        	printf("3. Modify metric\n");
        	printf("4. Delete metric\n");
        	printf("0. Exit\n");
        	printf("Enter your choice: ");

        	// Read user input and sanitize it
        	while (1) {
            		// Attempt to read an integer
            		if (scanf("%d", &choice) == 1) {
                		// Valid input received
                		break; // Exit the input loop
            		} else {
                		// If input is not an integer, clear the input buffer
                		printf("Invalid input. Please enter a number: ");
                		while (getchar() != '\n'); // Clear the input buffer
            		}
        	}

        	// Process the user's choice using if statements
        	if (choice == 1) {
            		printf("You selected Option 1.\n");
			if(option_one(db) != 0) {
				fprintf(stderr, "Error occured, try again");	
				return -1; 
			} else {
				printf("Added metric successfully");
			}
        	} else if (choice == 2) {
            		printf("You selected Option 2.\n");
			if(option_two(db) != 0) {
				fprintf(stderr, "Error occured, try again");	
				return -1; 
			} else {
				printf("Added metric successfully");
			}
        	} else if (choice == 3) {
            		printf("You selected Option 3.\n");
        	} else if(choice == 4){
		
		} else if (choice == 0) {
            		printf("Exiting the program.\n");
            		exit(0); 
        	} else {
            		printf("Invalid choice. Please try again.\n");
        	}

        	printf("\n"); // Print a newline for better readability
    	}
	return 0;
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
		if (list_dbs() != 0) {
			printf("Could not list directory. Try again\n");
        		return -1;
    		}
	} else if(strcmp(argv[1], "--newdb") == 0) {
		if (create_database(argv[2]) != 0) {
			printf("Could not create database. Try again\n");
        		return -1;
    		}
		printf("To use Database use --open");
	} else {
		fprintf(stderr, "Wrong arguments, please use --help to see more");
	}
	return 0;
}
