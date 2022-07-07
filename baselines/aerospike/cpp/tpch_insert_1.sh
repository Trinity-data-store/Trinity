# sudo gcc -DAS_USE_LIBEV -o tpch_async tpch_async.c -laerospike -lev -lssl -lcrypto -lpthread -lm -lz

for i in $(seq 0 19); 
do 
    sudo ./async /mntData/tpch_split/x$i &
done

# sudo rm async