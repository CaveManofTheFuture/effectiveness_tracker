#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#define MAX_INPUT_LENGTH 20 // Define a maximum input length for safety
#define DB_DIRECTORY "metric_app/" // Define the directory structure
#define MAX_USERS 100
#define MAX_METRICS 100 // For each user 
#define MAX_NAME_LENGTH 50
#define MAX_FILE_PATH 512

// Structure to hold metric data
typedef struct {
    char name[MAX_NAME_LENGTH];
    int totalScore;
    int daysCounted;
} Metric;

// Structure to hold user data
typedef struct {
    char username[MAX_NAME_LENGTH];
    Metric metrics[MAX_METRICS];
    int metricCount;
} User;

// Function prototypes
void addMetric(User *user);
void deleteMetric(User *user);
void recordScore(User *user);
void saveData(User *user);
void loadData(User *user);
void displayMetrics(User *user);
void initializeUser(User *user, const char *username);
void createDataDirectory();

int main() {
    User user;
    char username[MAX_NAME_LENGTH];

    // Create data directory if it doesn't exist
    createDataDirectory();

    // Get username from the user safely
    printf("Enter your username: ");
    if (fgets(username, sizeof(username), stdin) != NULL) {
        // Remove newline character if present
        username[strcspn(username, "\n")] = '\0';
        initializeUser(&user, username);
    } else {
        fprintf(stderr, "Error reading username.\n");
        return 1; // Exit the program with an error code
    }

    // Load existing data
    loadData(&user);
    
    // Choice logic
    char input[MAX_INPUT_LENGTH];
    int choice;
    
   do {
        printf("\nMenu:\n");
        printf("1. Add Metric\n");
        printf("2. Record Score\n");
        printf("3. Display Metrics\n");
        printf("4. Save Data\n");
	printf("5. Delete Metric\n");
        printf("6. Exit\n");
        printf("Choose an option: ");

        // Use fgets to read input safely
        if (fgets(input, sizeof(input), stdin) != NULL) {
            // Convert input to an integer
            char *endptr;
            choice = strtol(input, &endptr, 10); // Converts the string in input to a long integer (base 10).

            // Check for conversion errors
            if (endptr == input || *endptr != '\n') {
                printf("Invalid input. Please enter a number between 1 and 5.\n");
                continue; // Skip the rest of the loop and prompt again
            }

            // Check if the choice is within the valid range
            if (choice < 1 || choice > 6) {
                printf("Invalid choice. Please try again.\n");
                continue; // Skip the rest of the loop and prompt again
            }

            // Process the valid choice
            switch (choice) {
                case 1:
                    addMetric(&user);
                    break;
                case 2:
                    recordScore(&user);
                    break;
                case 3:
                    displayMetrics(&user);
                    break;
                case 4:
                    saveData(&user);
                    break;
                case 5: 
		    deleteMetric(&user); 
                case 6:
                    printf("Exiting the program.\n");
                    break;
            }
        } else {
            printf("Error reading input. Please try again.\n");
        }
    } while (choice != 6);
 
    return 0;
    
}

// Function to create data directory
void createDataDirectory() {
    char *home_dir = getenv("HOME"); // Get the user's home directory
    if (home_dir == NULL) {
        fprintf(stderr, "Error: Unable to get home directory.\n");
        return;
    }

    char db_dir_path[512]; 
    snprintf(db_dir_path, sizeof(db_dir_path), "%s/%s", home_dir, DB_DIRECTORY); // Construct the full path

    struct stat st = {0};

    // Check if the directory exists, if not, create it
    if (stat(db_dir_path, &st) == -1) {
        if (mkdir(db_dir_path, 0700) == -1) {
            fprintf(stderr, "Error: Unable to create directory '%s'.\n", db_dir_path);
        } else {
            printf("Directory '%s' created successfully.\n", db_dir_path);
        }
    } else {
        printf("Directory '%s' already exists.\n", db_dir_path);
    }
}

// Function to initialize user data
void initializeUser(User *user, const char *username) {
    strcpy(user->username, username);
    user->metricCount = 0;
}

// Function to delete a metric
void deleteMetric(User *user) {
    char metricName[MAX_NAME_LENGTH];

    printf("Enter metric name to delete: ");
    if (fgets(metricName, sizeof(metricName), stdin) != NULL) {
        // Remove newline character if present
        metricName[strcspn(metricName, "\n")] = '\0';
    } else {
        fprintf(stderr, "Error reading metric name.\n");
        return;
    }

    // Find the metric and delete it
    for (int i = 0; i < user->metricCount; i++) {
    	// If strings match 
        if (strcmp(user->metrics[i].name, metricName) == 0) {
            // Shift metrics down to fill the gap
            for (int j = i; j < user->metricCount - 1; j++) {
                user->metrics[j] = user->metrics[j + 1];
            }
            user->metricCount--; // Decrease the count of metrics
            printf("Metric '%s' deleted successfully.\n", metricName);
            return;
        }
    }

    printf("Metric '%s' not found.\n", metricName);
}

