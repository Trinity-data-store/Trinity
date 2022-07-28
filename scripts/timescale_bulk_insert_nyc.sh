for i in {0..39}
do
    echo "file $i"
    timescaledb-parallel-copy --db-name defaultdb --table nyc_taxi \
        --file /mntData2/nyc_taxi/nyc_taxi_split_675/x$i --copy-options "CSV" \
        --workers 20 --reporting-period 10s --batch-size 100000 \
        --connection "host=localhost user=postgres password=adifficultpassword sslmode=disable"
done