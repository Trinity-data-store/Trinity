python3 bar-chart2.py \
  -d sensitivity_storage_dataset.tsv \
  -o sensitivity_storage_dataset.pdf \
  --ylabel "{Storage (bytes/pt)}" \
  --xlabel "Fraction of Original Size" \
  --ymin 1 --ymax 50 \
  --xscale 0.03 --yscale 0.0175 \
  --colors "green,blue,violet,black,cyan,red" \
  --patterns "grid,crosshatch,dots,north east lines,crosshatch dots,north west lines"
