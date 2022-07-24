CREATE TABLE nyc_taxi ENGINE = MergeTree ORDER BY (pickup_date, dropoff_date) AS SELECT
    pickup_date,
    dropoff_date,
    pickup_longitude,
    pickup_latitude,
    dropoff_longitude,
    dropoff_latitude,
    passenger_count,
    trip_distance,
    fare_amount,
    extra,
    mta_tax,
    tip_amount,
    tolls_amount,
    ehail_fee,
    improvement_surcharge,
    total_amount
FROM nyc_taxi;


pickup_date, dropoff_date, pickup_longitude, pickup_latitude, dropoff_longitude, dropoff_latitude, passenger_count, trip_distance, fare_amount, extra, mta_tax, tip_amount, tolls_amount, ehail_fee, improvement_surcharge, total_amount

select count(*) from nyc_taxi where trip_distance < 30 and fare_amount between 0 AND 1000;
select count(*) from nyc_taxi where trip_distance >= 14 and trip_distance <= 30 and fare_amount < 1000 and tip_amount < 40;
select count(*) from nyc_taxi where pickup_date >= '2008-12-31' and pickup_date <= '2016-01-02' and dropoff_date >= '2008-12-31' and dropoff_date <= '2016-01-02';
select count(*) from nyc_taxi where pickup_date <= '2016-01-02' and passenger_count = 1;
select count(*) from nyc_taxi where pickup_longitude BETWEEN {} and {} AND pickup_latitude BETWEEN {} AND {};


SELECT * FROM nyc_taxi INTO OUTFILE '/mntData2/nyc_taxi/nyc_taxi_processed_ch.csv' FORMAT CSV


pkey UInt32, pickup_date UInt32, dropoff_date UInt32, pickup_longitude Float32, pickup_latitude Float32, dropoff_longitude Float32, dropoff_latitude Float32, passenger_count UInt32, trip_distance Float32, fare_amount Float32, extra Float32, mta_tax Float32, tip_amount Float32, tolls_amount Float32, ehail_fee Float32, improvement_surcharge Float32, total_amount Float32


CREATE TABLE nyc_taxi (
    pkey UInt32,
    pickup_date UInt32,
    dropoff_date UInt32,
    pickup_longitude Float32,
    pickup_latitude Float32,
    dropoff_longitude Float32,
    dropoff_latitude Float32,
    passenger_count UInt32,
    trip_distance Float32,
    fare_amount Float32,
    extra Float32,
    mta_tax Float32,
    tip_amount Float32,
    tolls_amount Float32,
    ehail_fee Float32,
    improvement_surcharge Float32,
    total_amount Float32
) ENGINE = MergeTree ORDER BY (pickup_date, dropoff_date)

cat '/mntData2/nyc_taxi/nyc_taxi_processed.csv' | clickhouse-client --query="INSERT INTO nyc_taxi FORMAT CSV";


CREATE TABLE nyc_taxi (
    pickup_date DateTime,
    dropoff_date DateTime,
    pickup_longitude Float32,
    pickup_latitude Float32,
    dropoff_longitude Float32,
    dropoff_latitude Float32,
    passenger_count UInt32,
    trip_distance Float32,
    fare_amount Float32,
    extra Float32,
    mta_tax Float32,
    tip_amount Float32,
    tolls_amount Float32,
    ehail_fee Float32,
    improvement_surcharge Float32,
    total_amount Float32
) ENGINE = MergeTree ORDER BY (pickup_date, dropoff_date)
cat '/mntData2/nyc_taxi/nyc_taxi.csv' | clickhouse-client --query="INSERT INTO nyc_taxi FORMAT CSV";


tar xvf trips_mergetree.tar -C /mntData/clickhouse/


SELECT MAX(pickup_date), MIN(pickup_date) from nyc_taxi;
┌─max(pickup_date)─┬─min(pickup_date)─┐
│         20160630 │         20090101 │
└──────────────────┴──────────────────┘

SELECT MAX(dropoff_date), MIN(dropoff_date) from nyc_taxi;
┌─max(dropoff_date)─┬─min(dropoff_date)─┐
│          20221220 │          19700101 │
└───────────────────┴───────────────────┘

SELECT MAX(pickup_longitude), MIN(pickup_longitude) from nyc_taxi;
┌─max(pickup_longitude)─┬─min(pickup_longitude)─┐
│                  89.9 │                     0 │
└───────────────────────┴───────────────────────┘

SELECT MAX(pickup_latitude), MIN(pickup_latitude) from nyc_taxi;
┌─max(pickup_latitude)─┬─min(pickup_latitude)─┐
│                 89.8 │                    0 │
└──────────────────────┴──────────────────────┘

SELECT MAX(dropoff_longitude), MIN(dropoff_longitude) from nyc_taxi;
┌─max(dropoff_longitude)─┬─min(dropoff_longitude)─┐
│                   89.9 │                      0 │
└────────────────────────┴────────────────────────┘

SELECT MAX(dropoff_latitude), MIN(dropoff_latitude) from nyc_taxi;
┌─max(dropoff_latitude)─┬─min(dropoff_latitude)─┐
│                  89.8 │                     0 │
└───────────────────────┴───────────────────────┘

SELECT MAX(passenger_count), MIN(passenger_count) from nyc_taxi;
┌─max(passenger_count)─┬─min(passenger_count)─┐
│                  255 │                    0 │
└──────────────────────┴──────────────────────┘

SELECT MAX(trip_distance), MIN(trip_distance) from nyc_taxi;
┌─max(trip_distance)─┬─min(trip_distance)─┐
│          198623000 │                  0 │
└────────────────────┴────────────────────┘

SELECT MAX(fare_amount), MIN(fare_amount) from nyc_taxi;
┌─max(fare_amount)─┬─min(fare_amount)─┐
│         21474808 │                0 │
└──────────────────┴──────────────────┘

SELECT MAX(extra), MIN(extra) from nyc_taxi;
┌─max(extra)─┬─min(extra)─┐
│       1000 │          0 │
└────────────┴────────────┘

SELECT MAX(mta_tax), MIN(mta_tax) from nyc_taxi;
┌─max(mta_tax)─┬─min(mta_tax)─┐
│       1311.2 │            0 │
└──────────────┴──────────────┘

SELECT MAX(tip_amount), MIN(tip_amount) from nyc_taxi;
┌─max(tip_amount)─┬─min(tip_amount)─┐
│       3950588.8 │               0 │
└─────────────────┴─────────────────┘

SELECT MAX(tolls_amount), MIN(tolls_amount) from nyc_taxi;
┌─max(tolls_amount)─┬─min(tolls_amount)─┐
│          21474836 │                 0 │
└───────────────────┴───────────────────┘

SELECT MAX(improvement_surcharge), MIN(improvement_surcharge) from nyc_taxi;
┌─max(improvement_surcharge)─┬─min(improvement_surcharge)─┐
│                      137.6 │                          0 │
└────────────────────────────┴────────────────────────────┘

SELECT MAX(total_amount), MIN(total_amount) from nyc_taxi;
┌─max(total_amount)─┬─min(total_amount)─┐
│          21474830 │                 0 │
└───────────────────┴───────────────────┘


ALTER TABLE nyc_taxi DROP COLUMN ehail_fee;