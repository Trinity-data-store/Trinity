# Not ready!
for i in {0..9}
do
    ssh -o StrictHostKeyChecking=no -i /proj/trinity-PG0/Trinity/scripts/key -l Ziming 10.10.1.$(($i % 5 + 12)) "timescaledb-parallel-copy --db-name defaultdb --table tpch_macro \
    --file /mntData/tpch_split_10/x$i --copy-options \"CSV\" \
    --workers 10 --reporting-period 10s -limit 5000000 --batch-size 100000 \
    --connection \"host=localhost user=postgres password=adifficultpassword sslmode=disable\"" &
done
