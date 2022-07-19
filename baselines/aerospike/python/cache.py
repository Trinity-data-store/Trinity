import aerospike
import sys
import csv
import time
import random
import sys
import random
from datetime import datetime
import multiprocessing
from multiprocessing import Process
from concurrent.futures import ProcessPoolExecutor
from threading import Thread, Lock


config = {
  'hosts': [ ('10.10.1.12', 3000), ('10.10.1.13', 3000), ('10.10.1.14', 3000), ('10.10.1.15', 3000), ('10.10.1.16', 3000)]
}

header = ["quantity", "extendedprice", "discount", "tax", "shipdate", "commitdate", "recepitdate", "totalprice", "orderdate"]
processes = []
total_vect = []
num_data_nodes = 5
skip_points = int(5000000) # 5M # Need to run aerospike_insert first!!
begin_measuring = int(5000000) # 6M
stop_measuring = int(10000000) # 8M
total_points = int(10000000) # 10M

def insert_lookup_each_worker(total_num_workers, worker_idx):

    try:
        client_insert = aerospike.client(config).connect()
        client_lookup = aerospike.client(config).connect()
    except:
        import sys
        print("failed to connect to the cluster with", config['hosts'])
        sys.exit(1)

    line_count = 0
    effective_line_count = 0
    start_time = time.time()
    end_time = 0
    insertion_count = 0
    lookup_count = 0

    primary_key_list = []
    for i in range(skip_points):
        primary_key_list.append(total_vect[i][0])

    # print(primary_key_list[:5], len(primary_key_list))
    # exit(0)
    started_measuring = False

    for i in range(skip_points + worker_idx, total_points, total_num_workers):

        line_count += 1
        if not started_measuring and i > begin_measuring and i <= stop_measuring:
            started_measuring = True
            start_time = time.time()
            # client_insert.close()
            # client_lookup.close()
            # client_insert = aerospike.client(config).connect()
            # client_lookup = aerospike.client(config).connect()
        
        if started_measuring and i >= stop_measuring:
            started_measuring = False
            end_time = time.time()

        if started_measuring:
            effective_line_count += 1

        if False and (not started_measuring or len(primary_key_list) < 1000 or line_count % 20 == 19):
            is_insert = True
        else:
            is_insert = False

        if is_insert:
            primary_key, rec = total_vect[i]
            key = ('tpch', 'tpch_macro', primary_key)
            if rec:
                try:
                    client_insert.put(key, rec)
                except Exception as e:
                    import sys
                    print(key, rec)
                    print("error: {0}".format(e), file=sys.stderr)
                    exit(0)
            del key 
            del rec
            # primary_key_list.append(primary_key)
            insertion_count += 1
        else:
            # primary_key_to_query = random.choice(primary_key_list)
            primary_key_to_query =  primary_key_list[i - skip_points]
            key = ('tpch', 'tpch_macro', primary_key_to_query)
            try:
                (_, _, _) = client_lookup.get(key)
            except Exception as e:
                import sys
                print(key)
                print("error: {0}".format(e), file=sys.stderr)
                exit(0)
            lookup_count += 1

        if line_count % (total_points / 10) == 0:
            print(line_count)

    if not end_time:
        end_time = time.time()

    print("effective_line_count ", effective_line_count)
    return effective_line_count / (end_time - start_time)


def insert_lookup_worker(worker, total_workers, return_dict):

    throughput = insert_lookup_each_worker(total_workers, worker)
    return_dict[worker] = {"throughput": throughput}

threads_per_node = 60
total_num_nodes = 10

manager = multiprocessing.Manager()
return_dict = manager.dict()

def load_all_points(client_idx):
    file_path = "/mntData/tpch_split_10/x{}".format(client_idx)
    loaded_lines = 0
    with open(file_path) as f:
        for line in f:

            string_list = line.split(",")
            colnum = 0
            rec = {}
            primary_key = 0
            for col in string_list:
                if colnum == 0:
                    primary_key = col
                else:
                    rec[header[colnum - 1]] = int(col)
                colnum += 1

            total_vect.append((primary_key, rec))
            loaded_lines += 1
            if loaded_lines == total_points:
                break

load_all_points(int(sys.argv[1]))

for worker in range(threads_per_node):
    p = Process(target=insert_lookup_worker, args=(worker + int(sys.argv[1]) * threads_per_node, threads_per_node * total_num_nodes, return_dict))
    p.start()
    processes.append(p)

for p in processes:
    p.join()

print("total throughput", sum([return_dict[worker]["throughput"] for worker in return_dict]))
with open('/proj/trinity-PG0/Trinity/baselines/aerospike/python/tpch_insert_lookup_throughput.txt', 'a') as f:
    print(sum([return_dict[worker]["throughput"] for worker in return_dict]), file=f)