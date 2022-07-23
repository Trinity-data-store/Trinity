
# for i in {0..10}
# do
#     timescaledb-parallel-copy --db-name defaultdb --table github_events \
#         --file /mntData2/github_split_10/x$i --copy-options "CSV" \
#         --workers 20 --reporting-period 10s --batch-size 100000 \
#         --connection "host=localhost user=postgres password=adifficultpassword sslmode=disable"
# done

for i in {0..41}
do
    echo "file $i"
    timescaledb-parallel-copy --db-name defaultdb --table github_events \
        --file /mntData2/github_split_10/x$i --copy-options "CSV" \
        --workers 20 --reporting-period 10s --batch-size 100000 \
        --connection "host=localhost user=postgres password=adifficultpassword sslmode=disable"
done