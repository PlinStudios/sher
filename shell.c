#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int c_pid;

int main(int argc, char *argv[]){
    while (1){
        printf(">");
        char *args[3];
        args[0] = (char*)malloc(100);
        args[1] = (char*)malloc(100);
        args[2] = NULL;
        scanf("%s %s",args[0],args[1]);

        c_pid = fork();
        if (c_pid==0)
            execvp(args[0],args);
        else if (c_pid<0)
            printf("epic fail\n");
        else{
            int status;
            waitpid(c_pid,&status,0);
        }
    }
}