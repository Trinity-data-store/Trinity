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

filename = "/proj/trinity-PG0/Trinity/queries/nyc/nyc_query_new"

with open(filename) as file:
    lines = file.readlines()
    lines = [line.rstrip() for line in lines]

client = Client(master[0], port=master[1])
total_points = 5000000
total_queries = 500 # should be 500
total_vect = []
processes = []

def load_all_points(client_idx):
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

load_all_points(int(sys.argv[1]))
client.execute("INSERT INTO nyc_taxi (pkey, pickup_date, dropoff_date, pickup_longitude, pickup_latitude, dropoff_longitude, dropoff_latitude, passenger_count, trip_distance, fare_amount, extra, mta_tax, tip_amount, tolls_amount, improvement_surcharge, total_amount) VALUES", total_vect)
threads_per_node = 5

def query_each_worker(worker_idx, total_workers):

    client = Client(master[0], port=master[1])
    effective_pts_count = 0
    start_time = time.time()

    for i in range(worker_idx, total_queries, total_workers):

        line = lines[i]
        query = line.split(",")[0] + ";"

        query_select = query.replace("count(*)", "*")

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
    with open('/proj/trinity-PG0/Trinity/baselines/clickhouse/python/nyc_search_throughput.txt', 'a') as f:
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

with open('/proj/trinity-PG0/Trinity/baselines/clickhouse/python/nyc_search_throughput.txt', 'a') as f:
    fcntl.flock(f, fcntl.LOCK_EX)
    print(sum([return_dict[worker]["throughput"] for worker in return_dict]), file=f)
    fcntl.flock(f, fcntl.LOCK_UN)


