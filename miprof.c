#define io_red "\x1b[31m"
#define io_green "\x1b[32m"
#define io_reset "\x1b[0m"

#define throwusagemsg write(2, io_red"usage: miyprof [ejec|ejecsave file|ejecutar maxtiempo(ms)] command args\n"io_reset, sizeof(io_red"usage: myprof [ejec|ejecsave file|ejecutar maxtiempo(ms)] command args\n"io_reset))

#include <stdio.h>
#include <sys/resource.h>
#include <time.h>

void timeout_handler(int sig) {
    _exit(0);
}

int miprof(int argc, char *argv[]){
    int fd = 1;

    if (strcmp(argv[1],"ejecsave") == 0){
        if (argc < 4) {throwusagemsg; return 1;}
        fd = open(argv[2], O_WRONLY | O_CREAT | O_APPEND, 0644);
    }else
        if (argc < 3) {throwusagemsg; return 1;}

    struct timespec start, end;
    clock_gettime(CLOCK_REALTIME, &start); //tiempo inicial

    int c_pid = fork();

    if (c_pid==0){
        if (strcmp(argv[1],"ejec") == 0){
            execvp(argv[2],argv+2);
            write(2, io_red"non existent command\n"io_reset, sizeof(io_red"non existent command\n"io_reset));
            exit(0);
        }else if (fd != 1){ //ejecsave
            dup2(fd, STDOUT_FILENO);

            //guarda el comando
            for (int i=3; i<argc; i++){
                write(fd, argv[i], strlen(argv[i]));
                write(fd, " ", 1);
            }
            write(fd,"\n", 1);
            
            execvp(argv[3],argv+3);
            write(2, io_red"non existent command\n"io_reset, sizeof(io_red"non existent command\n"io_reset));
            exit(0);
        }else if (strcmp(argv[1],"ejecutar") == 0){
            if (argc < 4) {throwusagemsg; exit(0);}

            
            signal(SIGALRM, timeout_handler);

            struct itimerval timer;
            timer.it_value.tv_sec = atoi(argv[2]) / 1000;
            timer.it_value.tv_usec = (atoi(argv[2]) % 1000) * 1000;
            timer.it_interval.tv_sec = 0;
            timer.it_interval.tv_usec = 0;

            setitimer(ITIMER_REAL, &timer, NULL);

            execvp(argv[3],argv+3);
            write(2, io_red"non existent command\n"io_reset, sizeof(io_red"non existent command\n"io_reset));
            exit(0);
        }else
            throwusagemsg;
            exit(0);
    }else{
        int status;
        struct rusage usage;
        //espera proceso hijo
        wait4(c_pid,&status,0,&usage);

        clock_gettime(CLOCK_REALTIME, &end); //tiempo final

        //tiempos
        double real = (end.tv_sec-start.tv_sec) + (end.tv_nsec-start.tv_nsec)/1000000000.0;
        double user = usage.ru_utime.tv_sec + usage.ru_utime.tv_usec/1000000.0;
        double sys = usage.ru_stime.tv_sec + usage.ru_stime.tv_usec/1000000.0;

        dprintf(fd,"Real: %fs | User: %fs | System: %fs | MaxRSS: %ld KB\n", real, user, sys, usage.ru_maxrss);

        if (fd!=1) close(fd);

        return 0;
    }
}