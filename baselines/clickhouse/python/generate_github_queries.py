# https://dev.to/zergon321/creating-a-clickhouse-cluster-part-i-sharding-4j20
from clickhouse_driver import Client
import time
import random
import sys
import random
from datetime import datetime
from multiprocessing import Process
from concurrent.futures import ProcessPoolExecutor

master = ("10.10.1.2", "9000")

total_num_points = 1000000000
query_selecitivity = [0.05 / 100, 0.15 / 100]
num_workers = 2
total_num_queries = 1000

''' Query templates
Select COUNT(*) from github_events_final where stars > 100 and forks > 100
Select COUNT(*) from github_events_final where adds < 10000 and dels < 10000 AND add_del_ratio < 10
Select COUNT(*) from github_events_final where events_count < 10000 and issues > 1 AND stars > 1
Select COUNT(*) from github_events_final where start_date <= 20190101 AND end_date >= 20200601 AND stars >= 1000
'''

dates = [0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31]
stars_range = [19920102, 19981201]  # min, max

def generate_date_int(start, end):

    date_candidate = 0

    while date_candidate < start or date_candidate > end: 
        year = random.randrange(1990, 2022 + 1)
        month = random.randrange(1, 12 + 1)
        max_possible_date = dates[month]
        if month == 2 and year % 4 == 0:
            max_possible_date = 29
        date = random.randrange(1, max_possible_date + 1)
        date_candidate = 1000 * year + month * 100 + date

    return date_candidate

def generate_queries():


    return query

def func(num_queries):

    finished_queries = 0
    client = Client(master[0], port=master[1])

    while finished_queries < num_queries:

        query = generate_queries()
        result_count = client.execute(query)[0][0]

        if result_count >= query_selecitivity[0] * total_num_points and result_count <= query_selecitivity[1] * total_num_points:
            print("{}, found points: {}".format(query, result_count), flush=True)
            finished_queries += 1

if __name__ == "__main__":

    processes = []

    for _ in range(num_workers):
        p = Process(target=func, args=(total_num_queries/num_workers, ))
        p.start()
        processes.append(p)

    for p in processes:
        p.join()