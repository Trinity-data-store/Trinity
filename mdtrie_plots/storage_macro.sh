python3 bar-chart2.py \
  -d storage_macro.tsv \
  -o storage_macro.pdf \
  --ylabel "{Storage Overhead (MB)}" \
  --xlabel "Systems" \
  --ymin 500 --ymax 30000 \
  --xscale 0.03 --yscale 0.0175 \
  --colors "green,blue,violet,black,cyan,red" \
  --patterns "grid,crosshatch,dots,north east lines,crosshatch dots,north west lines"
