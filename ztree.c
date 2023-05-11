#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>

#define MAX_CHILDREN 1024
#define MAX_ARGS 64
#define MAX_LINE 1024

int proc_eltime = 1;
int no_of_dfcs = 1;

int bTimeFlag = 0;
int bZCountFlag = 0;


void terminate_process(pid_t pid)
{
    kill(pid, SIGKILL);
    printf("%d\n", pid);
}
void handle_defunct_process(pid_t ppid)
{
    char command[MAX_LINE];
    
    snprintf(command, MAX_LINE, "ps -o etime= -p %d",ppid );
    FILE *fp = popen(command, "r");
    if (fp == NULL)
    {
        perror("popen");
        exit(EXIT_FAILURE);
    }

    char buffer1[256];
    fgets(buffer1, sizeof(buffer1), fp);
    char *token1;  
 
    int time = 0;
    int count = 0;

   token1 = strtok(buffer1, ":");
   
   while( token1 != NULL ) {
      count++;
      token1 = strtok(NULL, ":");
   }
   
    int i = 0;  
    char *token;
    char buffer[256];
    char command1[MAX_LINE];

    snprintf(command1, MAX_LINE, "ps -o etime= -p %d",ppid );
    FILE *fp2 = popen(command1, "r");
    fgets(buffer, sizeof(buffer), fp2);
     token = strtok(buffer, ":");
     while( token != NULL ) {

        int stage = count - i;
        if(stage == 4)
        {
            time += 24*60* atoi(token);
        }
        else if(stage == 3)
        {
            time += 60*atoi(token);
        }
        else if(stage == 2)
        {
            time += atoi(token);
        }
    i++;
    token = strtok(NULL, ":");
   }
    if (time >= proc_eltime)
    {
        terminate_process(ppid);
    }
}
int isZombile(pid_t pid)
{

    char command[MAX_LINE];
    snprintf(command, MAX_LINE, "ps -o stat %d", pid);
    FILE *fp = popen(command, "r");
    if (fp == NULL) {
        printf("Error: Failed to open pipe\n");
        return 1;
    }
        // Skip the first line of output (headers)
    char buffer[256];
    fgets(buffer, sizeof(buffer), fp);

    if(fgets(buffer, sizeof(buffer), fp) != NULL)
    {
        char stat = buffer[0];

        if(stat == 90)
        {
            pclose(fp);
            return 1;
        }
    }
    pclose(fp);
    return 0;
}
void handle_child_processes(pid_t pid, int *count)
{
    pid_t child_pid;
    char buffer0[256];


    FILE *fp0 = popen("whoami","r");
    if (fp0 == NULL) {
        printf("Error: Failed to open pipe\n");
        return ;
    }
    fgets(buffer0, sizeof(buffer0), fp0);

    char command[MAX_LINE];
    snprintf(command, MAX_LINE, "ps -o pid,ppid -u %s", buffer0);
    FILE *fp = popen(command, "r");
    if (fp == NULL) {
        printf("Error: Failed to open pipe\n");
        return ;
    }
    char buffer[256];
    // Skip the first line of output (headers)
    fgets(buffer, sizeof(buffer), fp);

    int child_ppid = 0;
    int zCount = 0;
    // Read each line of output and find the children of the specified process
    while (fgets(buffer, sizeof(buffer), fp) != NULL) {

        int child_ppid = atoi(buffer + 5);
        if (child_ppid == pid) {
            child_pid = atoi(buffer);            
            if ( isZombile(child_pid) == 1)
            {
                zCount++;
            }
        }
    }
    if (zCount >= no_of_dfcs && bTimeFlag != 1)
    {
        terminate_process(pid);
    }
    if(zCount >= 1 && bZCountFlag != 1)
    {
        handle_defunct_process(pid);
    }
}

void process_tree(pid_t pid)
{
    pid_t child_pid;
      char buffer0[256];


    FILE *fp0 = popen("whoami","r");
    if (fp0 == NULL) {
        printf("Error: Failed to open pipe\n");
        return ;
    }
    fgets(buffer0, sizeof(buffer0), fp0);

    char command[MAX_LINE];
    snprintf(command, MAX_LINE, "ps -o pid,ppid -u %s", buffer0);
    
    FILE *fp = popen("ps -o pid,ppid", "r");
    if (fp == NULL) {
        printf("Error: Failed to open pipe\n");
        return ;
    }
    char buffer[256];
    // Skip the first line of output (headers)
    fgets(buffer, sizeof(buffer), fp);

    int count = 0;
    int child_ppid = 0;
    // Read each line of output and find the children of the specified process
    while (fgets(buffer, sizeof(buffer), fp) != NULL &&  count < MAX_CHILDREN) {
        int child_ppid = atoi(buffer + 5);
        if (child_ppid == pid) {
            child_pid = atoi(buffer);            
            if (count < MAX_CHILDREN)
            {
                process_tree(child_pid);
            }
        }
    }
    handle_child_processes(pid, &count);
    pclose(fp);
    
}

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        printf("Usage: ztree [root_process] [OPTION1] [OPTION2]\n");
        exit(EXIT_FAILURE);
    }
    pid_t pid = atoi(argv[1]);
    if (pid <= 0)
    {
        printf("Invalid root process ID\n");
        exit(EXIT_FAILURE);
    }
    int i;
    for (i = 2; i < argc; i++)
    {
        if (strcmp(argv[i], "-t") == 0 && i + 1 < argc)
        {
            bTimeFlag = 1;
            proc_eltime = atoi(argv[++i]);
            if (proc_eltime < 1)
            {
                printf("Invalid elapsed time\n");
                exit(EXIT_FAILURE);
            }
        }
        else if (strcmp(argv[i], "-b") == 0 && i + 1 < argc)
        {
            bZCountFlag = 1;
            no_of_dfcs = atoi(argv[++i]);
            if (no_of_dfcs < 1)
            {
                printf("Invalid number of defunct child processes\n");
                exit(EXIT_FAILURE);
            }
        }
        else
        {
            printf("Invalid option: %s\n", argv[i]);
            exit(EXIT_FAILURE);
        }
    }
    process_tree(pid);
    exit(EXIT_SUCCESS);
}