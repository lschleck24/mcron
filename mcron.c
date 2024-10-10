#include "list.h"
#include "mu.h"
#include "mu.h"
#include "unistd.h"
#include "stdio.h"
#include "getopt.h" 
#include <string.h>
#include <stdbool.h>
#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>
#include <signal.h>

const char *pid_file = "mcron.pid";
const char *log_file = "mcron.log";
int log_rotation_count = 0;

struct job {
    unsigned int interval; // Job interval in seconds
    char command[256];     // Command to run
    int job_id;           // Job ID
    struct job *next;      // Pointer to the next job in the list
};

struct job *job_list = NULL;  // Head of the job list
int job_count = 0; 

// Parse a configuration line into a job struct
struct job *job_from_config_line(const char *line) {
    struct job *new_job = malloc(sizeof(struct job));
    if (new_job == NULL) {
        perror("Failed to allocate memory for new job");
        return NULL;
    }

    if (sscanf(line, "%u %255[^\n]", &new_job->interval, new_job->command) != 2) {
        fprintf(stderr, "Invalid config line: %s\n", line);
        free(new_job);
        return NULL;
    }

    new_job->job_id = job_count++; // Assign job ID based on current job count
    new_job->next = NULL;          // Initialize next pointer
    return new_job;
}

void read_config_file(const char *config_file) {
    FILE *file = fopen(config_file, "r");
    if (!file) {
        perror("Failed to open config file");
        return;
    }

    char line[256];
    while (fgets(line, sizeof(line), file)) {
        struct job *new_job = job_from_config_line(line);
        if (new_job) {
            // Add to the linked list of jobs
            new_job->next = job_list;  // Insert at the beginning
            job_list = new_job;
        }
    }

    fclose(file);
}

void handle_sigterm_or_sigint(int sig) {
    (void)sig;
    // Delete the PID file
    if (remove(pid_file) == 0) {
        printf("PID file deleted successfully.\n");
    } else {
        perror("Error deleting PID file");
    }
    // Exit with status 0
    exit(0);
}

void handle_sigusr1(int sig) {
    (void)sig;
    char rotated_log_file[256];
    sprintf(rotated_log_file, "%s-%d", log_file, log_rotation_count++);

    // Rename the log file
    if (rename(log_file, rotated_log_file) == 0) {
        printf("Log file rotated to: %s\n", rotated_log_file);
    } else {
        perror("Error rotating log file");
    }

    // Truncate the active log file
    FILE *file = fopen(log_file, "w");
    if (file) {
        fclose(file);
        printf("Active log file truncated.\n");
    } else {
        perror("Error truncating log file");
    }
}

void handle_sighup(int sig) {
    (void)sig;
    printf("Received SIGHUP: Clearing job list and re-reading configuration file.\n");
    // Simulated re-reading of config file
    // read_config_file("a.conf"); // Uncomment and implement this if you have a config file to read
}

void setup_signal_handlers() {
    struct sigaction sa;

    // Handle SIGTERM and SIGINT
    sa.sa_handler = handle_sigterm_or_sigint;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGTERM, &sa, NULL);
    sigaction(SIGINT, &sa, NULL);

    // Handle SIGUSR1 for log rotation
    sa.sa_handler = handle_sigusr1;
    sigaction(SIGUSR1, &sa, NULL);

    // Handle SIGHUP for re-reading config and clearing jobs
    sa.sa_handler = handle_sighup;
    sigaction(SIGHUP, &sa, NULL);
}

void write_pid_file() {
    FILE *pid_file = fopen("mcron.pid", "w");
    if (pid_file) {
        fprintf(pid_file, "%d\n", getpid());
        fclose(pid_file);
    } else {
        perror("Failed to create pid file");
    }
}
/*
void read_config_file(const char *config_file) {
    FILE *file = fopen(config_file, "r");
    if (!file) {
        perror("Failed to open config file");
        return;
    }

    char line[256];
    while (fgets(line, sizeof(line), file)) {
        struct job *new_job = job_from_config_line(line);
        if (new_job) {
            // Add to your linked list of jobs
        } else {
            fprintf(stderr, "Invalid config line: %s\n", line);
        }
    }

    fclose(file);
}
*/
FILE *open_log_file(const char *log_file) {
    FILE *log_fp = fopen(log_file, "w");  // "w" ensures truncation and overwrite
    if (!log_fp) {
        fprintf(stderr, "Error: Cannot open log file '%s': %s\n", log_file, strerror(errno));
        exit(EXIT_FAILURE);
    }
    return log_fp;
}
int directory_exists(const char *path) {
    struct stat st;
    return (stat(path, &st) == 0 && S_ISDIR(st.st_mode));
}

int 
main(int argc,char *argv[])
{
    /* 
     * TODO: delete the two MU_UNUSED lines below (they are just there so that
     * this file compiles without warnings) and implement the project.  You'll
     * want to parse the command-line arguments in main and create other
     * functions as needed.
     */
    int opt;
    const char *short_opts = "hl:";
    struct option long_opts[] = {
        {"help", no_argument, NULL, 'h'},
        {"log-file", required_argument, NULL, 'l'},
    };

     while((opt = getopt_long(argc, argv, short_opts,long_opts,NULL))!=-1){
        if(opt=='h'){
            printf("usage : \nmcron [-h] [-l LOG_FILE] CONFIG_FILE\n");
            return 0;
        }
        switch(opt){
            case 'l':{
                log_file = optarg;
                break;
            }
            default:
                return 1;
        }
     }
    char *last_slash = strrchr(log_file, '/');
    if (last_slash != NULL) {
        // Extract the directory path from log_file
        char dir_path[1024];
        strncpy(dir_path, log_file, last_slash - log_file);
        dir_path[last_slash - log_file] = '\0';  // Null terminate the directory path

        if (!directory_exists(dir_path)) {
            fprintf(stderr, "Error: Directory '%s' does not exist.\n", dir_path);
            exit(EXIT_FAILURE);
        }
    }
    FILE *log_fp = open_log_file(log_file);
    fclose(log_fp);

    while (1) {
            struct job *current = job_list;
    while (current) {
        sleep(current->interval); // Wait for the job's interval
        // Log the command output
        FILE *log_fp = fopen(log_file, "a"); // Open log file in append mode
        if (log_fp) {
            fprintf(log_fp, "%s\n", current->command); // Log the command
            fclose(log_fp);
        }

        // Here, you can execute the command if needed
        // system(current->command); // Uncomment to actually run the command
        current = current->next; // Move to the next job
    }
    }


    return 0;
}
