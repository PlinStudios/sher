#define io_red "\x1b[31m"
#define io_green "\x1b[32m"
#define io_reset "\x1b[0m"

#define throwusagemsg write(2, io_red"usage: miyprof [ejec|ejecsave file] command args\n"io_reset, sizeof(io_red"usage: myprof [ejec|ejecsave file] command args\n"io_reset))

int miprof(int argc, char *argv[]){
    if (argc < 3) {throwusagemsg; return 1;}

    int c_pid = fork();

    if (c_pid==0){
        if (strcmp(argv[1],"ejec") == 0){
            execvp(argv[2],argv+2);
            write(2, io_red"non existent command\n"io_reset, sizeof(io_red"non existent command\n"io_reset));
            exit(0);
        }else if (strcmp(argv[1],"ejecsave") == 0){
            if (argc < 4) {throwusagemsg; exit(0);}

            int fd = open(argv[2], O_WRONLY | O_CREAT | O_APPEND, 0644);
            dup2(fd, STDOUT_FILENO);

            for (int i=3; i<argc; i++){
                write(fd, argv[i], strlen(argv[i]));
                write(fd, " ", 1);
            }
            write(fd,"\n", 1);
            
            execvp(argv[3],argv+3);
            write(2, io_red"non existent command\n"io_reset, sizeof(io_red"non existent command\n"io_reset));
            exit(0);
        }else
            throwusagemsg;
            exit(0);
    }else{
        int status;
        waitpid(c_pid,&status,0);
        return 0;
    }
}