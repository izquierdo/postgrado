set encoding iso_8859_1
set terminal postscript eps enhanced "Times-Roman" 26
set output "plot18.eps"
set key bottom
set title "18 subobjetivos"
set ylabel "tiempo"
set xlabel "cantidad de vistas"
set logscale y
plot 'data_mcd_obj=18' using 1:3 with lp lw 2 lt 1 ps 2 title "McdSat", \
     'data_mms_obj=18' using 1:3 with lp lw 2 lt 1 ps 2 title "MiniMaxSat"

