#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/time.h>
#include "miprof.c"

#define io_red "\x1b[31m"
#define io_green "\x1b[32m"
#define io_reset "\x1b[0m"

#define BUFFER_SIZE 64
#define COMMAND_BUFFER_SIZE 4

int c_pid;
char cwd[1024];

/*Encargado de dividir la string en cada comando de la pipe
Devuelve la cantidad de comandos divididos por pipe
1er arg: La string que hay que dividir
2do arg: Array de 2 casillas para strings, que almacena las 2 partes*/
int dividirPipe(char* String, char*** StringConPipes){
    int contador=0;
    int capacidad = 3;
    *StringConPipes = malloc(capacidad * sizeof(char*));
    if (!*StringConPipes) {
        write(2, "malloc failed\n", 14);
        return -1;
    }
    char* token = strtok(String,"|");
    while (token!=NULL){
        if (contador >= capacidad) {
            capacidad += 1;
            char** temp = realloc(*StringConPipes, capacidad * sizeof(char*));
            if (!temp) {
                free(*StringConPipes);
                write(2, "realloc failed\n", 15);
                return -1;
            }
            *StringConPipes = temp;
        }
        (*StringConPipes)[contador++] = token;
        token = strtok(NULL, "|");
    }
    return contador;
}
// entregas el buffer que quieres parsear 
// y el puntero a la variable n donde guardaras la cantidad de argumentos
// retorna la string parseada
char** parse(char*buffer, int* n){
    char** args = malloc(COMMAND_BUFFER_SIZE * sizeof(char*));
    if (!args) {
        write(2,"malloc failed",sizeof("malloc failed"));
        return NULL;
    }
    int i=0;
    args[0] = strtok(buffer," \n");
    while (args[i]!=NULL){
        i++;
        if (i%COMMAND_BUFFER_SIZE == COMMAND_BUFFER_SIZE-1){
            char** new_args = realloc(args, (i + COMMAND_BUFFER_SIZE) * sizeof(char*));
            if (!new_args) {
                free(args);
                write(2,"realloc failed",sizeof("realloc failed"));
                return NULL;
            }
            args = new_args;
        }
        args[i] = strtok(NULL," \n");
    }
    if(n) *n=i;
    return args;
}

void ejecutarPipe(char*** comandos,int cantidadPipes){
    //pipefd[0] es para leer, [1] es para escribir
    int pipefd[2];
    int input_fd = STDIN_FILENO;
    pid_t pid;

    for (int i=0;i<cantidadPipes;i++){
        //si no es la ultima entonces crea un pipe
        if(i<cantidadPipes-1){
            if (pipe(pipefd)<0){
            char* errorNoPipe = io_red"error: Pipe couldn not be initialized\n";
            write(2, errorNoPipe, strlen(errorNoPipe));
            return;
            }
        }
        pid = fork();
        if (pid<0){
            //error
            char* error_NoFork_WriteP = io_red"error: Could not fork main process\n";
            write(2, error_NoFork_WriteP, strlen(error_NoFork_WriteP));
            return;
        }
        else if (pid==0){
            //hijo
            //si no es el primero entonces redirige la entrada
            if (input_fd != STDIN_FILENO) {
                dup2(input_fd, STDIN_FILENO);
                close(input_fd);
            }
            //si no es el ultimo entonces redirige la salida
            if (i<cantidadPipes-1){
                //Proceso hijo, ejecutara el comando correspondiente
                dup2(pipefd[1], STDOUT_FILENO);
                close(pipefd[1]); // Una vez duplicado el file descriptor, no necesitamos escribir nada mas
                close(pipefd[0]); // Este proceso no leera datos, solo los escribira
                
            }
            execvp(comandos[i][0], comandos[i]);
            //Si es que el codigo continua aqui, es que el execvp no se realizó correctamente
            char* error_Comando_WriteP = io_red"error: Could not execute pipe command (write)\n";
            write(2, error_Comando_WriteP, strlen(error_Comando_WriteP));
            exit(1);
        } 
        else 
        {
            //padre
            if (input_fd !=STDIN_FILENO) close(input_fd);

            if (i<cantidadPipes-1){
                close(pipefd[1]);
                input_fd=pipefd[0];
            }
        }
    }
    //espera todos los hijos
    for (int i = 0; i < cantidadPipes; i++) wait(NULL);
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

        //Parsea los comandos y argumentos de cada comando separado por pipe
        char** comandosSeparados;
        int cantidadPipes = dividirPipe(buffer, &comandosSeparados);
        char*** comandosParaPipe = malloc(cantidadPipes * sizeof(char**));
        for (int j = 0; j < cantidadPipes; j++) {
            int numArgs;
            comandosParaPipe[j] = parse(comandosSeparados[j], &numArgs);
        }
        //si es que el prompt tiene pipes
        if (cantidadPipes>1) {ejecutarPipe(comandosParaPipe, cantidadPipes);}
        else{
            //parse
            int i=0;
            char **args = parse(buffer, &i);
            //exec command
            if (strcmp(args[0],"cd") == 0)
                chdir(args[1]);
            else if (strcmp(args[0],"exit") == 0)
                exit(0);
            else if (strcmp(args[0],"miprof") == 0)
                miprof(i,args);
            else{
                c_pid = fork();
                if (c_pid==0){
                    execvp(args[0],args);
                    write(2, io_red"error: non existent command\n", strlen(io_red"error: non existent command\n"));
                    exit(1);
                }
                else if (c_pid<0)
                    write(2, io_red"error: can't create process\n", strlen(io_red"error: can't create process\n"));
                else{
                    int status;
                    waitpid(c_pid,&status,0);
                }
            }
            if (args) free(args);
        }

        if (buffer) free(buffer);
        for (int j = 0; j < cantidadPipes; j++) {
            free(comandosParaPipe[j]);
        }
        free(comandosSeparados);
        free(comandosParaPipe);
    }
}