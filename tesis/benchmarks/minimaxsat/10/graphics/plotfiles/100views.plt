set encoding iso_8859_1
set terminal postscript eps enhanced "Times-Roman" 26
set output "graphic_100views.eps"
set key bottom
set title "100 vistas"
set ylabel "tiempo"
set xlabel "cantidad de objetivos"
set logscale y
plot 'data/data_mcd_views=100' using 2:3 with lp lw 2 lt 1 ps 2 title "McdSat", \
     'data/data_mms_views=100' using 2:3 with lp lw 2 lt 1 ps 2 title "MiniMaxSat"

