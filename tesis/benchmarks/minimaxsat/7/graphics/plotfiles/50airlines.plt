set encoding iso_8859_1
set terminal postscript eps enhanced "Times-Roman" 26
set output "50airlines.eps"
set key bottom
set title "50 aerolineas (100 vistas)"
set ylabel "tiempo"
set xlabel "cantidad de objetivos"
set logscale y
plot 'data/data_mcd_airlines=50' using 2:3 with lp lw 2 lt 1 ps 2 title "McdSat", \
     'data/data_mms_airlines=50' using 2:3 with lp lw 2 lt 1 ps 2 title "MiniMaxSat"

