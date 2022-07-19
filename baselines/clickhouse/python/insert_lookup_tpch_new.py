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
total_points = int(100000000 / 100)
num_workers = 20
total_workers = 0

processes = []
total_vect = []
num_data_nodes = 5

def insert_lookup_each_worker(worker_idx, total_workers):

    client_list = []
    for node_id in range(num_data_nodes):
        client_list.append(Client("10.10.1.{}".format(12 + node_id), "9000"))

    line_count = 0
    insertion_count = 0
    lookup_count = 0
    primary_key_list = []
    start = time.time()

    for i in range(worker_idx, total_points, total_workers):

        line_count += 1

        if len(primary_key_list) < 1000 or line_count % 20 == 19:
            is_insert = True
        else:
            is_insert = False

        if is_insert:
            insert_list = [total_vect[i]]
            p_key = insert_list[0][0]
            client_list[hash(p_key) % 5].execute("INSERT INTO tpch_macro (ID, QUANTITY, EXTENDEDPRICE, DISCOUNT, TAX, SHIPDATE, COMMITDATE, RECEIPTDATE, TOTALPRICE, ORDERDATE) VALUES", insert_list)
            primary_key_list.append(p_key)
            insertion_count += 1
        else:
            primary_key_to_query = random.choice(primary_key_list)
            client_list[hash(primary_key_to_query) % 5].execute("SELECT * FROM tpch_macro WHERE ID = {}".format(primary_key_to_query))
            lookup_count += 1

        if line_count % 100 == 0:
            print(line_count)

    end_to_end_time = time.time() - start

    return insertion_count, lookup_count, end_to_end_time


def insert_lookup_worker(worker, total_workers, return_dict):

    insertion_count, lookup_count, end_to_end_time = insert_lookup_each_worker(worker, total_workers)
    per_worker_return_dict = {}
    per_worker_return_dict["throughput"] = (lookup_count + insertion_count) / end_to_end_time
    return_dict[worker] = per_worker_return_dict

threads_per_node = 60
total_num_nodes = 10

manager = multiprocessing.Manager()
return_dict = manager.dict()

def load_all_points(client_idx):
    file_path = "/mntData/tpch_split_10/x9"
    loaded_lines = 0
    with open(file_path) as f:
        for line in f:
            string_list = line.split(",")
            total_vect.append([int(entry) for entry in string_list])
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


print("total throughput", sum([return_dict[worker]["throughput"] for worker in return_dict]), flush=True)

with open('/proj/trinity-PG0/Trinity/baselines/clickhouse/python/tpch_insert_lookup_throughput.txt', 'a') as f:
    print(sum([return_dict[worker]["throughput"] for worker in return_dict]), file=f, flush=True)
