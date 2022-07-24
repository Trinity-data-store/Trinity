# "2009-01-01","2009-01-01",-73.99748400000001,40.72595400000001,-74.00593799999999,40.73570200000001,2,0.9,5.4,0,0,0,0,0,0,5.4

# pickup_date, dropoff_date, pickup_longitude, pickup_latitude, dropoff_longitude, dropoff_latitude, passenger_count, trip_distance, fare_amount, extra, mta_tax, tip_amount, tolls_amount, ehail_fee, improvement_surcharge, total_amount

processed_lines = 0

with open("/mntData2/nyc_taxi/nyc_taxi_processed.csv", 'a') as outfile:
    with open("/mntData2/nyc_taxi/nyc_taxi.csv") as infile:
        for line in infile:
            line_split = line.strip().split(",")

            line_split[0] = "".join(line_split[0][1:-1].split("-"))
            line_split[1] = "".join(line_split[1][1:-1].split("-"))


            line_split[2] = "{:.1f}".format(abs(float(line_split[2])))
            line_split[3] = "{:.1f}".format(abs(float(line_split[3])))
            line_split[4] = "{:.1f}".format(abs(float(line_split[4])))
            line_split[5] = "{:.1f}".format(abs(float(line_split[5])))

            if float(line_split[2]) >= 90 or float(line_split[3]) >= 90 or float(line_split[4]) >= 90 or float(line_split[5]) >= 90:
                continue

            line_split[7] = "{:.1f}".format(abs(float(line_split[7])))
            line_split[8] = "{:.1f}".format(abs(float(line_split[8])))
            line_split[9] = "{:.1f}".format(abs(float(line_split[9])))
            line_split[10] = "{:.1f}".format(abs(float(line_split[10])))
            line_split[11] = "{:.1f}".format(abs(float(line_split[11])))
            line_split[12] = "{:.1f}".format(abs(float(line_split[12])))
            line_split[13] = "{:.1f}".format(abs(float(line_split[13])))
            line_split[14] = "{:.1f}".format(abs(float(line_split[14])))
            line_split[15] = "{:.1f}".format(abs(float(line_split[15])))

            line_split = [str(processed_lines)] + line_split
            outfile.write(",".join(line_split) + '\n')

            if processed_lines % 1000000 == 0:
                print(processed_lines)
            
            processed_lines += 1
