python3 bar-chart2.py \
  -d sensitivity_latency_dimension.tsv \
  -o sensitivity_latency_dimension.pdf \
  --ylabel "{Latency (us/pt)}" \
  --xlabel "Number of Dimensions" \
  --ymin 1 --ymax 90 \
  --xscale 0.03 --yscale 0.0175 \
  --colors "green,blue,violet,black,cyan,red" \
  --patterns "grid,crosshatch,dots,north east lines,crosshatch dots,north west lines"
