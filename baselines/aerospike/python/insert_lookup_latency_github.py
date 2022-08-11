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
total_vect = []
num_data_nodes = 5
total_points = int(30000) 
warmup_points = int(total_points * 0.2)
file_path = "/mntData/github_split_10/x0"
insert_latency_vect = []
lookup_latency_vect = []

def flush_list_to_file(vect, file_name):
    with open(file_name, 'w') as f:
        for val in vect:
            f.write("{}\n".format(val))

def insert_each_worker():

    try:
        client = aerospike.client(config).connect()
    except:
        import sys
        print("failed to connect to the cluster with", config['hosts'])
        sys.exit(1)

    line_count = 0
    effective_line_count = 0
    cumulative = 0
    warmup_ended = False

    with open(file_path) as f:
        for line in f:

            line_count += 1

            '''
            Load points
            '''

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

            '''
            Insertion
            '''

            if line_count > warmup_points:
                if not warmup_ended:
                    warmup_ended = True
                effective_line_count += 1

            key = ('macro_bench', 'github_macro', primary_key)
            if rec:
                try:
                    start_time = time.time()
                    client.put(key, rec)
                    if warmup_ended:
                        latency = time.time() - start_time
                        cumulative += latency
                        insert_latency_vect.append(latency * 1000000)

                except Exception as e:
                    import sys
                    print(key, rec)
                    print("error: {0}".format(e), file=sys.stderr)
                    exit(-1)
            del key 
            del rec

            if line_count > total_points:
                break

    return cumulative / effective_line_count

def lookup_each_worker():

    try:
        client = aerospike.client(config).connect()
    except:
        import sys
        print("failed to connect to the cluster with", config['hosts'])
        sys.exit(1)

    line_count = 0
    effective_line_count = 0
    cumulative = 0
    warmup_ended = False

    with open(file_path) as f:
        for line in f:

            line_count += 1

            '''
            Load points
            '''

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


            if line_count > warmup_points:
                if not warmup_ended:
                    warmup_ended = True
                effective_line_count += 1


            '''
            Lookup
            '''     

            key = ('macro_bench', 'github_macro', primary_key)
            try:
                start_time = time.time()
                (_, _, _) = client.get(key)
                if warmup_ended:
                    latency =  time.time() - start_time
                    cumulative += latency
                    lookup_latency_vect.append(latency * 1000000)
            except Exception as e:
                import sys
                print(key)
                print("error: {0}".format(e), file=sys.stderr)
                exit(0)

            del key 

            if line_count > total_points:
                break

    return cumulative / effective_line_count

insertion_latency = insert_each_worker()
print("insertion latency: ", insertion_latency * 1000000, "us")
lookup_latency = lookup_each_worker()
print("lookup latency: ", lookup_latency * 1000000, "us")


flush_list_to_file(insert_latency_vect, "/proj/trinity-PG0/Trinity/results/latency_cdf/aerospike_github_insert")
flush_list_to_file(lookup_latency_vect, "/proj/trinity-PG0/Trinity/results/latency_cdf/aerospike_github_lookup")