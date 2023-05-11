#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>

#define MAX_LINE 1024
#define MAX_CHILDREN 1024

int isZombie(pid_t pid)
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
void printParent(pid_t pid,int depth)
{
    char command[MAX_LINE];
    snprintf(command, MAX_LINE, "ps -o ppid %d", pid);
    FILE *fp = popen(command, "r");
    if (fp == NULL) {
        printf("Error: Failed to open pipe\n");
        return;
    }
        // Skip the first line of output (headers)
    char buffer[256];
    fgets(buffer, sizeof(buffer), fp);


    if(fgets(buffer, sizeof(buffer), fp) != NULL)
    {
         int parent_pid = atoi(buffer);

        if(depth < 2)
        {
            printf("\n%d\n",parent_pid);
            printParent(parent_pid, depth +1);
        }
    }
    return ;
}
int isRooted(pid_t root, pid_t pid)
{
    if(root == pid)
    {
        return 1;
    }
    char command[MAX_LINE];
    snprintf(command, MAX_LINE, "ps -o ppid %d", pid);
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
        
         int parent_pid = atoi(buffer);

        if (parent_pid == root) 
        {
            return 1;
        }
        else
        {
            return isRooted(root, parent_pid);
        }
    }
    return 0;
}
void process_tree(pid_t root, pid_t pid,int c, int s, int gp, int  gc, int z, int zl,int depth)
{
    if(s == 1)
    {
        char command[MAX_LINE];
        snprintf(command, MAX_LINE, "ps -o ppid %d", pid);
        FILE *fp = popen(command, "r");
        if (fp == NULL) {
            printf("Error: Failed to open pipe\n");
            return;
        }
            // Skip the first line of output (headers)
        char buffer[256];
        fgets(buffer, sizeof(buffer), fp);


        if(fgets(buffer, sizeof(buffer), fp) != NULL)
        {
            pid = atoi(buffer);
        }
    }
    if(gp == 1)
    {
        printParent(pid,0);
        return;
    }
    if(z == 1)
    {
        if(isZombie(pid) == 1)
        {
            printf("\n Defunc %d", pid);
        }
        else
        {
            printf("Not Defunc %d",pid);

        }
        return;
    }
    
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
        int child_ppid = atoi(buffer);
        if (child_ppid == pid) {
            child_pid = atoi(buffer + 5);            
            if (count < MAX_CHILDREN)
            {
                if(c == 1)
                {
                    printf("%d",child_pid);

                }
                if( s== 1 && child_pid != pid)
                {
                    printf("\n%d\n",child_pid);
                }
                if(gc == 1)
                {
                    process_tree(root,child_pid,c,s,gp,gc,z,zl,depth - 1);
                }
                if(zl == 1)
                {
                    if(isZombie(pid) == 1)
                    {
                        printf("\n Defunc %d", pid);
                    }
                }
                // process_tree(child_pid);
            }
        }
    }
    // handle_child_processes(pid, &count);
    pclose(fp);
    
}
int main(int argc, char *argv[])
{
    // check that the correct number of arguments have been provided
    if (argc < 3)
    {
        printf("Usage: %s [root_process] [process_id] [OPTION]\n", argv[0]);
        return 1;
    }
    // get the root process ID and the process ID
    pid_t root_process = atoi(argv[1]);
    pid_t process_id = atoi(argv[2]);


    // // check that the root process ID and process ID are numeric
    // if (!is_numeric(root_process) || !is_numeric(process_id))
    // {
    //     fprintf(stderr, "Error: root_process and process_id must be numeric\n");
    //     return 1;
    // }
    // get the options
    int opt_c = 0; 
    int opt_s = 0;
    int opt_gp = 0;
    int opt_gc = 0;
    int opt_z = 0;
    int opt_zl = 0;
    int option;
    for (int i = 2; i < argc; i++)
    {
        if (strcmp(argv[i], "c") == 0 )
        {
            opt_c = 1;
        }
        else if(strcmp(argv[i], "s") == 0 )
        {
            opt_s = 1;
        }
        else if(strcmp(argv[i], "gp") == 0 )
        {
            opt_gp = 1;
        }
        else if(strcmp(argv[i], "gc") == 0 )
        {
            opt_gc = 1;
        }
        else if(strcmp(argv[i], "z") == 0 )
        {
            opt_z = 1;
        }
        else if(strcmp(argv[i], "lc") == 0 )
        {
            opt_zl = 1;
        }
    }
    // search for the process and output the requested information
    if(isRooted(root_process,process_id) == 1)
    {
        printf("\n%d\n",process_id);
        printParent(process_id,1);
    }
    process_tree(root_process, process_id, opt_c, opt_s, opt_gp, opt_gc, opt_z, opt_zl, 3);
    return 0;
};