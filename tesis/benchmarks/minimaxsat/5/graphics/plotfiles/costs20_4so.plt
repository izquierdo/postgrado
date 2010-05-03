set encoding iso_8859_1
set terminal postscript eps enhanced "Times-Roman" 26
set output "costs20_4so.eps"
set key bottom
set title "4 subobjetivos (rango de costo 1-20)"
set ylabel "tiempo"
set xlabel "cantidad de vistas"
set logscale y
plot 'data/data_mcd_costs20_obj=4' using 1:3 with lp lw 2 lt 1 ps 2 title "McdSat", \
     'data/data_mms_costs20_obj=4' using 1:3 with lp lw 2 lt 1 ps 2 title "MiniMaxSat"

