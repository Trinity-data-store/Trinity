python3 bar-chart2.py \
  -d throughput_fs.tsv \
  -o throughput_fs.pdf \
  --ylabel "{Throughput (kOps)}" \
  --xlabel "Systems" \
  --ymin 5 --ymax 1200 \
  --xscale 0.03 --yscale 0.0175 \
  --colors "green,blue,violet,black,cyan,red" \
  --patterns "grid,crosshatch,dots,north east lines,crosshatch dots,north west lines"
