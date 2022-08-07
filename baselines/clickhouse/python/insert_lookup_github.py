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
import fcntl

master = ("10.10.1.2", "9000")
skip_points = int(10000000 / 100) # 10M # Need to run aerospike_insert first!!
total_points = skip_points + int(skip_points / 10) # 11M

num_workers = 20
total_workers = 0

processes = []
total_vect = []
num_data_nodes = 5

zipf_keys = []
zipf_distribution = "/proj/trinity-PG0/Trinity/queries/zipf_keys_30m"
file_path = "/mntData/github_split_10/x{}".format(int(sys.argv[1]))

with open(zipf_distribution) as f:
    i = 0
    for line in f:
        i += 1
        zipf_keys.append(int(line) % skip_points)
        if i >= total_points:
            break

def insert_lookup_each_worker(worker_idx, total_workers):

    client_list = []
    for node_id in range(num_data_nodes):
        client_list.append(Client("10.10.1.{}".format(12 + node_id), "9000"))

    line_count = 0
    insertion_count = 0
    lookup_count = 0
    primary_key_list = []
    start_time = time.time()

    for i in range(worker_idx, total_points, total_workers):

        line_count += 1

        if worker_idx < skip_points or line_count % 20 == 19:
            is_insert = True
        else:
            is_insert = False

        if is_insert:
            insert_list = [total_vect[i]]
            p_key = insert_list[0][0]
            client_list[hash(p_key) % 5].execute("INSERT INTO github_events (pkey, events_count, authors_count, forks, stars, issues, pushes, pulls, downloads, start_date, end_date) VALUES", insert_list)
            primary_key_list.append(p_key)
            insertion_count += 1
        else:
            primary_key_to_query = primary_key_list[zipf_keys[i - skip_points] % len(primary_key_list)]
            client_list[hash(primary_key_to_query) % 5].execute("SELECT * FROM github_events WHERE pkey = {}".format(primary_key_to_query))
            lookup_count += 1

        if line_count % 1000 == 0:
            print(line_count)

    end_time = time.time()

    return line_count / (end_time - start_time)


def insert_lookup_worker(worker, total_workers, return_dict):

    throughput = insert_lookup_each_worker(worker, total_workers)
    return_dict[worker] = {"throughput": throughput}

threads_per_node = 20
total_num_nodes = 10

manager = multiprocessing.Manager()
return_dict = manager.dict()

if int(sys.argv[1]) == 0:
    with open('/proj/trinity-PG0/Trinity/baselines/clickhouse/python/github_insert_lookup_throughput.txt', 'a') as f:
        fcntl.flock(f, fcntl.LOCK_EX)
        print("---- {}K ----".format(int(total_points / 1000)), file=f)
        fcntl.flock(f, fcntl.LOCK_UN)

def load_all_points(client_idx):
    file_path = "/mntData/github_split_10/x{}".format(client_idx)
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
    p = Process(target=insert_lookup_worker, args=(worker, threads_per_node, return_dict, ))
    p.start()
    processes.append(p)

for p in processes:
    p.join()


print("total throughput", sum([return_dict[worker]["throughput"] for worker in return_dict]))
with open('/proj/trinity-PG0/Trinity/baselines/clickhouse/python/github_insert_lookup_throughput.txt', 'a') as f:
    fcntl.flock(f, fcntl.LOCK_EX)
    print(sum([return_dict[worker]["throughput"] for worker in return_dict]), file=f)
    fcntl.flock(f, fcntl.LOCK_UN)
