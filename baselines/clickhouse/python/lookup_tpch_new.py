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
total_points = int(100000000 / 1000)
processes = []
total_vect = []
num_data_nodes = 5

def insert_each_worker(worker_idx, total_workers):

    client_list = []
    for node_id in range(num_data_nodes):
        client_list.append(Client("10.10.1.{}".format(12 + node_id), "9000"))
    # client = Client(master[0], master[1])
    line_count = 0
    start_time = time.time()

    for i in range(worker_idx, total_points, total_workers):

        hash_i = hash(i)
        results = client_list[hash_i % num_data_nodes].execute("SELECT * FROM tpch_macro WHERE ID = {}".format(total_vect[i]))
        if not results:
            print("failed!", total_vect)
            exit(0)
        del results
        line_count += 1
        if line_count % 5000 == 0:
            print(line_count)

    end_time = time.time()
    return line_count / (end_time - start_time)

def insert_worker(worker, total_workers, return_dict):

    throughput = insert_each_worker(worker, total_workers)
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
            total_vect.append(string_list[0])
            loaded_lines += 1
            if loaded_lines == total_points:
                break

load_all_points(int(sys.argv[1]))

for worker in range(threads_per_node):
    p = Process(target=insert_worker, args=(worker, threads_per_node, return_dict, ))
    p.start()
    processes.append(p)

for p in processes:
    p.join()

print("total throughput", sum([return_dict[worker]["throughput"] for worker in return_dict]))

with open('/proj/trinity-PG0/Trinity/baselines/clickhouse/python/tpch_lookup_throughput.txt', 'a') as f:
    print(sum([return_dict[worker]["throughput"] for worker in return_dict]), file=f)