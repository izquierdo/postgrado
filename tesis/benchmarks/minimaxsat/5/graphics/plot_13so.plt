set encoding iso_8859_1
set terminal postscript eps enhanced "Times-Roman" 26
set output "plot13.eps"
set key bottom
set title "13 subobjetivos"
set ylabel "tiempo"
set xlabel "cantidad de vistas"
set logscale y
plot 'plot_mcd_obj=13' using 1:3 with lp lw 2 lt 1 ps 2 title "McdSat", \
     'plot_mms_obj=13' using 1:3 with lp lw 2 lt 1 ps 2 title "MiniMaxSat"

