python3 bar-chart2.py \
  -d sensitivity_range_dimension.tsv \
  -o sensitivity_range_dimension.pdf \
  --ylabel "{Latency (ms)}" \
  --xlabel "Number of Dimensions" \
  --ymin 1 --ymax 120 \
  --xscale 0.03 --yscale 0.0175 \
  --colors "green,blue,violet,black,cyan,red" \
  --patterns "grid,crosshatch,dots,north east lines,crosshatch dots,north west lines"
