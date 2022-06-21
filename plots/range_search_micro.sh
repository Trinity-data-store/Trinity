python3 bar-chart2.py \
  -d range_search_micro.tsv \
  -o range_search_micro.pdf \
  --ylabel "{Latency (ms)}" \
  --xlabel "Datasets" \
  --logscale \
  --ymin 1 --ymax 100000 \
  --xscale 0.03 --yscale 0.0175 \
  --colors "green,blue,violet,black,cyan,red" \
  --patterns "grid,crosshatch,dots,north east lines,crosshatch dots,north west lines"
