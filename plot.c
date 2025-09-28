#include <stdlib.h>
#include <stdio.h>

int plot(double x[], double y[], int size) {
    FILE * temp = fopen("data.temp", "w");
    FILE * gp = popen ("gnuplot -persistent", "w");  //persitent mantiene el plot abierto después de que el programa termine
    if (gp == NULL) {
        fprintf(stderr, "Error abriendo archivo \n");
        exit(1);
    }
    int i;
    for (i=0; i < size; i++)
    {
    fprintf(temp, "%lf %lf \n", x[i], y[i]); //Escribimos los datos en un archivo temporal par apoder ser leido por gnuplot
    }

   // Configuramos vista y tipo de output
    fprintf(gp, "set terminal qt\n");  //se abre ventana iteractiva
    fprintf(gp, "set ylabel \"Tiempo de ejecución\"\n");
    fprintf(gp, "set xlabel \"Largo del archivo\"\n");
    fprintf(gp, "set title 'Tiempo de ejecución vs Largo del archivo'\n");
    fprintf(gp, "plot 'data.temp' using 1:2 title 'Data' with linespoints lw 2\n");
    fflush(gp);  //nos aseguramos que se manden los comandos a gnuplot
    fprintf(gp, "set terminal png enhanced font 'arial,10' fontscale 1.0 size 600, 400\n");
    fprintf(gp, "set terminal png background rgb \"0xADD8E6\"\n"); 
    fprintf(gp, "set output 'plot.png'\n");
    fprintf(gp, "replot\n"); 
    fflush(gp);
    printf("Plot guardado como 'plot.png'.\n");
    return 0;
}