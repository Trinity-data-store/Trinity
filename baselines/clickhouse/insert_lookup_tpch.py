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
import re

master = ("10.10.1.2", "9000")
num_workers = 20
total_workers = 0

processes = []

def insert_lookup_each_worker(worker_idx):

    client = Client(master[0], port=master[1])
    file_path = "/mntData/tpch_split/tpch_split_split/x{}".format(worker_idx)
    cumulative_time = 0
    line_count = 0
    insertion_count = 0
    lookup_count = 0
    insertion_latency_cumulative = 0
    lookup_latency_cumulative = 0
    primary_key_list = []

    with open(file_path) as f:
        for line in f:
            line_count += 1
            string_list = line.split(",")
            insert_list = [[int(entry) for entry in string_list]]
            primary_key_list.append(insert_list[0][0])
            if line_count % 10 != 9:
                is_insert = True
            else:
                is_insert = False

            primary_key_to_query = random.choice(primary_key_list)
            start = time.time()
            if is_insert:
                client.execute("INSERT INTO tpch_macro (ID, QUANTITY, EXTENDEDPRICE, DISCOUNT, TAX, SHIPDATE, COMMITDATE, RECEIPTDATE, TOTALPRICE, ORDERDATE) VALUES", insert_list)
            else:
                client.execute("SELECT * FROM tpch_macro WHERE ID = {}".format(primary_key_to_query))
            elapsed_time = time.time() - start
            cumulative_time += elapsed_time

            if is_insert:
                insertion_count += 1
                insertion_latency_cumulative += elapsed_time
            else: 
                lookup_count += 1
                lookup_latency_cumulative += elapsed_time

            if line_count % 100 == 0:
                print(line_count)

            if line_count == 30000:
                break

    return insertion_count, insertion_latency_cumulative, lookup_count, lookup_latency_cumulative, cumulative_time


def insert_lookup_worker(worker, return_dict):

    insertion_count, insertion_latency_cumulative, lookup_count, lookup_latency_cumulative, cumulative_time = insert_lookup_each_worker(worker)
    per_worker_return_dict = {}
    per_worker_return_dict["insertion latency"] = (insertion_latency_cumulative / insertion_count) * 1000
    per_worker_return_dict["lookup latency"] = (lookup_latency_cumulative / lookup_count) * 1000
    per_worker_return_dict["total throughput"] = (lookup_count + insertion_count) / cumulative_time
    return_dict[worker] = per_worker_return_dict

from_worker = 0
to_worker = 19

manager = multiprocessing.Manager()
return_dict = manager.dict()
print(from_worker, to_worker)

client = Client(master[0], port=master[1])
# client.execute("TRUNCATE tpch_macro")
result = client.execute("SELECT COUNT(*) FROM tpch_macro")
print("initialized? count: ", result[0])

for worker in range(from_worker, to_worker):
    p = Process(target=insert_lookup_worker, args=(worker, return_dict, ))
    p.start()
    processes.append(p)

for p in processes:
    p.join()

throughput_list = []
insertion_latency_list = []
lookup_latency_list = []

for worker in range(from_worker, to_worker):
    throughput_list.append(return_dict[worker]["total throughput"])
    insertion_latency_list.append(return_dict[worker]["insertion latency"])
    lookup_latency_list.append(return_dict[worker]["lookup latency"])

print(throughput_list)
print(insertion_latency_list)
print(lookup_latency_list)
print("total throughput", sum(throughput_list))
print("average insertion latency (ms)", sum(insertion_latency_list) / len(insertion_latency_list))
print("average lookup latency (ms)", sum(lookup_latency_list) / len(lookup_latency_list))
