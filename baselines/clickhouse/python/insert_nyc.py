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
import fcntl

num_workers = 20
total_points = int(100000) # 100k
warmup_points = int(total_points * 0.2)
processes = []
total_vect = []
num_data_nodes = 5

def insert_each_worker(worker_idx, total_workers):

    client_list = []
    for node_id in range(num_data_nodes):
        client_list.append(Client("10.10.1.{}".format(12 + node_id), "9000"))

    effective_line_count = 0
    start_time = time.time()
    warmup_ended = False

    for i in range(worker_idx, total_points, total_workers):

        if i > warmup_points:
            if not warmup_ended:
                start_time = time.time()
                warmup_ended = True
            effective_line_count += 1

        hash_i = hash(int(total_vect[i][0]))
        try:
            client_list[hash_i % num_data_nodes].execute("INSERT INTO nyc_taxi (pkey, pickup_date, dropoff_date, pickup_longitude, pickup_latitude, dropoff_longitude, dropoff_latitude, passenger_count, trip_distance, fare_amount, extra, mta_tax, tip_amount, tolls_amount, improvement_surcharge, total_amount) VALUES", [total_vect[i]])
        except Exception as e:
            import sys
            print(len(total_vect), i)
            print("error: {0}".format(e), file=sys.stderr)
            exit(0)

        if warmup_ended and effective_line_count % 5000 == 0:
            print(effective_line_count)

    end_time = time.time()
    return effective_line_count / (end_time - start_time)

def insert_worker(worker, total_workers, return_dict):

    throughput = insert_each_worker(worker, total_workers)
    return_dict[worker] = {"throughput": throughput}

threads_per_node = 40
total_num_nodes = 10

manager = multiprocessing.Manager()
return_dict = manager.dict()

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

if int(sys.argv[1]) == 0:
    with open('/proj/trinity-PG0/Trinity/baselines/clickhouse/python/nyc_insert_throughput.txt', 'a') as f:
        fcntl.flock(f, fcntl.LOCK_EX)
        print("---- {}K ----".format(int(total_points / 1000)), file=f)
        fcntl.flock(f, fcntl.LOCK_UN)

for worker in range(threads_per_node):
    p = Process(target=insert_worker, args=(worker, threads_per_node, return_dict, ))
    p.start()
    processes.append(p)

for p in processes:
    p.join()

print("total throughput", sum([return_dict[worker]["throughput"] for worker in return_dict]))

with open('/proj/trinity-PG0/Trinity/baselines/clickhouse/python/nyc_insert_throughput.txt', 'a') as f:
    fcntl.flock(f, fcntl.LOCK_EX)
    print(sum([return_dict[worker]["throughput"] for worker in return_dict]), file=f)
    fcntl.flock(f, fcntl.LOCK_UN)
