set encoding iso_8859_1
set terminal postscript eps enhanced "Times-Roman" 26
set output "pot2_4so.eps"
set key bottom
set title "4 subobjetivos (incr. de costo potencias de 2)"
set ylabel "tiempo"
set xlabel "cantidad de vistas"
set logscale y
plot 'data/data_mcd_pot2_obj=4' using 1:3 with lp lw 2 lt 1 ps 2 title "McdSat", \
     'data/data_mms_pot2_obj=4' using 1:3 with lp lw 2 lt 1 ps 2 title "MiniMaxSat"

