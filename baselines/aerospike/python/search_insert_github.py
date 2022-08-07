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

header = ["events_count", "authors_count", "forks", "stars", "issues", "pushes", "pulls", "downloads", "start_date", "end_date"]
processes = []
total_vect = []
num_data_nodes = 5
skip_points = int(50000000) # 5M # Need to run aerospike_insert first!!
total_points = int(50000100) # 10M
no_insert = False

if len(sys.argv) == 3 and sys.argv[2] == "no_insert":
    no_insert = True
elif len(sys.argv) == 3:
    print(sys.argv)
    exit(0)

filename = "/proj/trinity-PG0/Trinity/queries/github/github_query_final"

with open(filename) as file:
    lines = file.readlines()
    lines = [line.rstrip() for line in lines]


def perform_search(line_idx, client):


    stars_range = [0, 354850]  # min, max
    forks_range = [0, 262926]  # min, max
    issues_range = [0, 379379]  # min, max
    events_count_range = [1, 7451541]  # min, max
    start_date_range = [20110211, 20201206]  # min, max
    end_date_range = [20110211, 20201206]  # min, max

    regex_template_1 = re.compile(r"where start_date <= (?P<start_date_end>[0-9.]+?) AND end_date >= (?P<end_date_start>[0-9.]+?) AND stars >= (?P<stars_start>[0-9.]+?),")
    regex_template_2 = re.compile(r"where stars >= (?P<stars_start>[0-9.]+?) and forks >= (?P<forks_start>[0-9.]+?),")
    regex_template_3 = re.compile(r"where events_count <= (?P<events_count_end>[0-9.]+?) and issues >= (?P<issues_start>[0-9.]+?) AND stars >= (?P<stars_start>[0-9.]+?),")
    regex_template_4 = re.compile(r"where start_date >= (?P<start_date_start>[0-9.]+?) AND start_date <= (?P<start_date_end>[0-9.]+?) AND end_date >= (?P<end_date_start>[0-9.]+?) AND end_date <= (?P<end_date_end>[0-9.]+?),")

    line = lines[line_idx]

    line = line.split(";,")[0] + ";"
    line = line.replace("COUNT(*)", "*")

    query = client.query('macro_bench', 'github_macro')
    query.select("events_count", "authors_count", "forks", "stars", "issues", "pushes", "pulls", "downloads", "start_date", "end_date")

    picked_template = 1
    for reg in [regex_template_1, regex_template_2, regex_template_3, regex_template_4]:
        m = reg.search(line)
        if m:
            break
        picked_template += 1
    if not m:
        print("error!", line)
        exit(0)

    attribute_to_selectivity = {}

    # Picking other fields are always the best
    '''
    if (picked_template == 1) and int(m.group("start_date_end")) != start_date_range[1]:
        attribute_to_selectivity["start_date"] = (int(m.group("start_date_end")) - start_date_range[0]) / (start_date_range[1] - start_date_range[0])

    if (picked_template == 1) and int(m.group("end_date_start")) != end_date_range[0]:
        attribute_to_selectivity["end_date"] = (end_date_range[1] - int(m.group("end_date_start"))) / (end_date_range[1] - end_date_range[0])
    '''

    if (picked_template == 1 or picked_template == 2 or picked_template == 3) and int(m.group("stars_start")) != stars_range[0]:
        attribute_to_selectivity["stars"] = (stars_range[1] - int(m.group("stars_start"))) / (stars_range[1] - stars_range[0])

    if (picked_template == 2) and int(m.group("forks_start")) != forks_range[0]:
        attribute_to_selectivity["forks"] = (forks_range[1] - int(m.group("forks_start"))) / (forks_range[1] - forks_range[0])

    '''
    if (picked_template == 3) and int(m.group("events_count_end")) != events_count_range[1]:
        attribute_to_selectivity["events_count"] = (int(m.group("events_count_end")) - events_count_range[0]) / (events_count_range[1] - events_count_range[0])
    '''
    
    if (picked_template == 3) and int(m.group("issues_start")) != issues_range[0]:
        attribute_to_selectivity["issues"] = (issues_range[1] - int(m.group("issues_start"))) / (issues_range[1] - issues_range[0])

    if (picked_template == 4):
        attribute_to_selectivity["start_date"] = (int(m.group("start_date_end")) - int(m.group("start_date_start"))) / (start_date_range[1] - start_date_range[0])
        attribute_to_selectivity["end_date"] = (int(m.group("end_date_end")) - int(m.group("end_date_start"))) / (end_date_range[1] - end_date_range[0])     

    picked_query = min(attribute_to_selectivity, key=attribute_to_selectivity.get)
    # print(line, picked_query, attribute_to_selectivity[picked_query])

    if picked_query == "start_date":
        # query.where(predicates.between('start_date', start_date_range[0], int(m.group("start_date_end"))))
        # print('query:', 'start_date', start_date_range[0], int(m.group("start_date_end")))
        query.where(predicates.between('start_date', int(m.group("start_date_start")), int(m.group("start_date_end"))))
        # print('query:', 'start_date', int(m.group("start_date_start")), int(m.group("start_date_end")))

    elif picked_query == "end_date":
        # query.where(predicates.between('end_date', int(m.group("end_date_start")), end_date_range[1]))
        # print('query:', 'end_date', int(m.group("end_date_start")), end_date_range[1])
        query.where(predicates.between('end_date', int(m.group("end_date_start")), int(m.group("end_date_end"))))
        # print('query:', 'end_date', int(m.group("end_date_start")), int(m.group("end_date_end")))

    elif picked_query == "stars":
        query.where(predicates.between('stars', int(m.group("stars_start")), stars_range[1]))
        # print('query:', 'stars', int(m.group("stars_start")), stars_range[1])

    elif picked_query == "forks":
        query.where(predicates.between('forks', int(m.group("forks_start")), forks_range[1]))
        # print('query:', 'forks', int(m.group("forks_start")), forks_range[1])

    elif picked_query == "events_count":
        query.where(predicates.between('events_count', events_count_range[0], int(m.group("events_count_end"))))
        # print('query:', 'events_count', events_count_range[0], int(m.group("events_count_end")))

    elif picked_query == "issues":
        query.where(predicates.between('issues', int(m.group("issues_start")), issues_range[1]))
        # print('query:', 'issues', int(m.group("issues_start")), issues_range[1])

    else:
        print("wrong!")
        exit(0)

    if picked_template == 1:

        '''
        regex_template_1 = re.compile(r"where start_date <= (?P<start_date_end>[0-9.]+?) AND end_date >= (?P<end_date_start>[0-9.]+?) AND stars >= (?P<stars_start>[0-9.]+?),")
        '''

        # print(picked_template, int(m.group("end_date_start")), int(m.group("start_date_end")), int(m.group("stars_start")))
        if picked_query == "start_date":

            expr = exp.And(
                exp.GE(exp.IntBin("end_date"), int(m.group("end_date_start"))),
                exp.GE(exp.IntBin("stars"), int(m.group("stars_start"))),
            ).compile()

        if picked_query == "end_date":

            expr = exp.And(
                exp.LE(exp.IntBin("start_date"), int(m.group("start_date_end"))),
                exp.GE(exp.IntBin("stars"), int(m.group("stars_start"))),
            ).compile()

        if picked_query == "stars":

            expr = exp.And(
                exp.LE(exp.IntBin("start_date"), int(m.group("start_date_end"))),
                exp.GE(exp.IntBin("end_date"), int(m.group("end_date_start"))),
            ).compile()

    if picked_template == 2:

        '''
        regex_template_2 = re.compile(r"where stars >= (?P<stars_start>[0-9.]+?) and forks >= (?P<forks_start>[0-9.]+?),")
        '''

        # print(picked_template, int(m.group("forks_start")), int(m.group("stars_start")))

        if picked_query == "stars":

            expr = exp.GE(exp.IntBin("forks"), int(m.group("forks_start"))).compile()

        if picked_query == "forks":

            expr = exp.GE(exp.IntBin("stars"), int(m.group("stars_start"))).compile()

    if picked_template == 3:

        '''
        regex_template_3 = re.compile(r"where events_count <= (?P<events_count_end>[0-9.]+?) and issues >= (?P<issues_start>[0-9.]+?) AND stars >= (?P<stars_start>[0-9.]+?),")
        '''

        # print(picked_template, int(m.group("issues_start")), int(m.group("stars_start")), int(m.group("events_count_end")))

        if picked_query == "events_count":
            expr = exp.And(
                exp.GE(exp.IntBin("issues"), int(m.group("issues_start"))),
                exp.GE(exp.IntBin("stars"), int(m.group("stars_start"))),
            ).compile()

        if picked_query == "issues":
            expr = exp.And(
                exp.LE(exp.IntBin("events_count"), int(m.group("events_count_end"))),
                exp.GE(exp.IntBin("stars"), int(m.group("stars_start"))),
            ).compile()

        if picked_query == "stars":
            expr = exp.And(
                exp.LE(exp.IntBin("events_count"), int(m.group("events_count_end"))),
                exp.GE(exp.IntBin("issues"), int(m.group("issues_start"))),
            ).compile()


    if picked_template == 4:

        '''
        regex_template_4 = re.compile(r"where start_date >= (?P<start_date_start>[0-9.]+?) AND start_date <= (?P<start_date_end>[0-9.]+?) AND end_date >= (?P<end_date_start>[0-9.]+?) AND end_date <= (?P<end_date_end>[0-9.]+?);")
        '''

        # print(picked_template, int(m.group("start_date_start")), int(m.group("start_date_end")), int(m.group("end_date_start")), int(m.group("end_date_end")))

        if picked_query == "start_date":
            expr = exp.And(
                exp.GE(exp.IntBin("end_date"), int(m.group("end_date_start"))),
                exp.LE(exp.IntBin("end_date"), int(m.group("end_date_end"))),
            ).compile()

        if picked_query == "end_date":
            expr = exp.And(
                exp.GE(exp.IntBin("start_date"), int(m.group("start_date_start"))),
                exp.LE(exp.IntBin("start_date"), int(m.group("start_date_end"))),
            ).compile()


    policy = {
        'expressions': expr,
        'total_timeout':300000,
        'socket_timeout': 300000
    }

    records_query = query.results(policy)
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
            if not return_num_points:
                print("return_num_points not found")
                exit(0)
        else:
            primary_key, rec = total_vect[i]
            key = ('macro_bench', 'github_macro', primary_key)
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

threads_per_node = 10 # Out of time.
total_num_nodes = 10

manager = multiprocessing.Manager()
return_dict = manager.dict()

def load_all_points(client_idx):
    file_path = "/mntData/github_split_10/x{}".format(client_idx)
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
                else:
                    rec[header[column - 1]] = int(col)
                column += 1

            total_vect.append((primary_key, rec))
            if loaded_lines == total_points:
                break

if not no_insert:
    load_all_points(int(sys.argv[1]))

if int(sys.argv[1]) == 0:
    with open('/proj/trinity-PG0/Trinity/baselines/aerospike/python/github_search_insert_throughput.txt', 'a') as f:
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

with open('/proj/trinity-PG0/Trinity/baselines/aerospike/python/github_search_insert_throughput.txt', 'a') as f:
    fcntl.flock(f, fcntl.LOCK_EX)
    print(sum([return_dict[worker]["throughput"] for worker in return_dict]), file=f)
    fcntl.flock(f, fcntl.LOCK_UN)