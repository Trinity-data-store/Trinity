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
import fcntl

processes = []
total_vect = []
num_data_nodes = 5
total_points = int(5000000) # 2M # 5 min
warmup_points = int(total_points * 0.2)
file_path = "/mntData/nyc_split_10/x{}".format(int(sys.argv[1]))

def insert_each_worker(worker_idx, total_workers):

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
            if line_count % total_workers != worker_idx:
                continue

            '''
            Load points
            '''

            string_list = line.split(",")
            chunk = []
            for i, entry in enumerate(string_list):
                if i >= 3 and i <= 6:
                    chunk.append(float(entry))
                else:
                    chunk.append(int(entry))

            chunk[1] = datetime.strptime(str(chunk[1]), "%Y%m%d")
            chunk[2] = datetime.strptime(str(chunk[2]), "%Y%m%d")

            '''
            Insertion
            '''

            if line_count > warmup_points:
                if not warmup_ended:
                    start_time = time.time()
                    warmup_ended = True
                effective_line_count += 1

            hash_i = hash(int(chunk[0]))
            cursor_list[hash_i % 5].execute("INSERT INTO nyc_taxi (pkey, pickup_date, dropoff_date, pickup_lon, pickup_lat, dropoff_lon, dropoff_lat, passenger_cnt, trip_dist, fare_amt, extra, mta_tax, tip_amt, tolls_amt, impt_sur, total_amt) VALUES (%s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s);", chunk)
            connection_list[hash_i % 5].commit() # Needed for atomic transaction.

            '''
            Break
            '''

            if line_count > total_points:
                break

            if warmup_ended and effective_line_count % 1000 == 0:
                print(effective_line_count)

    end_time = time.time()

    return effective_line_count / (end_time - start_time)

def insert_worker(worker, total_workers, return_dict):

    throughput = insert_each_worker(worker, total_workers)
    return_dict[worker] = {"throughput": throughput}

threads_per_node = 20

manager = multiprocessing.Manager()
return_dict = manager.dict()

'''
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
'''

for worker in range(threads_per_node):
    p = Process(target=insert_worker, args=(worker, threads_per_node, return_dict, ))
    p.start()
    processes.append(p)

for p in processes:
    p.join()
