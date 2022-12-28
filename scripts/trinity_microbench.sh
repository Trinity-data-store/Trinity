cd build
make

echo "*** tpch ***"
taskset -c 0 /proj/trinity-PG0/Trinity/build/libmdtrie/microbench -b tpch -o SM & 
sleep 10

echo "*** github ***"
taskset -c 1 /proj/trinity-PG0/Trinity/build/libmdtrie/microbench -b github -o SM & 
sleep 10

echo "*** nyc ***"
taskset -c 2 /proj/trinity-PG0/Trinity/build/libmdtrie/microbench -b nyc -o SM & 
sleep 10

echo "*** sensitivity_num_dimensions ***"
taskset -c 3 /proj/trinity-PG0/Trinity/build/libmdtrie/microbench -b sensitivity_num_dimensions -o SM -d 9 & 
sleep 10

taskset -c 14 /proj/trinity-PG0/Trinity/build/libmdtrie/microbench -b sensitivity_num_dimensions -o SM -d 8 & 
sleep 10

taskset -c 15 /proj/trinity-PG0/Trinity/build/libmdtrie/microbench -b sensitivity_num_dimensions -o SM -d 7 & 
sleep 10

taskset -c 16 /proj/trinity-PG0/Trinity/build/libmdtrie/microbench -b sensitivity_num_dimensions -o SM -d 6 & 
sleep 10

taskset -c 17 /proj/trinity-PG0/Trinity/build/libmdtrie/microbench -b sensitivity_num_dimensions -o SM -d 5 & 
sleep 10

taskset -c 18 /proj/trinity-PG0/Trinity/build/libmdtrie/microbench -b sensitivity_num_dimensions -o SM -d 4 & 
sleep 10

echo "*** sensitivity_selectivity ***"
taskset -c 4 /proj/trinity-PG0/Trinity/build/libmdtrie/microbench -b sensitivity_selectivity -o SM
sleep 10

echo "*** sensitivity_optimizations ***"
taskset -c 5 /proj/trinity-PG0/Trinity/build/libmdtrie/microbench -b tpch -o B &
sleep 10

taskset -c 6 /proj/trinity-PG0/Trinity/build/libmdtrie/microbench -b tpch -o CN &
sleep 10

taskset -c 7 /proj/trinity-PG0/Trinity/build/libmdtrie/microbench -b tpch -o GM &
sleep 10

taskset -c 8 /proj/trinity-PG0/Trinity/build/libmdtrie/microbench -b github -o B &
sleep 10

taskset -c 9 /proj/trinity-PG0/Trinity/build/libmdtrie/microbench -b github -o CN &
sleep 10

taskset -c 10 /proj/trinity-PG0/Trinity/build/libmdtrie/microbench -b github -o GM &
sleep 10

taskset -c 11 /proj/trinity-PG0/Trinity/build/libmdtrie/microbench -b nyc -o B &
sleep 10

taskset -c 12 /proj/trinity-PG0/Trinity/build/libmdtrie/microbench -b nyc -o CN &
sleep 10

taskset -c 13 /proj/trinity-PG0/Trinity/build/libmdtrie/microbench -b nyc -o GM &
sleep 10
