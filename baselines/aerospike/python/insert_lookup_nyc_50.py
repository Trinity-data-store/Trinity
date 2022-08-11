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

config = {
  'hosts': [ ('10.10.1.12', 3000), ('10.10.1.13', 3000), ('10.10.1.14', 3000), ('10.10.1.15', 3000), ('10.10.1.16', 3000)]
}

header = ["pickup_date", "dropoff_date", "pickup_lon", "pickup_lat", "dropoff_lon", "dropoff_lat", "passenger_cnt", "trip_dist", "fare_amt", "extra", "mta_tax", "tip_amt", "tolls_amt", "impt_sur", "total_amt"]
processes = []
total_vect = []
num_data_nodes = 5
skip_points = int(10000000) # 10M # Need to run aerospike_insert first!!
total_points = skip_points + int(skip_points / 10) # 11M

zipf_keys = []
zipf_distribution = "/proj/trinity-PG0/Trinity/queries/zipf_keys_30m"
file_path = "/mntData/nyc_split_10/x{}".format(int(sys.argv[1]))

with open(zipf_distribution) as f:
    i = 0
    for line in f:
        i += 1
        zipf_keys.append(int(line) % skip_points)
        if i >= total_points:
            break

def insert_lookup_each_worker(total_num_workers, worker_idx):

    try:
        client_insert = aerospike.client(config).connect()
        client_lookup = aerospike.client(config).connect()
    except:
        import sys
        print("failed to connect to the cluster with", config['hosts'])
        sys.exit(1)

    effective_line_count = 0
    started_measuring = False
    start_time = time.time()
    inserted_pts = []

    i = worker_idx
    with open(file_path) as f:
        for line in f:
            
            if i > total_points:
                break

            i += total_num_workers


            effective_line_count += 1
            
            if effective_line_count % 2 == 1:
                is_insert = True
            else:
                is_insert = False

            if is_insert or i < skip_points:
                # primary_key, rec = total_vect[i]
                string_list = line.split(",")
                column = 0
                rec = {}
                primary_key = 0
                for col in string_list:
                    if column == 0:
                        primary_key = int(col)
                    elif column >= 3 and column <= 6:
                        rec[header[column - 1]] = float(col)
                    else:
                        rec[header[column - 1]] = int(col)
                    column += 1

                key = ('macro_bench', 'nyc_taxi_macro', primary_key)
                if rec:
                    try:
                        client_insert.put(key, rec)
                    except Exception as e:
                        import sys
                        print(key, rec)
                        print("error: {0}".format(e), file=sys.stderr)
                        exit(0)
                del key 
                del rec
                inserted_pts.append(primary_key)

            else:
                # primary_key_to_query = random.choice(primary_key_list)
                primary_key = inserted_pts[zipf_keys[i - skip_points] % len(inserted_pts)]
                
                key_query = ('macro_bench', 'nyc_taxi_macro', primary_key)
                try:
                    (_, _, _) = client_lookup.get(key_query)
                except Exception as e:
                    import sys
                    print(key_query)
                    print("error: {0}".format(e), file=sys.stderr)
                    exit(0)

    end_time = time.time()

    return effective_line_count / (end_time - start_time)


def insert_lookup_worker(worker, total_workers, return_dict):

    throughput = insert_lookup_each_worker(total_workers, worker)
    return_dict[worker] = {"throughput": throughput}

threads_per_node = 60
total_num_nodes = 10

manager = multiprocessing.Manager()
return_dict = manager.dict()

'''
def load_all_points(client_idx):
    file_path = "/mntData/tpch_split_10/x{}".format(client_idx)
    loaded_lines = 0
    with open(file_path) as f:
        for line in f:

            string_list = line.split(",")

            column = 0
            rec = {}
            primary_key = 0
            for col in string_list:
                if column == 0:
                    primary_key = int(col)
                else:
                    rec[header[column - 1]] = int(col)
                column += 1

            total_vect.append((primary_key, rec))
            loaded_lines += 1
            if loaded_lines == total_points:
                break
'''

if int(sys.argv[1]) == 0:
    with open('/proj/trinity-PG0/Trinity/baselines/aerospike/python/nyc_insert_lookup_throughput_50.txt', 'a') as f:
        fcntl.flock(f, fcntl.LOCK_EX)
        print("---- {}K ----".format(int(total_points / 1000)), file=f)
        fcntl.flock(f, fcntl.LOCK_UN)

# load_all_points(int(sys.argv[1]))

for worker in range(threads_per_node):
    p = Process(target=insert_lookup_worker, args=(worker, threads_per_node, return_dict, ))
    p.start()
    processes.append(p)

for p in processes:
    p.join()

print("total throughput", sum([return_dict[worker]["throughput"] for worker in return_dict]))

with open('/proj/trinity-PG0/Trinity/baselines/aerospike/python/nyc_insert_lookup_throughput_50.txt', 'a') as f:
    fcntl.flock(f, fcntl.LOCK_EX)
    print(sum([return_dict[worker]["throughput"] for worker in return_dict]), file=f)
    fcntl.flock(f, fcntl.LOCK_UN)