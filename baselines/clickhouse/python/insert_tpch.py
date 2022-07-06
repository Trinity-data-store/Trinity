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

master = ("10.10.1.2", "9000")
num_workers = 20
total_workers = 0

processes = []

def insert_each_worker(worker_idx):

    client = Client(master[0], port=master[1])
    file_path = "/mntData/tpch_split/x{}".format(worker_idx)
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

            if line_count % 100 == 0:
                print(line_count)

            if line_count == 100000:
                break

    end_time = time.time()
    return line_count / (end_time - start_time), cumulative_time / line_count * 1000

def insert_worker(worker, return_dict):

    throughput, latency = insert_each_worker(worker)
    return_dict[worker] = {"throughput": throughput, "latency": latency}

from_worker = 0
to_worker = 19

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

client = Client(master[0], port=master[1])
# client.execute("TRUNCATE tpch_macro")

for worker in range(from_worker, to_worker):
    p = Process(target=insert_worker, args=(worker, return_dict, ))
    p.start()
    processes.append(p)

for p in processes:
    p.join()

result = client.execute("SELECT COUNT(*) FROM tpch_macro")
print(result)
print("total throughput", sum([return_dict[worker]["throughput"] for worker in return_dict]))
print("average latency (ms)", sum([return_dict[worker]["latency"] for worker in return_dict]) / len(return_dict.keys()))