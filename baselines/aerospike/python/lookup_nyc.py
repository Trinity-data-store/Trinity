import aerospike
import sys
import csv
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
from zipf_generator import ZipfGenerator

config = {
  'hosts': [ ('10.10.1.12', 3000), ('10.10.1.13', 3000), ('10.10.1.14', 3000), ('10.10.1.15', 3000), ('10.10.1.16', 3000)]
}

read_policies = {'total_timeout': 3000, 'max_retries': 1}
policies = {'read': read_policies}
config['policies'] = policies

header = ["pickup_date", "dropoff_date", "pickup_lon", "pickup_lat", "dropoff_lon", "dropoff_lat", "passenger_cnt", "trip_dist", "fare_amt", "extra", "mta_tax", "tip_amt", "tolls_amt", "impt_sur", "total_amt"]
processes = []
total_vect = []
num_data_nodes = 5
total_points = int(30000000) # 30M
warmup_points = int(total_points * 0.2)
# file_path = "/mntData/nyc_split_10/x{}".format(int(sys.argv[1]))


zipf_distribution = "/proj/trinity-PG0/Trinity/queries/zipf_keys_30m"
zipf_keys = []

def load_zipf():

    global zipf_keys
    with open(zipf_distribution) as file:
        lines = file.readlines()
        zipf_keys = [int(line) for line in lines]


def lookup_each_worker(total_num_workers, worker_idx):

    try:
        client = aerospike.client(config).connect()
    except:
        import sys
        print("failed to connect to the cluster with", config['hosts'])
        sys.exit(1)

    # line_count = 0
    start_time = time.time()
    effective_line_count = 0
    warmup_ended = False

    i = worker_idx
    with open(zipf_distribution) as f:
        for line in f:
            
            if i >= total_points:
                break

            zipf_key = int(line)
        # for i in range(worker_idx, total_points, total_num_workers):

            # line_count += 1

            # if line_count % total_num_workers != worker_idx:
            #     continue

            if i > warmup_points:
                if not warmup_ended:
                    start_time = time.time()
                    warmup_ended = True
                effective_line_count += 1

            '''
            Load points
            '''
            primary_key = total_vect[zipf_key]
            # primary_key = total_vect[i]
            # primary_key = total_vect[zipf_g.next()]

            '''
            Lookup
            '''            

            key_query = ('macro_bench', 'nyc_taxi_macro', primary_key)
            try:
                (key, meta, record) = client.get(key_query)
            except Exception as e:
                import sys
                print(key)
                print("error: {0}".format(e), file=sys.stderr)
                exit(0)

            del key
            del meta
            del record
            del key_query
            del primary_key

            i += total_num_workers

    end_time = time.time()
    return effective_line_count / (end_time - start_time)


def lookup_worker(worker, total_workers, return_dict):

    throughput = lookup_each_worker(total_workers, worker)
    return_dict[worker] = {"throughput": throughput}

threads_per_node = 60

manager = multiprocessing.Manager()
return_dict = manager.dict()


def load_all_points(client_idx):
    file_path = "/mntData/nyc_split_10/x{}".format(client_idx)
    loaded_lines = 0
    with open(file_path) as f:
        for line in f:

            string_list = line.split(",")
            total_vect.append(int(string_list[0]))
            loaded_lines += 1
            if loaded_lines == total_points:
                break

# load_zipf()
load_all_points(int(sys.argv[1]))

if int(sys.argv[1]) == 0:
    with open('/proj/trinity-PG0/Trinity/baselines/aerospike/python/nyc_lookup_throughput.txt', 'a') as f:
        fcntl.flock(f, fcntl.LOCK_EX)
        print("---- {}M ----".format(int(total_points / 1000000)), file=f)
        fcntl.flock(f, fcntl.LOCK_UN)

for worker in range(threads_per_node):
    p = Process(target=lookup_worker, args=(worker, threads_per_node, return_dict, ))
    p.start()
    processes.append(p)

for p in processes:
    p.join()

print("total throughput", sum([return_dict[worker]["throughput"] for worker in return_dict]))
with open('/proj/trinity-PG0/Trinity/baselines/aerospike/python/nyc_lookup_throughput.txt', 'a') as f:
    fcntl.flock(f, fcntl.LOCK_EX)
    print(sum([return_dict[worker]["throughput"] for worker in return_dict]), file=f)
    fcntl.flock(f, fcntl.LOCK_UN)
