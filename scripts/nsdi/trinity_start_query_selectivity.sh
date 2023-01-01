cd /proj/trinity-PG0/Trinity/build/
make

for i in {1..3}
do
    /proj/trinity-PG0/Trinity/build/libmdtrie/sensitivity_query_selectivity &
done
