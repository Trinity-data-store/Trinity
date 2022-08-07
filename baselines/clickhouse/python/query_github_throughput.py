from clickhouse_driver import Client
import time
import random
import sys
import random
from datetime import datetime
import re
import multiprocessing
from multiprocessing import Process
from concurrent.futures import ProcessPoolExecutor
from threading import Thread, Lock
import fcntl

master = ("10.10.1.{}".format(int(sys.argv[1]) + 2), "9000")

filename = "/proj/trinity-PG0/Trinity/queries/github/github_query_final"

with open(filename) as file:
    lines = file.readlines()
    lines = [line.rstrip() for line in lines]

client = Client(master[0], port=master[1])
total_points = 50000000
total_queries = 100 # should be 500
total_vect = []
processes = []

def load_all_points(client_idx):
    file_path = "/mntData/github_split_10/x{}".format(client_idx)
    loaded_lines = 0
    with open(file_path) as f:
        for line in f:
            string_list = line.split(",")
            string_list = string_list
            total_vect.append([int(entry) for entry in string_list])
            loaded_lines += 1
            if loaded_lines == total_points:
                break

load_all_points(int(sys.argv[1]))
client.execute("INSERT INTO github_events (pkey, events_count, authors_count, forks, stars, issues, pushes, pulls, downloads, start_date, end_date) VALUES", total_vect)
threads_per_node = 20

def query_each_worker(worker_idx, total_workers):

    client = Client(master[0], port=master[1])
    effective_pts_count = 0
    start_time = time.time()

    for i in range(worker_idx, total_queries, total_workers):

        line = lines[i]
        query = line.split(",")[0] + ";"

        query_select = query.replace("COUNT(*)", "*").replace("github_events_final", "github_events")

        results = client.execute(query_select)
        effective_pts_count += len(results)
        print("i: ", i, "len(results)", len(results))
        del results

    end_time = time.time()
    return effective_pts_count / (end_time - start_time)

def query_worker(worker, total_workers, return_dict):

    throughput = query_each_worker(worker, total_workers)
    return_dict[worker] = {"throughput": throughput}

manager = multiprocessing.Manager()
return_dict = manager.dict()

if int(sys.argv[1]) == 0:
    with open('/proj/trinity-PG0/Trinity/baselines/clickhouse/python/github_search_throughput.txt', 'a') as f:
        fcntl.flock(f, fcntl.LOCK_EX)
        print("---- {}K ----".format(int(total_points / 1000)), file=f)
        fcntl.flock(f, fcntl.LOCK_UN)

for worker in range(threads_per_node):
    p = Process(target=query_worker, args=(worker, threads_per_node, return_dict, ))
    p.start()
    processes.append(p)

for p in processes:
    p.join()

print("total throughput", sum([return_dict[worker]["throughput"] for worker in return_dict]))

with open('/proj/trinity-PG0/Trinity/baselines/clickhouse/python/github_search_throughput.txt', 'a') as f:
    fcntl.flock(f, fcntl.LOCK_EX)
    print(sum([return_dict[worker]["throughput"] for worker in return_dict]), file=f)
    fcntl.flock(f, fcntl.LOCK_UN)


