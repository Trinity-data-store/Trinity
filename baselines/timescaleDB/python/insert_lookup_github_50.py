import psycopg2
from pgcopy import CopyManager
import pandas as pd 
from datetime import datetime
import time
import sys
import multiprocessing
from multiprocessing import Process
from concurrent.futures import ProcessPoolExecutor
from random import randrange
import random
import fcntl

COLS = ['id', 'quantity', 'extendedprice', 'discount', 'tax', 'shipdate', 'commitdate', 'receiptdate', 'totalprice', 'orderdate']

processes = []
total_vect = []
num_data_nodes = 5
skip_points = int(10000000 / 10) # 10M 
total_points = skip_points + int(skip_points / 10) # 11M

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

    connection_list = []
    cursor_insert_list = []
    cursor_lookup_list = []
    for data_node_idx in range(num_data_nodes):
        client_ip = "10.10.1.{}".format(12 + data_node_idx)
        CONNECTION = "dbname=defaultdb host={} user=postgres password=adifficultpassword sslmode=disable".format(client_ip)
        CONN = psycopg2.connect(CONNECTION)
        # CONN.autocommit = True
        connection_list.append(CONN)
        cursor_insert_list.append(CONN.cursor())
        cursor_lookup_list.append(CONN.cursor())

    line_count = 0
    start_time = time.time()
    insertion_count = 0
    lookup_count = 0
    primary_key_list = []
    started_measuring = False
    effective_line_count = 0
    inserted_pts = []

    with open(file_path) as f:
        for line in f:

            line_count += 1
            if line_count % total_workers != worker_idx:
                continue

            if line_count > total_points:
                break

            effective_line_count += 1
            
            if effective_line_count % 2 == 1:
                is_insert = True
            else:
                is_insert = False
 
            if is_insert or line_count < skip_points:

                '''
                Load points
                '''

                string_list = line.split(",")
                chunk = [int(entry) for entry in string_list]
                chunk[9] = datetime.strptime(str(chunk[9]), "%Y%m%d")
                chunk[10] = datetime.strptime(str(chunk[10]), "%Y%m%d")

                p_key = chunk[0]
                cursor_insert_list[hash(p_key) % 5].execute("INSERT INTO github_events (pkey, events_count, authors_count, forks, stars, issues, pushes, pulls, downloads, start_date, end_date) VALUES (%s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s);", chunk)
                connection_list[hash(p_key) % 5].commit() # Needed for atomic transaction.

                inserted_pts.append(p_key)
            else:
                primary_key = inserted_pts[zipf_keys[i - skip_points] % len(inserted_pts)]

                cursor_lookup_list[hash(primary_key) % 5].execute("SELECT * FROM github_events WHERE pkey = {}".format(primary_key))

                results = cursor_lookup_list[hash(primary_key) % 5].fetchone()
            
    end_time = time.time()

    return effective_line_count / (end_time - start_time)

def insert_lookup_worker(worker, total_workers, return_dict):

    throughput = insert_lookup_each_worker(worker, total_workers)
    return_dict[worker] = {"throughput": throughput}

threads_per_node = 60
total_num_nodes = 10

manager = multiprocessing.Manager()
return_dict = manager.dict()

if int(sys.argv[1]) == 0:
    with open('/proj/trinity-PG0/Trinity/baselines/timescaleDB/python/github_insert_lookup_throughput_50.txt', 'a') as f:
        fcntl.flock(f, fcntl.LOCK_EX)
        print("---- {}K ----".format(int(total_points / 1000)), file=f)
        fcntl.flock(f, fcntl.LOCK_UN)

for worker in range(threads_per_node):
    p = Process(target=insert_lookup_worker, args=(worker, threads_per_node, return_dict, ))
    p.start()
    processes.append(p)
    
for p in processes:
    p.join()

print("total throughput", sum([return_dict[worker]["throughput"] for worker in return_dict]))
with open('/proj/trinity-PG0/Trinity/baselines/timescaleDB/python/github_insert_lookup_throughput_50.txt', 'a') as f:
    fcntl.flock(f, fcntl.LOCK_EX)
    print(sum([return_dict[worker]["throughput"] for worker in return_dict]), file=f)
    fcntl.flock(f, fcntl.LOCK_UN)