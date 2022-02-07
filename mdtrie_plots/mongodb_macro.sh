python3 bar-chart-query.py \
  -d mongodb_macro.tsv \
  -o mongodb_macro.pdf \
  --ylabel "{Latency (ms)}" \
  --xlabel "Systems" \
  --logscale \
  --ymin 100 --ymax 100000 \
  --xscale 0.03 --yscale 0.0175 \
  --colors "green,blue,violet,black,cyan,red" \
  --patterns "grid,crosshatch,dots,north east lines,crosshatch dots,north west lines"
