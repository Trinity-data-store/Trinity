# "2009-01-01","2009-01-01",-73.99748400000001,40.72595400000001,-74.00593799999999,40.73570200000001,2,0.9,5.4,0,0,0,0,0,0,5.4

# pickup_date, dropoff_date, pickup_longitude, pickup_latitude, dropoff_longitude, dropoff_latitude, passenger_count, trip_distance, fare_amount, extra, mta_tax, tip_amount, tolls_amount, ehail_fee, improvement_surcharge, total_amount

processed_lines = 0

with open("/mntData2/nyc_taxi/nyc_taxi_processed_675.csv", 'a') as outfile:
    with open("/mntData2/nyc_taxi/nyc_taxi_processed_ch.csv") as infile:
        for line in infile:

            outfile.write(line)

            if processed_lines % 1000000 == 0:
                print(processed_lines)
            
            processed_lines += 1
            if processed_lines == 675200000:
                break
