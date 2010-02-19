unset log                              # remove any log-scaling
unset label                            # remove any previous labels
set xtic auto                          # set xtics automatically
set ytic auto                          # set ytics automatically
set title "Best Model Enumeration Time / Experiment 3"
set xlabel "number of views"
set ylabel "time in seconds"
set key right bottom
set arrow from 0.0028,250 to 0.003,280
set xr [0:80]
set yr [0.1:1000]
set logscale y                       # scale axes automatically
set terminal postscript
set output "| ps2pdf - plot3.pdf"
plot "2sgs.dat" using 1:2 title '2 sub-goals' with linespoints , \
     "3sgs.dat" using 1:3 title '3 sub-goals' with linespoints , \
     "4sgs.dat" using 1:3 title '4 sub-goals' with linespoints , \
     "5sgs.dat" using 1:3 title '5 sub-goals' with linespoints
