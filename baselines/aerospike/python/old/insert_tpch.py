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

# Create a client and connect it to the cluster
# Records are addressable via a tuple of (namespace, set, key)
header = ["quantity", "extendedprice", "discount", "tax", "shipdate", "commitdate", "recepitdate", "totalprice", "orderdate"]
processes = []

def insert_each_worker(total_num_workers, worker_idx):

    try:
        client = aerospike.client(config).connect()
    except:
        import sys
        print("failed to connect to the cluster with", config['hosts'])
        sys.exit(1)

    file_path = "/mntData/tpch_split_600/x{}".format(worker_idx)
    # file_path = "/mntData2/tpch/data_300/tpch_processed_1B.csv"
    # file_path = "/mntData/tpch/tpch_processed_1B.csv"
    cumulative_time = 0
    line_count = 0
    effective_line_count = 0
    start_time = time.time()
    warmed_up = False
    with open(file_path) as f:
        for line in f:

            line_count += 1

            if line_count > 200000 and not warmed_up: # WARMUP
                warmed_up = True
                start_time = time.time()

            if warmed_up:
                effective_line_count += 1

            string_list = line.split(",")

            colnum = 0
            primary_key = 0
            rec = {}
            for col in string_list:
                if colnum == 0:
                    primary_key = col
                else:
                    rec[header[colnum - 1]] = int(col)
                colnum += 1

            key = ('tpch', 'tpch_macro', str(primary_key))

            # start = time.time()
            if rec:
                try:
                    client.put(key, rec)
                except Exception as e:
                    import sys
                    print(key, rec)
                    print("error: {0}".format(e), file=sys.stderr)
                    exit(0)
            # cumulative_time += time.time() - start

            if effective_line_count and effective_line_count % 500000 == 0:
                print(effective_line_count)

    end_time = time.time()
    # print(effective_line_count, end_time - start_time)
    return effective_line_count / (end_time - start_time)


def insert_worker(worker, total_num_workers, return_dict):

    throughput = insert_each_worker(total_num_workers, worker)
    return_dict[worker] = {"throughput": throughput}

threads_per_node = 60
total_num_nodes = 10

if len(sys.argv) == 2:
    from_worker = int(sys.argv[1]) * threads_per_node
    to_worker = int(sys.argv[1]) * threads_per_node + threads_per_node
    total_num_workers = threads_per_node * total_num_nodes
else:
    exit(0)

manager = multiprocessing.Manager()
return_dict = manager.dict()
print(from_worker, to_worker)

for worker in range(from_worker, to_worker):
    p = Process(target=insert_worker, args=(worker, total_num_workers, return_dict, ))
    p.start()
    processes.append(p)

for p in processes:
    p.join()

print("total throughput", sum([return_dict[worker]["throughput"] for worker in return_dict]))

with open('tpch_throughput.txt', 'a') as f:
    print(sum([return_dict[worker]["throughput"] for worker in return_dict]), file=f)