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

write_policies = {'total_timeout': 4000, 'max_retries': 0}
read_policies = {'total_timeout': 3000, 'max_retries': 1}
policies = {'write': write_policies, 'read': read_policies}
config['policies'] = policies

header = ["events_count", "authors_count", "forks", "stars", "issues", "pushes", "pulls", "downloads", "start_date", "end_date"]
processes = []
threads = []
total_vect = []
num_data_nodes = 5
total_points = int(50000000) # 30M
warmup_points = int(total_points * 0.2)
file_path = "/mntData/github_split_10/x{}".format(int(sys.argv[1]))

def insert_each_worker(total_num_workers, worker_idx):

    try:
        client = aerospike.client(config).connect()
    except:
        import sys
        print("failed to connect to the cluster with", config['hosts'])
        sys.exit(1)

    line_count = 0
    effective_line_count = 0
    start_time = time.time()
    warmup_ended = False

    with open(file_path) as f:
        for line in f:

            line_count += 1
            if line_count % total_num_workers != worker_idx:
                continue

            '''
            Load points
            '''

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

            '''
            Insertion
            '''

            if line_count > warmup_points:
                if not warmup_ended:
                    start_time = time.time()
                    warmup_ended = True
                effective_line_count += 1

            key = ('macro_bench', 'github_macro', primary_key)
            if rec:
                try:
                    client.put(key, rec)
                except Exception as e:
                    import sys
                    print(key, rec)
                    print("error: {0}".format(e), file=sys.stderr)
                    exit(0)

            del key 
            del rec

            if line_count > total_points:
                break

    end_time = time.time()
    return effective_line_count / (end_time - start_time)


def insert_worker(worker, total_workers, return_dict):

    throughput = insert_each_worker(total_workers, worker)
    return_dict[worker] = {"throughput": throughput}


'''
def load_all_points(client_idx):
    file_path = "/mntData/tpch_split_10/x{}".format(client_idx)
    loaded_lines = 0
    with open(file_path) as f:
        for line in f:

            string_list = line.split(",")
            string_list = string_list[:9] + string_list[12:]

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

load_all_points(int(sys.argv[1]))
'''

# threads_per_node = 60 # Some client timeout error
threads_per_node = 40

manager = multiprocessing.Manager()
return_dict = manager.dict()


for worker in range(threads_per_node):
    p = Process(target=insert_worker, args=(worker, threads_per_node, return_dict, ))
    p.start()
    processes.append(p)
    
for p in processes:
    p.join()
