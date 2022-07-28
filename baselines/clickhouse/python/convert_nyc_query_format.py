import re 
import sys

datapath = "/proj/trinity-PG0/Trinity/queries/nyc/nyc_taxi_query"
outfile = open("/proj/trinity-PG0/Trinity/queries/nyc/nyc_query_converted", "w")  # write mode

# ["pickup_date", "dropoff_date", "pickup_lon", "pickup_lat", "dropoff_lon", "dropoff_lat", "passenger_cnt", "trip_dist", "fare_amt", "extra", "mta_tax", "tip_amt", "tolls_amt", "impt_sur", "total_amt"]

if __name__ == "__main__": 

    regex_template_1 = re.compile(r"where trip_distance <= (?P<trip_distance_end>[0-9.]+?) and fare_amount between (?P<fare_amount_start>[0-9.]+?) AND (?P<fare_amount_end>[0-9.]+?),")
    regex_template_2 = re.compile(r"where trip_distance >= (?P<trip_distance_start>[0-9.]+?) and trip_distance <= (?P<trip_distance_end>[0-9.]+?) and fare_amount <= (?P<fare_amount_end>[0-9.]+?) and tip_amount <= (?P<tip_amount_end>[0-9.]+?),")
    regex_template_3 = re.compile(r"where pickup_date >= (?P<pickup_date_start>[0-9.]+?) and pickup_date <= (?P<pickup_date_end>[0-9.]+?) and dropoff_date >= (?P<dropoff_date_start>[0-9.]+?) and dropoff_date <= (?P<dropoff_date_end>[0-9.]+?),")
    regex_template_4 = re.compile(r"where pickup_date <= (?P<pickup_date_end>[0-9.]+?) and passenger_count = (?P<passenger_count>[0-9.]+?),")
    regex_template_5 = re.compile(r"where pickup_longitude BETWEEN (?P<pickup_longitude_start>[0-9.]+?) and (?P<pickup_longitude_end>[0-9.]+?) AND pickup_latitude BETWEEN (?P<pickup_latitude_start>[0-9.]+?) AND (?P<pickup_latitude_end>[0-9.]+?),")

    with open("{}".format(datapath)) as ifile:

        for line in ifile:

            m = regex_template_1.search(line)
            if m:
                out_line = "7,{},{},8,{},{}".format(-1, m.group("trip_distance_end"), m.group("fare_amount_start"), m.group("fare_amount_end"))
                outfile.write(out_line + "\n")
                continue

            m = regex_template_2.search(line)
            if m:
                out_line = "7,{},{},8,{},{},11,{},{}".format(m.group("trip_distance_start"), m.group("trip_distance_end"), -1, m.group("fare_amount_end"), -1, m.group("tip_amount_end"))
                outfile.write(out_line + "\n")
                continue

            m = regex_template_3.search(line)
            if m:
                out_line = "0,{},{},1,{},{}".format(m.group("pickup_date_start"), m.group("pickup_date_end"), m.group("dropoff_date_start"), m.group("dropoff_date_end"))
                outfile.write(out_line + "\n")
                continue

            m = regex_template_4.search(line)
            if m:
                out_line = "0,{},{},6,{},{}".format(-1, m.group("pickup_date_end"), m.group("passenger_count"), m.group("passenger_count"))
                outfile.write(out_line + "\n")
                continue

            m = regex_template_5.search(line)
            if m:
                out_line = "2,{},{},3,{},{}".format(m.group("pickup_longitude_start"), m.group("pickup_longitude_end"), m.group("pickup_latitude_start"), m.group("pickup_latitude_end"))
                outfile.write(out_line + "\n")
                continue

    outfile.close()