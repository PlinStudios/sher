#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <string.h>

#define io_red "\x1b[31m"
#define io_green "\x1b[32m"
#define io_reset "\x1b[0m"

#define BUFFER_SIZE 64
#define COMMAND_BUFFER_SIZE 4

int c_pid;
char cwd[1024];

/*Encargado de dividir en 2 la string, el primer comando antes del pipes y el resto de la string
Devuelve 1 en caso de tener pipe, 0 si no tiene
1er arg: La string que hay que dividir
2do arg: Array de 2 casillas para strings, que almacena las 2 partes*/
int dividirPipe(char* String, char** StringConPipes){
    StringConPipes[0] = strtok(String, "|");
    StringConPipes[1] = strtok(NULL, "");
    if (StringConPipes[1] == NULL) return 0;
    else return 1;
}

int main(int argc, char *argv[]){
    write(2, io_red, sizeof(io_red));
    while (1){
        //prompt
        getcwd(cwd, sizeof(cwd));
        write(1, io_green, sizeof(io_green));
        write(1, cwd, strlen(cwd));
        write(1, "> ", 3);
        write(1, io_reset, sizeof(io_reset));

        //Lectura de Input
        //Deja en el puntero buffer la string que contiene todo el input del usuario
        //long long int size almacena el tamaño de la string total
        long long int size = 0;
        char*buffer = (char *)malloc(BUFFER_SIZE * sizeof(char));
        ssize_t n = read(0, buffer, BUFFER_SIZE-1);
        if (n<=1) continue;
        size+=n;
        while (n==BUFFER_SIZE-1 && buffer[size-1]!='\n'){
            buffer = realloc(buffer, (size + BUFFER_SIZE) * sizeof(char));
            n = read(0,buffer+size,BUFFER_SIZE-1);
            size+=n;
            if (n<=0) break;
        }
        buffer[size]='\0';

        //parse
        char **args = (char **)malloc(COMMAND_BUFFER_SIZE * sizeof(char *));
        int i=0;
        args[0] = strtok(buffer," \n");
        while (args[i]!=NULL)
        {
            i++;
            args[i] = strtok(NULL," \n");

            if (i%COMMAND_BUFFER_SIZE == COMMAND_BUFFER_SIZE-1)
            args = realloc(args, (i+COMMAND_BUFFER_SIZE+1) * sizeof(char *));
        }
        args[i]=NULL;

        //exec command
        if (strcmp(args[0],"cd") == 0)
            chdir(args[1]);
        else if (strcmp(args[0],"exit") == 0)
            exit(0);
        else{
            c_pid = fork();
            if (c_pid==0){
                char a = execvp(args[0],args);
                write(2, io_red"error: non existent command\n", strlen(io_red"error: non existent command\n"));
                exit(0);
            }
            else if (c_pid<0)
                write(2, io_red"error: can't create process\n", strlen(io_red"error: can't create process\n"));
            else{
                int status;
                waitpid(c_pid,&status,0);
            }
        }
    }
}


void ejecutarPipe(char** comando, char** comandoPostPipe){
    //pipefd[0] es para leer, [1] es para escribir
    int pipefd[2]; 
    pid_t pWrite, pRead;
    if (pipe(pipefd)<0){
        char* errorNoPipe = io_red"error: Pipe couldn not be initialized\n";
        write(2, errorNoPipe, strlen(errorNoPipe));
        return;
    }
    pWrite = fork();
    if (pWrite<0){
        char* error_NoFork_WriteP = io_red"error: Could not fork main process\n";
        write(2, error_NoFork_WriteP, strlen(error_NoFork_WriteP));
        return;
    }
    if (pWrite==0){
        //Proceso hijo, ejecutara el comando correspondiente
        close(pipefd[0]); // Este proceso no leera datos, solo los escribira
        dup2(pipefd[1], STDOUT_FILENO);
        close(pipefd[1]); // Una vez duplicado el file descriptor, no necesitamos escribir nada mas
        execvp(parsed[0], parsed);
        //Si es que el codigo continua aqui, es que el execvp no se realizó correctamente
        char* error_Comando_WriteP = io_red"error: Could not execute pipe command (write)\n";
        write(2, error_Comando_WriteP, strlen(error_Comando_WriteP));
    } 
    else 
    {
        //Proceso padre, creara un nuevo hijo para que lea el resultado del anterior
        pRead = fork();
        if (pRead<0){
            char* error_NoFork_ReadP = io_red"error: Could not fork pipe process\n";
            write(2, error_NoFork_ReadP, strlen(error_NoFork_ReadP));
            return;
        }
        if (pRead==0){

            //Proceso hijo, ejecutara el comando correspondiente
            close(pipefd[1]); // Este proceso no escribira datos, solo los leera
            dup2(pipefd[0], STDIN_FILENO);
            close(pipefd[0]); // Una vez duplicado el file descriptor, no necesitamos escribir nada mas

            /*NOTA: Para tener pipes "ilimitados", deberiamos llamar nuevamente a la función ejecutarPipe, pero
            parseando correctamente a la string para ver si es que es necesario usar pipes o este es el ultimo comando*/

            execvp(parsed[0], parsed);
            //Si es que el codigo continua aqui, es que el execvp no se realizó correctamente
            char* error_Comando_ReadP = io_red"error: Could not execute pipe command (read)\n";
            write(2, error_Comando_ReadP, strlen(error_Comando_ReadP));

        } else {
            close(pipefd[0]);
            close(pipefd[1]);
            wait(NULL);
            wait(NULL);

        }

    }

}