#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>

int c_pid;
char cwd[1024];

int main(int argc, char *argv[]){
    while (1){
        getcwd(cwd, sizeof(cwd));
        printf("%s>",cwd);
        char *args[3];
        args[0] = (char*)malloc(100);
        args[1] = (char*)malloc(100);
        args[2] = NULL;
        scanf("%s %s",args[0],args[1]);

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