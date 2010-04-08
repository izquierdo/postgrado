set encoding iso_8859_1
set terminal postscript eps enhanced "Times-Roman" 26
set output "plot40views.eps"
set key bottom
set title "40 vistas"
set ylabel "tiempo"
set xlabel "cantidad de subobjetivos"
set logscale y
plot 'plot_mcd_views=40' using 2:3 with lp lw 2 lt 1 ps 2 title "McdSat", \
     'plot_mms_views=40' using 2:3 with lp lw 2 lt 1 ps 2 title "MiniMaxSat"

