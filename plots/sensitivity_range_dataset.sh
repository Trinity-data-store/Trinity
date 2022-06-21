python3 bar-chart2.py \
  -d sensitivity_range_dataset.tsv \
  -o sensitivity_range_dataset.pdf \
  --ylabel "{Latency (ms)}" \
  --xlabel "Fraction of Original Size" \
  --ymin 10 --ymax 130 \
  --xscale 0.03 --yscale 0.0175 \
  --colors "green,blue,violet,black,cyan,red" \
  --patterns "grid,crosshatch,dots,north east lines,crosshatch dots,north west lines"
