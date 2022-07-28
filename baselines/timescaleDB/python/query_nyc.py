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

COLS = ["pickup_date", "dropoff_date", "pickup_lon", "pickup_lat", "dropoff_lon", "dropoff_lat", "passenger_cnt", "trip_dist", "fare_amt", "extra", "mta_tax", "tip_amt", "tolls_amt", "impt_sur", "total_amt"]
filename = "/proj/trinity-PG0/Trinity/results/nyc_clickhouse"
outfile_addr = "/proj/trinity-PG0/Trinity/results/nyc_timescale"


dates = [0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31]

if sys.argv == 2:
    filename = sys.argv[1]

with open(filename) as file:
    lines = file.readlines()
    lines = [line.rstrip() for line in lines]

CONN = psycopg2.connect(CONNECTION)
cursor = CONN.cursor()

finished_line = 0
for line in lines:

    query = line.split(";,")[0] + ";"
    query_select = query.replace("COUNT(*)", "*")
    # print(query_select)

    StringRegex = re.compile(r'\d\d\d\d\d\d\d\d')
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
            new_string = new_string.replace(old_dateString, "\'{}\'".format(new_dateString))

        query_select = query_select.replace(old_string, new_string)

    print(query_select)

    start = time.time()
    cursor.execute(query_select)
    results = cursor.fetchall()
    end = time.time()

    with open(outfile_addr, "a") as outfile:
        outfile.write("{}, elapsed: {}s, found points: {}\n".format(query_select, end - start, len(results)))

    del results
    finished_line += 1



