python3 bar-chart2.py \
  -d sensitivity_latency_dataset.tsv \
  -o sensitivity_latency_dataset.pdf \
  --ylabel "{Latency (us/pt)}" \
  --xlabel "Fraction of Original Size" \
  --ymin 1 --ymax 80 \
  --xscale 0.03 --yscale 0.0175 \
  --colors "green,blue,violet,black,cyan,red" \
  --patterns "grid,crosshatch,dots,north east lines,crosshatch dots,north west lines"
