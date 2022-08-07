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
import re
from aerospike import exception as ex
from aerospike_helpers import expressions as exp
from aerospike import predicates
import fcntl

config = {
  'hosts': [ ('10.10.1.12', 3000), ('10.10.1.13', 3000), ('10.10.1.14', 3000), ('10.10.1.15', 3000), ('10.10.1.16', 3000)]
}

header = ["pickup_date", "dropoff_date", "pickup_lon", "pickup_lat", "dropoff_lon", "dropoff_lat", "passenger_cnt", "trip_dist", "fare_amt", "extra", "mta_tax", "tip_amt", "tolls_amt", "impt_sur", "total_amt"]

processes = []
total_vect = []
num_data_nodes = 5
skip_points = int(5000000) # 5M # Need to run aerospike_insert first!!
total_points = int(5000500) # 10M
no_insert = False

if len(sys.argv) == 3 and sys.argv[2] == "no_insert":
    no_insert = True
elif len(sys.argv) == 3:
    print(sys.argv)
    exit(0)

filename = "/proj/trinity-PG0/Trinity/queries/nyc/nyc_query_new"

with open(filename) as file:
    lines = file.readlines()
    lines = [line.rstrip() for line in lines]


def perform_search(line_idx, client):

    trip_distance_range = [0, 198623000]
    fare_amount_range = [0, 21474808]
    tip_amount_range = [0, 3950588]

    pickup_date_range = [20090101, 20160618]
    dropoff_date_range = [19700101, 20161103]
    passenger_count_range = [0, 255]
    pickup_longitude_range = [0, 90]
    pickup_latitude_range = [0, 90]

    regex_template_1 = re.compile(r"where trip_distance <= (?P<trip_distance_end>[0-9.]+?) and fare_amount between (?P<fare_amount_start>[0-9.]+?) AND (?P<fare_amount_end>[0-9.]+?);")
    regex_template_2 = re.compile(r"where trip_distance >= (?P<trip_distance_start>[0-9.]+?) and trip_distance <= (?P<trip_distance_end>[0-9.]+?) and fare_amount <= (?P<fare_amount_end>[0-9.]+?) and tip_amount <= (?P<tip_amount_end>[0-9.]+?);")
    regex_template_3 = re.compile(r"where pickup_date >= (?P<pickup_date_start>[0-9.]+?) and pickup_date <= (?P<pickup_date_end>[0-9.]+?) and dropoff_date >= (?P<dropoff_date_start>[0-9.]+?) and dropoff_date <= (?P<dropoff_date_end>[0-9.]+?);")
    regex_template_4 = re.compile(r"where pickup_date <= (?P<pickup_date_end>[0-9.]+?) and passenger_count = (?P<passenger_count>[0-9.]+?);")
    regex_template_5 = re.compile(r"where pickup_longitude BETWEEN (?P<pickup_longitude_start>[0-9.]+?) and (?P<pickup_longitude_end>[0-9.]+?) AND pickup_latitude BETWEEN (?P<pickup_latitude_start>[0-9.]+?) AND (?P<pickup_latitude_end>[0-9.]+?);")

    line = lines[line_idx]

    line = line.split(",")[0] + ";"
    line = line.replace("count(*)", "*")

    query = client.query('macro_bench', 'nyc_taxi_macro')
    query.select("pickup_date", "dropoff_date", "pickup_lon", "pickup_lat", "dropoff_lon", "dropoff_lat", "passenger_cnt", "trip_dist", "fare_amt", "extra", "mta_tax", "tip_amt", "tolls_amt", "impt_sur", "total_amt")
    picked_template = 1
    for reg in [regex_template_1, regex_template_2, regex_template_3, regex_template_4, regex_template_5]:
        m = reg.search(line)
        if m:
            break
        picked_template += 1
    if not m:
        print("error!", line)
        exit(0)

    attribute_to_selectivity = {}

    # Picking other fields are always the best
    
    if picked_template == 1:
        attribute_to_selectivity["trip_distance"] = (int(m.group("trip_distance_end")) - trip_distance_range[0]) / (trip_distance_range[1] - trip_distance_range[0])
        attribute_to_selectivity["fare_amount"] = (int(m.group("fare_amount_end")) - int(m.group("fare_amount_start"))) / (fare_amount_range[1] - fare_amount_range[0])

    if picked_template == 2:
        attribute_to_selectivity["trip_distance"] = (int(m.group("trip_distance_end")) - int(m.group("trip_distance_start"))) / (trip_distance_range[1] - trip_distance_range[0])
        attribute_to_selectivity["fare_amount"] = (int(m.group("fare_amount_end")) - fare_amount_range[0]) / (fare_amount_range[1] - fare_amount_range[0])
        attribute_to_selectivity["tip_amount"] = (int(m.group("tip_amount_end")) - tip_amount_range[0]) / (tip_amount_range[1] - tip_amount_range[0])

    if picked_template == 3:
        attribute_to_selectivity["pickup_date"] = (int(m.group("pickup_date_end")) - int(m.group("pickup_date_start"))) / (pickup_date_range[1] - pickup_date_range[0])
        attribute_to_selectivity["dropoff_date"] = (int(m.group("dropoff_date_end")) - int(m.group("dropoff_date_start"))) / (dropoff_date_range[1] - dropoff_date_range[0])

    if picked_template == 4:
        attribute_to_selectivity["pickup_date"] = (int(m.group("pickup_date_end")) - pickup_date_range[0]) / (pickup_date_range[1] - pickup_date_range[0])
        attribute_to_selectivity["passenger_count"] = 0

    if picked_template == 5:
        attribute_to_selectivity["pickup_longitude"] = (int(m.group("pickup_longitude_end")) - int(m.group("pickup_longitude_start"))) / (pickup_longitude_range[1] - pickup_longitude_range[0])
        attribute_to_selectivity["pickup_latitude"] = (int(m.group("pickup_latitude_end")) - int(m.group("pickup_latitude_start"))) / (pickup_latitude_range[1] - pickup_latitude_range[0])

    picked_query = min(attribute_to_selectivity, key=attribute_to_selectivity.get)
    # print(line, picked_query, attribute_to_selectivity[picked_query])

    if picked_template == 1:

        '''
        regex_template_1 = re.compile(r"where trip_distance <= (?P<trip_distance_end>[0-9.]+?) and fare_amount between (?P<fare_amount_start>[0-9.]+?) AND (?P<fare_amount_end>[0-9.]+?);")
        '''

        if picked_query == "trip_distance":
            query.where(predicates.between('trip_dist', int(trip_distance_range[0]), int(m.group("trip_distance_end"))))
            expr = exp.And(
                exp.GE(exp.IntBin("fare_amt"), int(m.group("fare_amount_start"))),
                exp.LE(exp.IntBin("fare_amt"), int(m.group("fare_amount_end"))),
            ).compile()

        if picked_query == "fare_amount":
            query.where(predicates.between('fare_amt', int(m.group("fare_amount_start")), int(m.group("fare_amount_end"))))
            expr = exp.LE(exp.IntBin("trip_dist"), int(m.group("trip_distance_end"))).compile()

    if picked_template == 2:

        '''
        regex_template_2 = re.compile(r"where trip_distance >= (?P<trip_distance_start>[0-9.]+?) and trip_distance <= (?P<trip_distance_end>[0-9.]+?) and fare_amount <= (?P<fare_amount_end>[0-9.]+?) and tip_amount <= (?P<tip_amount_end>[0-9.]+?);")
        '''

        if picked_query == "trip_distance":
            query.where(predicates.between('trip_dist', int(m.group("trip_distance_start")), int(m.group("trip_distance_end"))))
            expr = exp.And(
                exp.LE(exp.IntBin("fare_amt"), int(m.group("fare_amount_end"))),
                exp.LE(exp.IntBin("tip_amt"), int(m.group("tip_amount_end")))
            ).compile()

        if picked_query == "fare_amount":
            query.where(predicates.between('fare_amt', int(fare_amount_range[0]), int(m.group("fare_amount_end"))))
            expr = exp.And(
                exp.GE(exp.IntBin("trip_dist"), int(m.group("trip_distance_start"))),
                exp.LE(exp.IntBin("trip_dist"), int(m.group("trip_distance_end"))),
                exp.LE(exp.IntBin("tip_amt"), int(m.group("tip_amount_end"))),
            ).compile()

        if picked_query == "tip_amount":
            query.where(predicates.between('tip_amt', int(tip_amount_range[0]), int(m.group("tip_amount_end"))))
            expr = exp.And(
                exp.GE(exp.IntBin("trip_dist"), int(m.group("trip_distance_start"))),
                exp.LE(exp.IntBin("trip_dist"), int(m.group("trip_distance_end"))),
                exp.LE(exp.IntBin("fare_amt"), int(m.group("fare_amount_end"))),
            ).compile()

    if picked_template == 3:

        '''
        regex_template_3 = re.compile(r"where pickup_date >= (?P<pickup_date_start>[0-9.]+?) and pickup_date <= (?P<pickup_date_end>[0-9.]+?) and dropoff_date >= (?P<dropoff_date_start>[0-9.]+?) and dropoff_date <= (?P<dropoff_date_end>[0-9.]+?);")
        '''

        if picked_query == "pickup_date":
            query.where(predicates.between('pickup_date', int(m.group("pickup_date_start")), int(m.group("pickup_date_end"))))
            expr = exp.And(
                exp.GE(exp.IntBin("dropoff_date"), int(m.group("dropoff_date_start"))),
                exp.LE(exp.IntBin("dropoff_date"), int(m.group("dropoff_date_end"))),
            ).compile()

        if picked_query == "dropoff_date":
            query.where(predicates.between('dropoff_date', int(m.group("dropoff_date_start")), int(m.group("dropoff_date_end"))))
            expr = exp.And(
                exp.GE(exp.IntBin("pickup_date"), int(m.group("pickup_date_start"))),
                exp.LE(exp.IntBin("pickup_date"), int(m.group("pickup_date_end"))),
            ).compile()

    if picked_template == 4:

        '''
        regex_template_4 = re.compile(r"where pickup_date <= (?P<pickup_date_end>[0-9.]+?) and passenger_count = (?P<passenger_count>[0-9.]+?);")
        '''

        query.where(predicates.between('passenger_cnt', int(m.group("passenger_count")), int(m.group("passenger_count"))))
        expr = exp.LE(exp.IntBin("pickup_date"), int(m.group("pickup_date_end"))).compile()


    if picked_template == 5:

        '''
        regex_template_5 = re.compile(r"where pickup_longitude BETWEEN (?P<pickup_longitude_start>[0-9.]+?) and (?P<pickup_longitude_end>[0-9.]+?) AND pickup_latitude BETWEEN (?P<pickup_latitude_start>[0-9.]+?) AND (?P<pickup_latitude_end>[0-9.]+?);")
        '''
        # print("template 5: ", float(m.group("pickup_longitude_start")), " ", float(m.group("pickup_longitude_end")), float(m.group("pickup_latitude_start")), float(m.group("pickup_latitude_end")))
        if picked_query == "pickup_longitude":
            # query.where(predicates.between('pickup_lon', int(m.group("pickup_longitude_start")), int(m.group("pickup_longitude_end"))))
            expr = exp.And(
                exp.GE(exp.FloatBin("pickup_lon"), float(m.group("pickup_longitude_start"))),
                exp.LE(exp.FloatBin("pickup_lon"), float(m.group("pickup_longitude_end"))),
                exp.GE(exp.FloatBin("pickup_lat"), float(m.group("pickup_latitude_start"))),
                exp.LE(exp.FloatBin("pickup_lat"), float(m.group("pickup_latitude_end"))),
            ).compile()

        if picked_query == "pickup_latitude":
            # query.where(predicates.between('pickup_lat', int(m.group("pickup_latitude_start")), int(m.group("pickup_latitude_end"))))
            expr = exp.And(
                exp.GE(exp.FloatBin("pickup_lon"), float(m.group("pickup_longitude_start"))),
                exp.LE(exp.FloatBin("pickup_lon"), float(m.group("pickup_longitude_end"))),
                exp.GE(exp.FloatBin("pickup_lat"), float(m.group("pickup_latitude_start"))),
                exp.LE(exp.FloatBin("pickup_lat"), float(m.group("pickup_latitude_end"))),
            ).compile()


    policy = {
        'expressions': expr,
        'total_timeout':200000,
        'socket_timeout': 200000
    }

    records_query = []

    try:
        records_query = query.results(policy)
        
    except Exception as e:
        print(e)

    return len(records_query)

