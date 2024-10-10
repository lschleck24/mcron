#include "list.h"
#include "mu.h"
#include "mu.h"
#include "unistd.h"
#include "stdio.h"
#include "getopt.h" 
#include <string.h>
#include <stdbool.h>

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



    return 0;
}
