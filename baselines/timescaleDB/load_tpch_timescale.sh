for i in {264..300}
# for i in {201..250}
do
    timescaledb-parallel-copy --db-name tpch_macro --table tpch \
        --file /mntData2/tpch_split/x$i --copy-options "CSV" \
        --workers 20 --reporting-period 30s --batch-size 100000 \
        --connection "host=localhost user=postgres password=adifficultpassword sslmode=disable"
    echo "$i"
done