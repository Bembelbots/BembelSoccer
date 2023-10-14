set autoscale
set datafile sep ','
set title 'localization'
set xlabel 'x'
set ylabel 'y' 

set xrange [-4.5: 4.5]
set yrange [-3.0: 3.0]

do for [idx=1:15000:2] {

plot 'out_avg.csv' every ::1::idx w l ls 1
}
