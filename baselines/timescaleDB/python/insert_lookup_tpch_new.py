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

COLS = ['id', 'quantity', 'extendedprice', 'discount', 'tax', 'shipdate', 'commitdate', 'receiptdate', 'totalprice', 'orderdate']

processes = []
total_vect = []
num_data_nodes = 5
begin_measuring = int(5000000) # 5M
stop_measuring = int(10000000) # 10M
total_points = int(10000000) # 10M

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

    for i in range(worker_idx, total_points, total_workers):

        if not started_measuring and i > begin_measuring and i <= stop_measuring:
            started_measuring = True
            start_time = time.time()
        
        if started_measuring and i > stop_measuring:
            started_measuring = False
            end_time = time.time()

        if started_measuring:
            effective_line_count += 1
        
        line_count += 1

        if len(primary_key_list) < 1000 or line_count % 20 == 19:
            is_insert = True
        else:
            is_insert = False

        if is_insert:
            chunk = total_vect[i]
            p_key = chunk[0]
            cursor_insert_list[hash(p_key) % 5].execute("INSERT INTO tpch_macro (ID, QUANTITY, EXTENDEDPRICE, DISCOUNT, TAX, SHIPDATE, COMMITDATE, RECEIPTDATE, TOTALPRICE, ORDERDATE) VALUES (%s, %s, %s, %s, %s, %s, %s, %s, %s, %s);", chunk)
            connection_list[hash(p_key) % 5].commit() # Needed for atomic transaction.
            primary_key_list.append(p_key)
            insertion_count += 1
        else:
            primary_key_to_query = random.choice(primary_key_list)
            cursor_lookup_list[hash(primary_key_to_query) % 5].execute("SELECT * FROM tpch_macro WHERE ID = {}".format(primary_key_to_query))
            results = cursor_lookup_list[hash(primary_key_to_query) % 5].fetchone()
            lookup_count += 1

        if line_count and line_count % (total_points / 10) == 0:
            print(line_count, flush=True)
            
    # end_time = time.time()

    return effective_line_count / (end_time - start_time)

def insert_lookup_worker(worker, total_workers, return_dict):

    throughput = insert_lookup_each_worker(worker, total_workers)
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
            chunk = [int(entry) for entry in string_list]

            chunk[5] = datetime.strptime(str(chunk[5]), "%Y%m%d")
            chunk[6] = datetime.strptime(str(chunk[6]), "%Y%m%d")
            chunk[7] = datetime.strptime(str(chunk[7]), "%Y%m%d")
            chunk[9] = datetime.strptime(str(chunk[9]), "%Y%m%d")

            total_vect.append(chunk)
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
with open('/proj/trinity-PG0/Trinity/baselines/timescaleDB/python/tpch_insert_lookup_throughput.txt', 'a') as f:
    print(sum([return_dict[worker]["throughput"] for worker in return_dict]), file=f)