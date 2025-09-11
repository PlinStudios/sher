#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>

int c_pid;
char cwd[1024];
char buffer[1024];

int main(int argc, char *argv[]){
    while (1){
        //prompt
        getcwd(cwd, sizeof(cwd));
        //printf("%s> ",cwd);
        write(1, cwd, strlen(cwd));
        write(1, "> ", 3);

        //read input
        ssize_t n = read(0, buffer, 1024);
        if (n<=0) continue;
        buffer[n]='\0';

        char *args[3];
        args[0] = strtok(buffer," \n");
        args[1] = strtok(NULL," \n");
        args[2] = NULL;//strtok(NULL," \n");

        //exec command
        printf(">%s %s %s<",args[0],args[1],args[2]);
        if (strcmp(args[0],"cd") == 0)
            chdir(args[1]);
        else if (strcmp(args[0],"exit") == 0)
            exit(0);
        else{
            c_pid = fork();
            if (c_pid==0)
                execvp(args[0],args);
            else if (c_pid<0)
                printf("upsi\n");
            else{
                int status;
                waitpid(c_pid,&status,0);
            }
        }
    }
}