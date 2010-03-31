set encoding iso_8859_1
set terminal postscript eps enhanced "Times-Roman" 26
set output "plot3.eps"
set key bottom
set title "Experimento III"
set ylabel "tiempo"
set xlabel "número cantidad de vistas"
set logscale y
plot 'exp3.tiempos.obj=2' using 1:3 with lp lw 2 lt 1 ps 2 title "2 objetivos en las vistas", \
     'exp3.tiempos.obj=3' using 1:3 with lp lw 2 lt 1 ps 2 title "3 objetivos en las vistas", \
     'exp3.tiempos.obj=4' using 1:3 with lp lw 2 lt 1 ps 2 title "4 objetivos en las vistas", \
     'exp3.tiempos.obj=5' using 1:3 with lp lw 2 lt 1 ps 2 title "5 objetivos en las vistas"

