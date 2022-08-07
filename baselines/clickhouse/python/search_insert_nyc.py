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
total_points = int(5000000) # 5M
ops_points = int(500)
num_workers = 20
total_workers = 0

processes = []
total_vect = []
num_data_nodes = 5
no_insert = False

if len(sys.argv) == 3 and sys.argv[2] == "no_insert":
    no_insert = True
elif len(sys.argv) == 3:
    print(sys.argv)
    exit(0)

filename = "/proj/trinity-PG0/Trinity/queries/nyc/nyc_query_new"
master = ("10.10.1.2", "9000")
client = Client(master[0], port=master[1])

'''
Preload!
'''

def load_all_points_pre(client_idx):
    file_path = "/mntData/nyc_split_10/x{}".format(client_idx)
    loaded_lines = 0
    with open(file_path) as f:
        for line in f:
            string_list = line.split(",")
            chunk = []

            for i, entry in enumerate(string_list):
                if i >= 3 and i <= 6:
                    chunk.append(float(entry))
                else:
                    chunk.append(int(entry))
            total_vect.append(chunk)
            loaded_lines += 1

            if loaded_lines == total_points:
                break

load_all_points_pre(int(sys.argv[1]))
client.execute("INSERT INTO nyc_taxi (pkey, pickup_date, dropoff_date, pickup_longitude, pickup_latitude, dropoff_longitude, dropoff_latitude, passenger_count, trip_distance, fare_amount, extra, mta_tax, tip_amount, tolls_amount, improvement_surcharge, total_amount) VALUES", total_vect)

total_vect = []


with open(filename) as file:
    lines = file.readlines()
    lines = [line.rstrip() for line in lines]

def run_query(line_idx, client):


    line = lines[line_idx]
    query = line.split(",")[0] + ";"

    query_select = query.replace("count(*)", "*")

    results = client.execute(query_select)

    return len(results)

def search_insert_each_worker(worker_idx, total_workers):

    client = Client(master[0], port=master[1])

    client_list = []
    for node_id in range(num_data_nodes):
        client_list.append(Client("10.10.1.{}".format(12 + node_id), "9000"))

    line_count = 0
    insertion_count = 0
    lookup_count = 0
    primary_key_list = []
    start = time.time()
    has_started = False
    effective_count = 0
    for i in range(worker_idx, ops_points, total_workers):

        line_count += 1
        effective_count += 1

        if not no_insert:

            if line_count % 20 == 19:
                is_insert = True
            else:
                is_insert = False

        else:
            is_insert = False
            if line_count == 5:
                break

        if is_insert:
            insert_list = [total_vect[i]]
            p_key = insert_list[0][0]
            hash_i = hash(p_key)
            client_list[hash_i % num_data_nodes].execute("INSERT INTO nyc_taxi (pkey, pickup_date, dropoff_date, pickup_longitude, pickup_latitude, dropoff_longitude, dropoff_latitude, passenger_count, trip_distance, fare_amount, extra, mta_tax, tip_amount, tolls_amount, improvement_surcharge, total_amount) VALUES", insert_list)
            primary_key_list.append(p_key)
        else:
            num_returned_points = run_query(i % 1000, client)
            effective_count += num_returned_points - 1
            effective_count += num_returned_points - 1
            print(i, num_returned_points)
        if no_insert:
            print(line_count, i, num_returned_points)

    end_to_end_time = time.time() - start

    return effective_count / end_to_end_time


def search_insert_worker(worker, total_workers, return_dict):

    throughput = search_insert_each_worker(worker, total_workers)
    per_worker_return_dict = {}
    per_worker_return_dict["throughput"] = throughput
    return_dict[worker] = per_worker_return_dict

threads_per_node = 20
total_num_nodes = 10

manager = multiprocessing.Manager()
return_dict = manager.dict()

def load_all_points(client_idx):
    file_path = "/mntData/nyc_split_10/x{}".format(client_idx)
    loaded_lines = 0
    with open(file_path) as f:
        for line in f:
            loaded_lines += 1
            if loaded_lines % 1000000 == 0:
                print(loaded_lines)

            if loaded_lines < total_points:
                continue

            string_list = line.split(",")
            chunk = []
            for i, entry in enumerate(string_list):
                if i >= 3 and i <= 6:
                    chunk.append(float(entry))
                else:
                    chunk.append(int(entry))

            total_vect.append(chunk)
            loaded_lines += 1
            if loaded_lines > total_points + ops_points:
                break

if not no_insert:
    load_all_points(int(sys.argv[1]))

for worker in range(threads_per_node):
    p = Process(target=search_insert_worker, args=(worker, threads_per_node, return_dict, ))
    p.start()
    processes.append(p)

for p in processes:
    p.join()


print("total throughput", sum([return_dict[worker]["throughput"] for worker in return_dict]), flush=True)

if not no_insert:
    with open('/proj/trinity-PG0/Trinity/baselines/clickhouse/python/nyc_search_insert_throughput.txt', 'a') as f:
        print(sum([return_dict[worker]["throughput"] for worker in return_dict]), file=f, flush=True)
if no_insert:
    with open('/proj/trinity-PG0/Trinity/baselines/clickhouse/python/nyc_search_throughput.txt', 'a') as f:
        print(sum([return_dict[worker]["throughput"] for worker in return_dict]), file=f, flush=True) 
