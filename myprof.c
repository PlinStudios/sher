#define io_red "\x1b[31m"
#define io_green "\x1b[32m"
#define io_reset "\x1b[0m"

int myprof(int argc, char *argv[]){
    int c_pid = fork();

    if (argc < 3) {write(2, io_red"usage: myprof [ejec|ejecsave file] command args\n"io_reset, sizeof(io_red"usage: myprof [ejec|ejecsave file] command args\n"io_reset)); return 1;}

    if (c_pid==0){
        if (strcmp(argv[1],"ejec") == 0)
            execvp(argv[2],argv+2);
        else if (strcmp(argv[1],"ejecsave") == 0){
            int fd = open(argv[2], O_WRONLY | O_CREAT | O_APPEND, 0644);
            dup2(fd, STDOUT_FILENO);

            for (int i=3; i<argc; i++){
                write(fd, argv[i], strlen(argv[i]));
                write(fd, " ", 1);
            }
            write(fd,"\n", 1);
            
            execvp(argv[3],argv+3);
        }else
            write(2, io_red"usage: myprof [ejec|ejecsave file] command args\n"io_reset, sizeof(io_red"usage: myprof [ejec|ejecsave file] command args\n"io_reset));
            return 1;
    }else{
        int status;
        waitpid(c_pid,&status,0);
        return 0;
    }
}