#define io_red "\x1b[31m"
#define io_green "\x1b[32m"
#define io_reset "\x1b[0m"

#define throwusagemsg write(2, io_red"usage: miyprof [ejec|ejecsave file|ejecutar maxtiempo(ms)] command args\n"io_reset, sizeof(io_red"usage: myprof [ejec|ejecsave file|ejecutar maxtiempo(ms)] command args\n"io_reset))

#include <stdio.h>
#include <sys/resource.h>
#include <time.h>
#include "plot.c"

void timeout_handler(int sig) {
    _exit(0);
}

long getFileSize(const char *filename) {
    FILE *fp = fopen(filename, "rb");
    if (fp == NULL) {
        printf("Error: Could not open file %s\n", filename);
        return -1; // error
    }
    fseek(fp, 0, SEEK_END);
    long size = ftell(fp);
    fclose(fp);

    return size;
}

void save_sorted_flag(int sorted) {
    FILE *file = fopen("/tmp/sorted_flag", "w");
    if (file != NULL) {
        fprintf(file, "%d", sorted); // guardamos la flag de sorted (0 or 1)
        fclose(file);
    }
}

int read_sorted_flag() {
    int sorted = 0;
    FILE *file = fopen("/tmp/sorted_flag", "r");
    if (file != NULL) {
        fscanf(file, "%d", &sorted); // leemos la flag (0 or 1)
        fclose(file);
    }
    return sorted;
}

int miprof(int argc, char *argv[]){
    int fd = 1;
    int sorted=0;
    double x[100]={ 0 };
    double y[3][100]={ 0 };
    int index=0;

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
            if (strcmp(argv[2], "sort") == 0){ sorted=1; save_sorted_flag(sorted);}
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
        
            if (strcmp(argv[3], "sort") == 0){ sorted=1; save_sorted_flag(sorted);}
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

            if (strcmp(argv[3], "sort") == 0){ sorted=1; save_sorted_flag(sorted); }
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

        sorted=read_sorted_flag();
        /*if (sorted==1){
            const char *file_to_sort = argv[argc - 1];
            long file_size = getFileSize(file_to_sort);
            if (file_size != -1) {
                    x[index]=(double)file_size;
            }
            y[0][index]=real;
            y[1][index]=user;
            y[2][index]=sys;
            plot(index,x,y);
            index++;
            sorted=0;
            save_sorted_flag(0);
        }*/

        dprintf(fd,"Real: %fs | User: %fs | System: %fs | MaxRSS: %ld KB\n", real, user, sys, usage.ru_maxrss);

        if (fd!=1) close(fd);

        return 0;
    }
}