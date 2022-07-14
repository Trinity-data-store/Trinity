from clickhouse_driver import Client
import time
import random
import sys
import random
from datetime import datetime
import multiprocessing
from multiprocessing import Process
from concurrent.futures import ProcessPoolExecutor
from threading import Thread, Lock

# master = ("10.10.1.{}".format(int(sys.argv[1]) + 2), "9000")
# master = ("10.10.1.2", "9000")

num_workers = 20
total_workers = 0

processes = []

def insert_each_worker(worker_idx):

    offset = worker_idx % 5
    client = Client("10.10.1.{}".format(12 + offset), "9000")
    client = Client(master[0], master[1])
    file_path = "/mntData/tpch_split_600/x{}".format(worker_idx)
    cumulative_time = 0
    line_count = 0
    start_time = time.time()

    with open(file_path) as f:
        for line in f:
            line_count += 1
            string_list = line.split(",")
            insert_list = [[int(entry) for entry in string_list]]

            start = time.time()
            client.execute("INSERT INTO tpch_macro (ID, QUANTITY, EXTENDEDPRICE, DISCOUNT, TAX, SHIPDATE, COMMITDATE, RECEIPTDATE, TOTALPRICE, ORDERDATE) VALUES", insert_list)
            cumulative_time += time.time() - start

            if line_count % 5000 == 0:
                print(line_count)

            if line_count == 100000:
                break

    end_time = time.time()
    return line_count / (end_time - start_time)

def insert_worker(worker, return_dict):

    throughput = insert_each_worker(worker)
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

client = Client(master[0], port=master[1])

for worker in range(from_worker, to_worker):
    p = Process(target=insert_worker, args=(worker, return_dict, ))
    p.start()
    processes.append(p)

for p in processes:
    p.join()

print("total throughput", sum([return_dict[worker]["throughput"] for worker in return_dict]))

with open('tpch_throughput.txt', 'a') as f:
    print(sum([return_dict[worker]["throughput"] for worker in return_dict]), file=f)