// Function to add a new metric
void addMetric(User *user) {
    if (user->metricCount >= MAX_METRICS) {
        printf("Maximum number of metrics reached.\n");
        return;
    }

    Metric newMetric;
    printf("Enter metric name: ");
    if (fgets(newMetric.name, sizeof(newMetric.name), stdin) != NULL) {
        // Remove newline character if present
        newMetric.name[strcspn(newMetric.name, "\n")] = '\0';
        newMetric.totalScore = 0;
        newMetric.daysCounted = 0;

        user->metrics[user->metricCount] = newMetric;
        user->metricCount++;
        printf("Metric '%s' added successfully.\n", newMetric.name);
    } else {
        fprintf(stderr, "Error reading metric name.\n");
    }
}



// Function to record a score for a metric
void recordScore(User *user) {
    char metricName[MAX_NAME_LENGTH];
    int score;

    printf("Enter metric name to record score: ");
    if (fgets(metricName, sizeof(metricName), stdin) != NULL) {
        // Remove newline character if present
        metricName[strcspn(metricName, "\n")] = '\0';
    } else {
        fprintf(stderr, "Error reading metric name.\n");
        return;
    }

    printf("Enter score (1 or 0): ");
    if (scanf("%d", &score) != 1 || (score != 0 && score != 1)) {
        fprintf(stderr, "Invalid score. Please enter 1 or 0.\n");
        // Clear the input buffer
        while (getchar() != '\n');
        return;
    }
    // Clear the input buffer after reading the score
    while (getchar() != '\n');

    // Find the metric and update the score
    for (int i = 0; i < user->metricCount; i++) {
        if (strcmp(user->metrics[i].name, metricName) == 0) {
            user->metrics[i].totalScore += score;
            user->metrics[i].daysCounted++;
            printf("Score recorded for metric '%s'.\n", metricName);
            return;
        }
    }

    printf("Metric '%s' not found.\n", metricName);
    
}


// Function to save user data to a CSV file
void saveData(User *user) {
    char *home_dir = getenv("HOME"); // Get the user's home directory
    if (home_dir == NULL) {
        fprintf(stderr, "Error: Unable to get home directory.\n");
        return;
    }

    char filePath[MAX_FILE_PATH];
    snprintf(filePath, sizeof(filePath), "%s/%s/%s.csv", home_dir, DB_DIRECTORY, user->username); // Construct the full file path

    FILE *file = fopen(filePath, "w");
    if (file == NULL) {
        fprintf(stderr, "Error opening file for writing: '%s'.\n", filePath);
        return;
    }

    fprintf(file, "Metric Name,Total Score,Days Counted\n");
    for (int i = 0; i < user->metricCount; i++) {
        fprintf(file, "%s,%d,%d\n", user->metrics[i].name, user->metrics[i].totalScore, user->metrics[i].daysCounted);
    }

    fclose(file);
    printf("Data saved successfully to '%s'.\n", filePath);
}

// Function to load user data from a CSV file
void loadData(User *user) {
    char *home_dir = getenv("HOME"); // Get the user's home directory
    if (home_dir == NULL) {
        fprintf(stderr, "Error: Unable to get home directory.\n");
        return;
    }

    char filePath[MAX_FILE_PATH];
    snprintf(filePath, sizeof(filePath), "%s/%s/%s.csv", home_dir, DB_DIRECTORY, user->username); // Construct the full file path

    FILE *file = fopen(filePath, "r");
    if (file == NULL) {
        printf("No existing data found for user '%s'.\n", user->username);
        return;
    }

    char line[256];
    fgets(line, sizeof(line), file); // Skip header line

    while (fgets(line, sizeof(line), file)) {
        Metric newMetric;
        sscanf(line, "%[^,],%d,%d", newMetric.name, &newMetric.totalScore, &newMetric.daysCounted);
        user->metrics[user->metricCount] = newMetric;
        user->metricCount++;
    }

    fclose(file);
    printf("Data loaded successfully for user '%s'.\n", user->username);
    
}

// Function to display metrics for the user
void displayMetrics(User *user) {
    if (user->metricCount == 0) {
        printf("No metrics found for user '%s'.\n", user->username);
        return;
    }

    printf("\nMetrics for user '%s':\n", user->username);
    printf("--------------------------------------------------\n");
    printf("%-20s %-12s %-12s\n", "Metric Name", "Total Score", "Days Counted");
    printf("--------------------------------------------------\n");
    for (int i = 0; i < user->metricCount; i++) {
        printf("%-20s %-12d %-12d\n", user->metrics[i].name, user->metrics[i].totalScore, user->metrics[i].daysCounted);
    }
    printf("--------------------------------------------------\n");
    
}

