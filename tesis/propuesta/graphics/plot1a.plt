set terminal postscript eps enhanced "Times-Roman" 26
set output "plot1a.eps"
set key bottom
set title "Experiment 1"
set xlabel "number of views"
set logscale y
plot 'exp1.tiempos.obj=2' using 1:3 with lp lw 2 lt 1 ps 2 title "2 goals in query", \
     'exp1.tiempos.obj=3' using 1:3 with lp lw 2 lt 1 ps 2 title "3 goals in query", \
     'exp1.tiempos.obj=4' using 1:3 with lp lw 2 lt 1 ps 2 title "4 goals in query", \
     'exp1.tiempos.obj=5' using 1:3 with lp lw 2 lt 1 ps 2 title "5 goals in query"


