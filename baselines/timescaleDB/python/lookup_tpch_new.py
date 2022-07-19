import psycopg2
from pgcopy import CopyManager
import pandas as pd 
from datetime import datetime
import time
import sys
import multiprocessing
from multiprocessing import Process
from concurrent.futures import ProcessPoolExecutor

processes = []
total_vect = []
total_points = int(1000000) # 1M
num_data_nodes = 5

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
    start_time = time.time()

    for i in range(worker_idx, total_points, total_workers):

        hash_i = hash(total_vect[i])
        cursor_list[hash_i % 5].execute("SELECT * FROM tpch_macro WHERE ID = {}".format(total_vect[i]))
        results = cursor_list[hash_i % 5].fetchone()
    
        if line_count and line_count % 10000 == 0:
            print(line_count, flush=True)
        
        line_count += 1

    end_time = time.time()
    return line_count / (end_time - start_time)

def lookup_worker(worker, total_workers, return_dict):

    throughput = lookup_each_worker(worker, total_workers)
    return_dict[worker] = {"throughput": throughput}

threads_per_node = 60
total_num_nodes = 10

def load_all_points(client_idx):
    file_path = "/mntData/tpch_split_10/x{}".format(client_idx)
    loaded_lines = 0
    with open(file_path) as f:
        for line in f:
            string_list = line.split(",")
            id = int(string_list[0])
            total_vect.append(id)
            loaded_lines += 1
            if loaded_lines == total_points:
                break

load_all_points(int(sys.argv[1]))

manager = multiprocessing.Manager()
return_dict = manager.dict()

for worker in range(threads_per_node):
    p = Process(target=lookup_worker, args=(worker, threads_per_node, return_dict, ))
    p.start()
    processes.append(p)

for p in processes:
    p.join()

print("total throughput", sum([return_dict[worker]["throughput"] for worker in return_dict]))
with open('/proj/trinity-PG0/Trinity/baselines/timescaleDB/python/tpch_lookup_throughput.txt', 'a') as f:
    print(sum([return_dict[worker]["throughput"] for worker in return_dict]), file=f)