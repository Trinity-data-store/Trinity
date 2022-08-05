manage sindex create numeric trip_dist_index ns macro_bench set nyc_taxi_macro bin trip_dist
manage sindex create numeric fare_amt_index ns macro_bench set nyc_taxi_macro bin fare_amt
# manage sindex create numeric tip_amt_index ns macro_bench set nyc_taxi_macro bin tip_amt
manage sindex create numeric pickup_date_index ns macro_bench set nyc_taxi_macro bin pickup_date
manage sindex create numeric dropoff_date_index ns macro_bench set nyc_taxi_macro bin dropoff_date
manage sindex create numeric passenger_cnt_index ns macro_bench set nyc_taxi_macro bin passenger_cnt
manage sindex create numeric pickup_lon_index ns macro_bench set nyc_taxi_macro bin pickup_lon
manage sindex create numeric pickup_lat_index ns macro_bench set nyc_taxi_macro bin pickup_lat

manage sindex delete  tip_amt_index ns macro_bench set nyc_taxi_macro