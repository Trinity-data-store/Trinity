from clickhouse_driver import Client
import time
import random
import sys
import random
from datetime import datetime
import re

master = ("10.10.1.3", "9000")
dates = [0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31]

filename = "/proj/trinity-PG0/Trinity/queries/tpch/tpch_query_new"
out_filename = "/proj/trinity-PG0/Trinity/results/tpch_clickhouse_new_more_mem"

with open(filename) as file:
    lines = file.readlines()
    lines = [line.rstrip() for line in lines]

client = Client(master[0], port=master[1])

finished_line = 0

def clean_query(query_select):

    StringRegex = re.compile(r'BETWEEN \d\d\d\d\d\d\d\d AND \d\d\d\d\d\d\d\d')
    while StringRegex.search(query_select):
        old_string = StringRegex.search(query_select).group()
        new_string = old_string
        DateRegex = re.compile(r'\d\d\d\d\d\d\d\d')

        for i in range(2):
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
    return query_select.replace("\'", "").replace("-", "")

for line in lines:

    query = line.split(";,")[0] + ";"

    query_select = query.replace("COUNT(*)", "*")
    query_select = clean_query(query_select)

    start = time.time()
    results = client.execute(query_select)
    end = time.time()

    with open(out_filename, "a") as outfile:
        outfile.write("{}, elapsed: {}s, found points: {}\n".format(query_select, end - start, len(results)))

    del results

    finished_line += 1

