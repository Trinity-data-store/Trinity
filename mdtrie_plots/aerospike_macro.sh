python3 bar-chart-query.py \
  -d aerospike_macro.tsv \
  -o aerospike_macro.pdf \
  --ylabel "{Latency (ms)}" \
  --xlabel "Systems" \
  --logscale \
  --ymin 1 --ymax 500000 \
  --xscale 0.03 --yscale 0.0175 \
  --colors "green,blue,violet,black,cyan,red" \
  --patterns "grid,crosshatch,dots,north east lines,crosshatch dots,north west lines"
