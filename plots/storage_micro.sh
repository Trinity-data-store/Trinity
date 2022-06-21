python3 bar-chart2.py \
  -d storage_micro.tsv \
  -o storage_micro.pdf \
  --ylabel "{Storage Overhead (bytes/pt)}" \
  --xlabel "Datasets" \
  --ymin 10 --ymax 500 \
  --xscale 0.03 --yscale 0.0175 \
  --colors "green,blue,violet,black,cyan,red" \
  --patterns "grid,crosshatch,dots,north east lines,crosshatch dots,north west lines"
