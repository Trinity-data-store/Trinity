python3 bar-chart2.py \
  -d storage_breakdown.tsv \
  -o storage_breakdown_tpch.pdf \
  --ylabel "{Storage (bytes/pt)}" \
  --xlabel "Optimizations" \
  --ymin 10 --ymax 1000 \
  --logscale \
  --xscale 0.03 --yscale 0.0175 \
  --colors "green,blue,violet,black,cyan,red" \
  --patterns "grid,crosshatch,dots,north east lines,crosshatch dots,north west lines"
