import psycopg2
from pgcopy import CopyManager
import pandas as pd 
from datetime import datetime
import time
import sys
import multiprocessing
from multiprocessing import Process
from concurrent.futures import ProcessPoolExecutor
import fcntl

processes = []
total_vect = []
total_points = int(30000000 / 15) # 2M # 5 min
warmup_points = int(total_points * 0.2)
num_data_nodes = 5
file_path = "/mntData/github_split_10/x{}".format(int(sys.argv[1]))

def lookup_each_worker(worker_idx, total_workers):

    connection_list = []
    cursor_list = []

    for data_node_idx in range(num_data_nodes):
        client_ip = "10.10.1.{}".format(12 + data_node_idx)
        CONNECTION = "dbname=defaultdb host={} user=postgres password=adifficultpassword sslmode=disable".format(client_ip)
        CONN = psycopg2.connect(CONNECTION)
        connection_list.append(CONN)
        cursor_list.append(CONN.cursor())

    line_count = 0
    effective_line_count = 0
    start_time = time.time()
    warmup_ended = False

    with open(file_path) as f:
        for line in f:

            line_count += 1

            if line_count % 100000 == 0:
                print(line_count)

            if line_count % total_workers != worker_idx:
                continue

            if line_count > warmup_points:
                if not warmup_ended:
                    start_time = time.time()
                    warmup_ended = True
                effective_line_count += 1

            '''
            Lookup
            '''

            string_list = line.split(",")
            primary_key = int(string_list[0])
            hash_i = hash(primary_key)
            cursor_list[hash_i % 5].execute("SELECT * FROM github_events WHERE pkey = {}".format(primary_key))
            results = cursor_list[hash_i % 5].fetchone()
            if not results:
                print("wrong!", primary_key)
                exit(0)

            '''
            Break
            '''

            del results
            if line_count > total_points:
                break

    end_time = time.time()
    return effective_line_count / (end_time - start_time)

def lookup_worker(worker, total_workers, return_dict):

    throughput = lookup_each_worker(worker, total_workers)
    return_dict[worker] = {"throughput": throughput}

threads_per_node = 20

print("total points", total_points)
if int(sys.argv[1]) == 0:
    with open('/proj/trinity-PG0/Trinity/baselines/timescaleDB/python/github_lookup_throughput.txt', 'a') as f:
        fcntl.flock(f, fcntl.LOCK_EX)
        print("---- {}M ----".format(int(total_points / 1000000)), file=f)
        fcntl.flock(f, fcntl.LOCK_UN)


manager = multiprocessing.Manager()
return_dict = manager.dict()

for worker in range(threads_per_node):
    p = Process(target=lookup_worker, args=(worker, threads_per_node, return_dict, ))
    p.start()
    processes.append(p)

for p in processes:
    p.join()

print("total throughput", sum([return_dict[worker]["throughput"] for worker in return_dict]))
with open('/proj/trinity-PG0/Trinity/baselines/timescaleDB/python/github_lookup_throughput.txt', 'a') as f:
    fcntl.flock(f, fcntl.LOCK_EX)
    print(sum([return_dict[worker]["throughput"] for worker in return_dict]), file=f)
    fcntl.flock(f, fcntl.LOCK_UN)
