set terminal postscript eps enhanced "Times-Roman" 14
set output "plot1.eps"
set key bottom
set logscale y
set logscale y2
set size 1,.5
set multiplot
set size .5,.5
set origin 0,0
set title "Experiment I"
set xlabel "number of airlines \n (a)"
plot 'exp1.tiempos.obj=2' using 1:3 with lp lw 1.5 lt 1 ps 1.5 title "2 goals in query", \
     'exp1.tiempos.obj=3' using 1:3 with lp lw 1.5 lt 1 ps 1.5 title "3 goals in query", \
     'exp1.tiempos.obj=4' using 1:3 with lp lw 1.5 lt 1 ps 1.5 title "4 goals in query", \
     'exp1.tiempos.obj=5' using 1:3 with lp lw 1.5 lt 1 ps 1.5 title "5 goals in query"
set origin .5,0
set title "Experiment II"
set xlabel "number of airlines \n (b)"
set y2tics
set format y ""
plot 'exp2.tiempos.obj=2' using 1:3 with lp lw 1.5 lt 1 ps 1.5 title "2 goals in query", \
     'exp2.tiempos.obj=3' using 1:3 with lp lw 1.5 lt 1 ps 1.5 title "3 goals in query", \
     'exp2.tiempos.obj=4' using 1:3 with lp lw 1.5 lt 1 ps 1.5 title "4 goals in query", \
     'exp2.tiempos.obj=5' using 1:3 with lp lw 1.5 lt 1 ps 1.5 title "5 goals in query"
unset multiplot

