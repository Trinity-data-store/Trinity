cd build
make

echo "*** tpch ***"
./libmdtrie/microbench -b tpch -o SM

echo "*** github ***"
./libmdtrie/microbench -b github -o SM

echo "*** nyc ***"
./libmdtrie/microbench -b nyc -o SM

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

echo "*** sensitivity_num_dimensions ***"
./libmdtrie/microbench -b sensitivity_num_dimensions -o SM

echo "*** sensitivity_selectivity ***"
./libmdtrie/microbench -b sensitivity_selectivity -o SM