def search_insert_each_worker(total_num_workers, worker_idx):

    try:
        client = aerospike.client(config).connect()
    except:
        import sys
        print("failed to connect to the cluster with", config['hosts'])
        sys.exit(1)

    line_count = 0
    effective_line_count = 0
    start_time = time.time()
    end_time = 0
    insertion_count = 0

    for i in range(skip_points + worker_idx - skip_points, total_points - skip_points, total_num_workers):

        line_count += 1
        effective_line_count += 1

        if line_count % 20 == 19 and not no_insert:
            is_search = 0
        else:
            is_search = 1
            
        if is_search:
            return_num_points = perform_search(i % 1000, client)
            effective_line_count += return_num_points - 1
        else:
            primary_key, rec = total_vect[i]
            key = ('macro_bench', 'nyc_taxi_macro', primary_key)
            if rec:
                try:
                    client.put(key, rec)
                except Exception as e:
                    import sys
                    print(key, rec)
                    print("error: {0}".format(e), file=sys.stderr)
            del key 
            del rec

        print(line_count, effective_line_count)

    end_time = time.time()

    return effective_line_count / (end_time - start_time)


def search_insert_worker(worker, total_workers, return_dict):

    throughput = search_insert_each_worker(total_workers, worker)
    return_dict[worker] = {"throughput": throughput}

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
            if loaded_lines < skip_points:
                continue

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

            total_vect.append((primary_key, rec))

            if loaded_lines == total_points:
                break

if not no_insert:
    load_all_points(int(sys.argv[1]))

if int(sys.argv[1]) == 0:
    with open('/proj/trinity-PG0/Trinity/baselines/aerospike/python/nyc_search_insert_throughput.txt', 'a') as f:
        fcntl.flock(f, fcntl.LOCK_EX)
        print("---- {}K ----".format(int(total_points / 1000)), file=f)
        fcntl.flock(f, fcntl.LOCK_UN)

for worker in range(threads_per_node):
    p = Process(target=search_insert_worker, args=(worker, threads_per_node, return_dict, ))
    p.start()
    processes.append(p)

for p in processes:
    p.join()

print("total throughput", sum([return_dict[worker]["throughput"] for worker in return_dict]))

with open('/proj/trinity-PG0/Trinity/baselines/aerospike/python/nyc_search_insert_throughput.txt', 'a') as f:
    fcntl.flock(f, fcntl.LOCK_EX)
    print(sum([return_dict[worker]["throughput"] for worker in return_dict]), file=f)
    fcntl.flock(f, fcntl.LOCK_UN)