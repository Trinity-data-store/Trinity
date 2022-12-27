cd build
make

echo "*** tpch ***"
taskset -c 0 ./libmdtrie/microbench -b tpch -o SM & 
sleep 3

echo "*** github ***"
taskset -c 1 ./libmdtrie/microbench -b github -o SM & 
sleep 3

echo "*** nyc ***"
taskset -c 2 ./libmdtrie/microbench -b nyc -o SM & 
sleep 3

echo "*** sensitivity_num_dimensions ***"
taskset -c 3 ./libmdtrie/microbench -b sensitivity_num_dimensions -o SM & 
sleep 3

echo "*** sensitivity_selectivity ***"
taskset -c 4 ./libmdtrie/microbench -b sensitivity_selectivity -o SM
sleep 3

echo "*** sensitivity_optimizations ***"
./libmdtrie/microbench -b tpch -o B
./libmdtrie/microbench -b tpch -o CN
./libmdtrie/microbench -b tpch -o GM
./libmdtrie/microbench -b github -o B
./libmdtrie/microbench -b github -o CN
./libmdtrie/microbench -b github -o GM
./libmdtrie/microbench -b nyc -o B
./libmdtrie/microbench -b nyc -o CN
./libmdtrie/microbench -b nyc -o GM