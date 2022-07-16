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
  'hosts': [ ('10.10.1.5', 3000), ('10.10.1.6', 3000), ('10.10.1.7', 3000), ('10.10.1.8', 3000), ('10.10.1.9', 3000)]
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

    file_path = "/mntData/tpch_split/x{}".format(worker_idx)
    file_path = "/mntData2/tpch/data_300/tpch_processed_1B.csv"
    file_path = "/mntData2/tpch/data_500/orders_lineitem_merged_indexed.csv"
    cumulative_time = 0
    line_count = 0
    start_time = time.time()

    with open(file_path) as f:
        for line in f:
            
            line_count += 1

            if line_count == 1000000:
                break

            if line_count % total_num_workers != worker_idx:
                continue

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

            start = time.time()

            if rec:
                try:
                    client.put(key, rec)
                except Exception as e:
                    import sys
                    print(key, rec)
                    print("error: {0}".format(e), file=sys.stderr)
                    exit(0)

            cumulative_time += time.time() - start

            if line_count % 100 == 0:
                print(line_count)



    end_time = time.time()
    return line_count / (end_time - start_time), cumulative_time / line_count * 1000


def insert_worker(worker, total_num_workers, return_dict):

    throughput, latency = insert_each_worker(total_num_workers, worker)
    return_dict[worker] = {"throughput": throughput, "latency": latency}

from_worker = 0
to_worker = 60

if len(sys.argv) == 2 and int(sys.argv[1]) == 0:
    from_worker = 0
    to_worker = 20
if len(sys.argv) == 2 and int(sys.argv[1]) == 1:
    from_worker = 20
    to_worker = 40
if len(sys.argv) == 2 and int(sys.argv[1]) == 2:
    from_worker = 40
    to_worker = 60

manager = multiprocessing.Manager()
return_dict = manager.dict()
print(from_worker, to_worker)

for worker in range(from_worker, to_worker):
    p = Process(target=insert_worker, args=(worker, to_worker - from_worker + 1, return_dict, ))
    p.start()
    processes.append(p)

for p in processes:
    p.join()

print("total throughput", sum([return_dict[worker]["throughput"] for worker in return_dict]))
print("average latency (ms)", sum([return_dict[worker]["latency"] for worker in return_dict]) / len(return_dict.keys()))