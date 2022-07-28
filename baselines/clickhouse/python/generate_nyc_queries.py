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

total_num_points = 675200000
query_selecitivity = [0.05 / 100, 0.15 / 100] # Scale query selectivity
num_workers = 5
total_num_queries = 1000

''' Query templates
select count(*) from nyc_taxi where trip_distance < 30 and fare_amount between 0 AND 1000;
select count(*) from nyc_taxi where trip_distance >= 14 and trip_distance <= 30 and fare_amount < 1000 and tip_amount < 40;
select count(*) from nyc_taxi where pickup_date >= '2008-12-31' and pickup_date <= '2016-01-02' and dropoff_date >= '2008-12-31' and dropoff_date <= '2016-01-02';
select count(*) from nyc_taxi where pickup_date <= '2016-01-02' and passenger_count = 1;
select count(*) from nyc_taxi where pickup_longitude BETWEEN {} and {} AND pickup_latitude BETWEEN {} AND {};
'''

dates = [0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31]

# Reduced range
trip_distance_range = [0, 300]
fare_amount_range = [0, 10000]
tip_amount_range = [0, 400]

pickup_date_range = [20090101, 20160618]
dropoff_date_range = [19700101, 20161103]
passenger_count_range = [0, 30]
pickup_longitude_range = [0, 90]
pickup_latitude_range = [0, 90]

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

        trip_distance_end = randrange(trip_distance_range[0], trip_distance_range[1] + 1)
        fare_amount_start = randrange(fare_amount_range[0], fare_amount_range[1] + 1)
        fare_amount_end = randrange(fare_amount_start, fare_amount_range[1] + 1)
        query = "select count(*) from nyc_taxi where trip_distance <= {} and fare_amount between {} AND {};".format(trip_distance_end, fare_amount_start, fare_amount_end)

    if template_id == 2:
        trip_distance_start = randrange(trip_distance_range[0], trip_distance_range[1] + 1)
        trip_distance_end = randrange(trip_distance_start, trip_distance_range[1] + 1)
        fare_amount_end = randrange(fare_amount_range[0], trip_distance_range[1] + 1)
        tip_amount_end = randrange(tip_amount_range[0], tip_amount_range[1] + 1)
        query = "select count(*) from nyc_taxi where trip_distance >= {} and trip_distance <= {} and fare_amount <= {} and tip_amount <= {};".format(trip_distance_start, trip_distance_end, fare_amount_end, tip_amount_end)

    if template_id == 3:

        pickup_date_start = generate_date_int(pickup_date_range[0], pickup_date_range[1])
        pickup_date_end = generate_date_int(pickup_date_start, pickup_date_range[1])

        dropoff_date_start = generate_date_int(dropoff_date_range[0], dropoff_date_range[1])
        dropoff_date_end = generate_date_int(dropoff_date_start, dropoff_date_range[1])

        query = "select count(*) from nyc_taxi where pickup_date >= {} and pickup_date <= {} and dropoff_date >= {} and dropoff_date <= {}".format(pickup_date_start, pickup_date_end, dropoff_date_start, dropoff_date_end)


    if template_id == 4:

        pickup_date_end = generate_date_int(pickup_date_range[0], pickup_date_range[1])
        passenger_count = randrange(passenger_count_range[0], passenger_count_range[1] + 1)
        query = "select count(*) from nyc_taxi where pickup_date <= {} and passenger_count = {};".format(pickup_date_end, passenger_count)

    if template_id == 5:

        pickup_longitude_start = randrange(pickup_longitude_range[0], pickup_longitude_range[1] + 1)
        pickup_longitude_end = randrange(pickup_longitude_start, pickup_longitude_range[1] + 1)
        pickup_latitude_start = randrange(pickup_latitude_range[0], pickup_latitude_range[1] + 1)
        pickup_latitude_end = randrange(pickup_latitude_start, pickup_latitude_range[1] + 1)

        query = "select count(*) from nyc_taxi where pickup_longitude BETWEEN {} and {} AND pickup_latitude BETWEEN {} AND {};".format(pickup_longitude_start, pickup_longitude_end, pickup_latitude_start, pickup_latitude_end)

    return query


def find_queries_per_process(num_queries):

    finished_queries = 0
    client = Client(master[0], port=master[1])

    while finished_queries < num_queries:

        query_template = randrange(1, 4 + 1)
        while query_template == 2:
            query_template = randrange(1, 4 + 1)
        # query_template = 1
        # query_template = randrange(3, 4 + 1)
        query = generate_queries(query_template)
        result_count = client.execute(query)[0][0]

        if result_count >= query_selecitivity[0] * total_num_points and result_count <= query_selecitivity[1] * total_num_points:
            print("{}, found points: {}".format(query, result_count), flush=True)
            finished_queries += 1

if __name__ == "__main__":

    processes = []
    for _ in range(num_workers):
        p = Process(target=find_queries_per_process, args=(total_num_queries/num_workers, ))
        p.start()
        processes.append(p)

    for p in processes:
        p.join()