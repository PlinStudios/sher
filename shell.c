#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <string.h>

#define io_green "\x1b[32m"
#define io_reset "\x1b[0m"

int c_pid;
char cwd[1024];
char buffer[1024];

int main(int argc, char *argv[]){
    while (1){
        //prompt
        getcwd(cwd, sizeof(cwd));
        write(1, io_green, sizeof(io_green));
        write(1, cwd, strlen(cwd));
        write(1, "> ", 3);
        write(1, io_reset, sizeof(io_reset));

        //read input
        ssize_t n = read(0, buffer, 1024);
        if (n<=0) continue;
        buffer[n]='\0';

        //parse
        char *args[64];
        int i=0;
        args[0] = strtok(buffer," \n");
        while (args[i]!=NULL && i<63)
        {
            i++;
            args[i] = strtok(NULL," \n");
        }
        args[i]=NULL;

        //exec command
        if (strcmp(args[0],"cd") == 0)
            chdir(args[1]);
        else if (strcmp(args[0],"exit") == 0)
            exit(0);
        else{
            c_pid = fork();
            if (c_pid==0)
                execvp(args[0],args);
            else if (c_pid<0)
                write(2, "error", strlen("error"));
            else{
                int status;
                waitpid(c_pid,&status,0);
            }
        }
    }
}