# https://dev.to/zergon321/creating-a-clickhouse-cluster-part-i-sharding-4j20
from clickhouse_driver import Client
import time
import random
import sys
import random
from datetime import datetime
from multiprocessing import Process
from concurrent.futures import ProcessPoolExecutor
from random import randrange

master = ("10.10.1.2", "9000")

total_num_points = 828056295
query_selecitivity = [0.05 / 100, 0.15 / 100] # Scale query selectivity
num_workers = 5
total_num_queries = 1000

''' Query templates
Select COUNT(*) from github_events_final where stars > 100 and forks > 100 => 398875 points
Select COUNT(*) from github_events_final where adds < 10000 and dels < 10000 AND add_del_ratio < 10 => 812106705 points
Select COUNT(*) from github_events_final where events_count < 10000 and issues > 1 AND stars > 1 => 8102660 points
Select COUNT(*) from github_events_final where start_date <= 20190101 AND end_date >= 20200601 AND stars >= 1000 => 121365 points
'''

dates = [0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31]
stars_range = [0, 354850]  # min, max
forks_range = [0, 262926]  # min, max
adds_range = [0, 3176317839]  # min, max
dels_range = [0, 1006504276]  # min, max
issues_range = [0, 379379]
events_count_range = [1, 7451541]
add_del_ratio_range = [0, 117942850]  # min, max
start_date_range = [20110211, 20201206]  # min, max
end_date_range = [20110211, 20201206]  # min, max

# Reduced range
stars_range = [0, 2000]
forks_range = [0, 200]
adds_range = [0, 20000]
dels_range = [0, 20000]
issues_range = [0, 10]
events_count_range = [0, 20000]
add_del_ratio_range = [0, 20]
start_date_range = [20180101, 20201206]
end_date_range = [20180101, 20201206]

# query_selecitivity = {
#     1: [3 * 1000000 * 0.5, 3 * 1000000 * 1.5],
#     2: [3 * 1000000 * 0.5, 3 * 1000000 * 1.5],
#     3: [3 * 1000000 * 0.5, 3 * 1000000 * 1.5],
#     4: [3 * 1000000 * 0.5, 3 * 1000000 * 1.5]
# }

def generate_date_int(start, end):

    date_candidate = 0

    while date_candidate < start or date_candidate > end: 
        year = randrange(2011, 2020)
        month = randrange(1, 12 + 1)
        max_possible_date = dates[month]
        if month == 2 and year % 4 == 0:
            max_possible_date = 29
        date = randrange(1, max_possible_date + 1)
        date_candidate = 10000 * year + month * 100 + date

    return date_candidate

def generate_queries(template_id):

    if template_id == 1:
        query = "Select COUNT(*) from github_events_final where stars >= {} and forks >= {}".format(randrange(stars_range[0], stars_range[1] + 1), randrange(forks_range[0], forks_range[1] + 1))
    if template_id == 2:
        query = "Select COUNT(*) from github_events_final where adds <= {} and dels <= {} AND add_del_ratio <= {}".format(randrange(adds_range[0], adds_range[1] + 1), randrange(dels_range[0], dels_range[1] + 1), randrange(add_del_ratio_range[0], add_del_ratio_range[1] + 1))
    if template_id == 3:
        query = "Select COUNT(*) from github_events_final where events_count <= {} and issues >= {} AND stars >= {}".format(randrange(events_count_range[0], events_count_range[1] + 1), randrange(issues_range[0], issues_range[1] + 1), randrange(stars_range[0], stars_range[1] + 1))
    if template_id == 4:
        query = "Select COUNT(*) from github_events_final where start_date <= {} AND end_date >= {} AND stars >= {}".format(generate_date_int(start_date_range[0], start_date_range[1]), generate_date_int(end_date_range[0], end_date_range[1]), randrange(stars_range[0], stars_range[1] + 1))
    return query

def generate_test_queries(template_id):

    if template_id == 2:

        # query = "Select COUNT(*) from github_events_final where start_date <= {} AND end_date >= {} AND stars >= {}".format(generate_date_int(start_date_range[0], start_date_range[1]), generate_date_int(end_date_range[0], end_date_range[1]), stars_range[0])

        
        start_date_start = generate_date_int(start_date_range[0], start_date_range[1])
        start_date_end = generate_date_int(start_date_start, start_date_range[1])
        end_date_start = generate_date_int(end_date_range[0], end_date_range[1])
        end_date_end = generate_date_int(end_date_start, end_date_range[1])

        query = "Select COUNT(*) from github_events_final where start_date >= {} AND start_date <= {} AND end_date >= {} AND end_date <= {}".format(start_date_start, start_date_end, end_date_start, end_date_end)
        

    return query

def find_queries_per_process(num_queries):

    finished_queries = 0
    client = Client(master[0], port=master[1])

    while finished_queries < num_queries:

        query_template = randrange(1, 4 + 1)

        # query = generate_queries(query_template)

        query_template = 2
        query = generate_test_queries(query_template)

        result_count = client.execute(query)[0][0]
        # print(query_template, query, result_count)

        if result_count >= query_selecitivity[0] * total_num_points and result_count <= query_selecitivity[1] * total_num_points:
        # if result_count >= query_selecitivity[query_template][0] and result_count <= query_selecitivity[query_template][1]:
            print("{}, found points: {}".format(query, result_count), flush=True)
            finished_queries += 1
        # print("{}, found points: {}".format(query, result_count), flush=True)
        # exit(0)
if __name__ == "__main__":

    processes = []

    for _ in range(num_workers):
        p = Process(target=find_queries_per_process, args=(total_num_queries/num_workers, ))
        p.start()
        processes.append(p)

    for p in processes:
        p.join()