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

# master = ("10.10.1.{}".format(int(sys.argv[1]) + 2), "9000")
# master = ("10.10.1.2", "9000")

num_workers = 20
total_points = int(30000) # 30M
warmup_points = int(total_points * 0.2)
processes = []
total_vect = []
num_data_nodes = 5

def insert_each_worker():

    client_list = []
    for node_id in range(num_data_nodes):
        client_list.append(Client("10.10.1.{}".format(12 + node_id), "9000"))

    effective_line_count = 0
    cumulative = 0
    warmup_ended = False

    for i in range(total_points):

        if i > warmup_points:
            if not warmup_ended:
                warmup_ended = True
            effective_line_count += 1

        hash_i = hash(i)
        try:
            start_time = time.time()
            client_list[hash_i % num_data_nodes].execute("INSERT INTO github_events (pkey, events_count, authors_count, forks, stars, issues, pushes, pulls, downloads, start_date, end_date) VALUES", [total_vect[i]])
            if warmup_ended:
                cumulative += time.time() - start_time

        except Exception as e:
            import sys
            print(len(total_vect), i)
            print("error: {0}".format(e), file=sys.stderr)
            exit(0)

        if i % 5000 == 0:
            print(i)

    return cumulative / effective_line_count

def lookup_each_worker():

    client_list = []
    for node_id in range(num_data_nodes):
        client_list.append(Client("10.10.1.{}".format(12 + node_id), "9000"))

    effective_line_count = 0
    cumulative = 0
    warmup_ended = False

    for i in range(total_points):

        if i > warmup_points:
            if not warmup_ended:
                warmup_ended = True
            effective_line_count += 1

        hash_i = hash(i)
        try:
            start_time = time.time()
            results = client_list[hash_i % num_data_nodes].execute("SELECT * FROM github_events WHERE pkey = {}".format(total_vect[i][0]))
            if warmup_ended:
                cumulative += time.time() - start_time

        except Exception as e:
            import sys
            print(len(total_vect), i)
            print("error: {0}".format(e), file=sys.stderr)
            exit(0)

        del results

        if i % 5000 == 0:
            print(i)

    return cumulative / effective_line_count

def load_all_points():
    file_path = "/mntData/github_split_10/x0"
    loaded_lines = 0
    with open(file_path) as f:
        for line in f:
            string_list = line.split(",")
            total_vect.append([int(entry) for entry in string_list])
            loaded_lines += 1
            if loaded_lines == total_points:
                break

load_all_points()
insertion_latency = insert_each_worker()
print("insertion latency: ", insertion_latency * 1000000, "us")
lookup_latency = lookup_each_worker()
print("lookup latency: ", lookup_latency * 1000000, "us")