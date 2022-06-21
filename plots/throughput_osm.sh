python3 bar-chart2.py \
  -d throughput_osm.tsv \
  -o throughput_osm.pdf \
  --ylabel "{Throughput (kOps)}" \
  --xlabel "Systems" \
  --ymin 0 --ymax 1500 \
  --xscale 0.03 --yscale 0.0175 \
  --colors "green,blue,violet,black,cyan,red" \
  --patterns "grid,crosshatch,dots,north east lines,crosshatch dots,north west lines"
