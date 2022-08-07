import psycopg2
from pgcopy import CopyManager
import pandas as pd 
from datetime import datetime
import time
import random
import sys
import random
import re

CONNECTION = "dbname=defaultdb host=localhost user=postgres password=adifficultpassword sslmode=disable"

COLS = ["pkey", "events_count", "authors_count", "forks", "stars", "issues", "pushes", "pulls", "downloads", "start_date", "end_date"]
filename = "/proj/trinity-PG0/Trinity/queries/github/github_query_final"

dates = [0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31]


with open(filename) as file:
    lines = file.readlines()
    lines = [line.rstrip() for line in lines]

# CONN = psycopg2.connect(CONNECTION)
# cursor = CONN.cursor()
total_points = 50000000
total_queries = 100 # should be 500
processes = []

threads_per_node = 20

def query_each_worker(worker_idx, total_workers):

    CONN = psycopg2.connect(CONNECTION)
    cursor = CONN.cursor()
    start_time = time.time()

    for i in range(worker_idx, total_queries, total_workers):

        line = lines[i] 

        query = line.split(";,")[0] + ";"
        query_select = query.replace("COUNT(*)", "*").replace("github_events_final", "github_events")
        # print(query_select)

        # StringRegex = re.compile(r'start_date <= \d\d\d\d\d\d\d\d AND end_date >= \d\d\d\d\d\d\d\d')
        StringRegex = re.compile(r'where start_date >= \d\d\d\d\d\d\d\d AND start_date <= \d\d\d\d\d\d\d\d AND end_date >= \d\d\d\d\d\d\d\d AND end_date <= \d\d\d\d\d\d\d\d;')
        while StringRegex.search(query_select):
            old_string = StringRegex.search(query_select).group()
            new_string = old_string
            DateRegex = re.compile(r'\d\d\d\d\d\d\d\d')

            # for i in range(4):
            i = -1
            while DateRegex.search(new_string):
                i += 1

                old_dateString = DateRegex.search(new_string).group()
                new_dateString = old_dateString[0:4] + "-" + old_dateString[4:6] + "-" + old_dateString[6:]
                
                year = int(new_dateString.split("-")[0])
                month = int(new_dateString.split("-")[1])
                day = int(new_dateString.split("-")[2])
                if i == 0:
                    if month <= 12 and month > 0 and day > dates[month]:
                        month += 1
                        day = 1
                if i == 1:
                    if month <= 12 and month > 0 and day > dates[month]:
                        day = dates[month]
                        if year % 4 == 0 and month == 2:
                            day = 29
                
                if month < 10 and month > 0:
                    month_string = "0{}".format(month)
                elif month == 0:
                    month_string = "01"
                else:
                    month_string = "{}".format(month)
                if day < 10 and day > 0:
                    day_string = "0{}".format(day)
                elif day == 0:
                    day_string = "01"
                else:
                    day_string = "{}".format(day)
                new_dateString = old_dateString[0:4] + "-" + month_string + "-" + day_string

                if i == 0 and month > 12:
                    new_dateString = str(int(new_dateString.split("-")[0]) + 1) + "-01-01"
                if i == 0 and month == 0:
                    new_dateString = str(int(new_dateString.split("-")[0])) + "-01-01"
                if i == 1 and month > 12:
                    new_dateString = str(int(new_dateString.split("-")[0])) + "-12-31"
                if i == 1 and month == 0:
                    new_dateString = str(int(new_dateString.split("-")[0]) - 1) + "-12-31"

                new_string = new_string.replace(old_dateString, "\'{}\'".format(new_dateString))

            query_select = query_select.replace(old_string, new_string)

        print(query_select)
        cursor.execute(query_select)
        results = cursor.fetchall()
        effective_pts_count += len(results)
        del results
        end = time.time()
    return effective_pts_count / (end_time - start_time)

def query_worker(worker, total_workers, return_dict):

    throughput = query_each_worker(worker, total_workers)
    return_dict[worker] = {"throughput": throughput}

manager = multiprocessing.Manager()
return_dict = manager.dict()

if int(sys.argv[1]) == 0:
    with open('/proj/trinity-PG0/Trinity/baselines/timescaleDB/python/github_search_throughput.txt', 'a') as f:
        fcntl.flock(f, fcntl.LOCK_EX)
        print("---- {}K ----".format(int(total_points / 1000)), file=f)
        fcntl.flock(f, fcntl.LOCK_UN)

for worker in range(threads_per_node):
    p = Process(target=query_worker, args=(worker, threads_per_node, return_dict, ))
    p.start()
    processes.append(p)

for p in processes:
    p.join()

print("total throughput", sum([return_dict[worker]["throughput"] for worker in return_dict]))

with open('/proj/trinity-PG0/Trinity/baselines/timescaleDB/python/github_search_throughput.txt', 'a') as f:
    fcntl.flock(f, fcntl.LOCK_EX)
    print(sum([return_dict[worker]["throughput"] for worker in return_dict]), file=f)
    fcntl.flock(f, fcntl.LOCK_UN)
