cd build
make

echo "*** sensitivity_treeblock_size ***"

taskset -c 3 /proj/trinity-PG0/Trinity/build/libmdtrie/microbench -b sensitivity_treeblock_size -o SM -d 9 -t 128
sleep 10

taskset -c 4 /proj/trinity-PG0/Trinity/build/libmdtrie/microbench -b sensitivity_treeblock_size -o SM -d 9 -t 256
sleep 10

taskset -c 5 /proj/trinity-PG0/Trinity/build/libmdtrie/microbench -b sensitivity_treeblock_size -o SM -d 9 -t 384
sleep 10

taskset -c 6 /proj/trinity-PG0/Trinity/build/libmdtrie/microbench -b sensitivity_treeblock_size -o SM -d 9 -t 512
sleep 10

taskset -c 7 /proj/trinity-PG0/Trinity/build/libmdtrie/microbench -b sensitivity_treeblock_size -o SM -d 9 -t 640
sleep 10

taskset -c 8 /proj/trinity-PG0/Trinity/build/libmdtrie/microbench -b sensitivity_treeblock_size -o SM -d 9 -t 768
sleep 10

taskset -c 9 /proj/trinity-PG0/Trinity/build/libmdtrie/microbench -b sensitivity_treeblock_size -o SM -d 9 -t 896
sleep 10

taskset -c 10 /proj/trinity-PG0/Trinity/build/libmdtrie/microbench -b sensitivity_treeblock_size -o SM -d 9 -t 1024
sleep 10
