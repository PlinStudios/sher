#include <stdlib.h>
#include <stdio.h>

int plot(int size,double x[size], double y[3][size]) {
    FILE * user = fopen("user.temp", "w");
    FILE * real = fopen("real.temp", "w");
    FILE * syst = fopen("system.temp", "w");
    FILE * gp = popen ("gnuplot -persistent", "w");  //persitent mantiene el plot abierto después de que el programa termine
    if (gp == NULL) {
        fprintf(stderr, "Error abriendo archivo \n");
        exit(1);
    }
    int i; 
    for (i=0; i < size; i++){
        fprintf(user, "%lf %lf \n", x[i], y[1][i]); //Escribimos los datos en un archivo temporal par apoder ser leido por gnuplot
        fprintf(real, "%lf %lf \n", x[i], y[0][i]);
        fprintf(syst, "%lf %lf \n", x[i], y[2][i]);
    }


   // Configuramos vista y tipo de output
    fprintf(gp, "set terminal qt\n");  //se abre ventana iteractiva
    fprintf(gp, "set ylabel \"Tiempo de ejecución\"\n");
    fprintf(gp, "set xlabel \"Largo del archivo\"\n");
    fprintf(gp, "set title 'Tiempo de ejecución vs Largo del archivo'\n");
    fprintf(gp, "plot 'user.temp' using 1:2 title 'Tiempo usuario' with linespoints lw 2, \'real.temp' using 1:2 title 'Tiempo real' with linespoints lw 2, \'system.temp' using 1:2 title 'Tiempo sistema' with linespoints lw 2\n");
    fflush(gp);  //nos aseguramos que se manden los comandos a gnuplot
    fprintf(gp, "set terminal png enhanced font 'arial,10' fontscale 1.0 size 600, 400\n");
    fprintf(gp, "set terminal png background rgb \"0xADD8E6\"\n"); 
    fprintf(gp, "set output 'plot.png'\n");
    fprintf(gp, "replot\n"); 
    fflush(gp);
    printf("Plot guardado como 'plot.png'.\n");
    return 0;
}