python3 bar-chart2.py \
  -d primary_lookup_micro.tsv \
  -o primary_lookup_micro.pdf \
  --ylabel "{Latency (us/pt)}" \
  --xlabel "Datasets" \
  --ymin 10 --ymax 100 \
  --xscale 0.03 --yscale 0.0175 \
  --colors "green,blue,violet,black,cyan,red" \
  --patterns "grid,crosshatch,dots,north east lines,crosshatch dots,north west lines"
