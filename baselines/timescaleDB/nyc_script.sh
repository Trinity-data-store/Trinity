sudo -u postgres psql postgres
alter user postgres with password 'adifficultpassword';

\c defaultdb
DROP TABLE nyc_taxi;
CREATE TABLE nyc_taxi (
    pkey           BIGINT             NOT NULL,
    pickup_date     TIMESTAMP NOT NULL,
    dropoff_date     TIMESTAMP NOT NULL,
    pickup_lon     float8             NOT NULL,
    pickup_lat     float8             NOT NULL,
    dropoff_lon     float8             NOT NULL,
    dropoff_lat     float8             NOT NULL,
    passenger_cnt     INT             NOT NULL,
    trip_dist     INT             NOT NULL,
    fare_amt     INT             NOT NULL,
    extra     INT             NOT NULL,
    mta_tax     INT             NOT NULL,
    tip_amt     INT             NOT NULL,
    tolls_amt     INT             NOT NULL,
    impt_sur     INT             NOT NULL,
    total_amt     INT             NOT NULL,
    PRIMARY KEY (pickup_date, pkey)
);


SELECT add_data_node('dn1', host => '10.10.1.12');
SELECT add_data_node('dn2', host => '10.10.1.13');
SELECT add_data_node('dn3', host => '10.10.1.14');
SELECT add_data_node('dn4', host => '10.10.1.15');
SELECT add_data_node('dn5', host => '10.10.1.16');

SELECT create_distributed_hypertable('nyc_taxi', 'pickup_date', 'pkey', 
    data_nodes => '{ "dn1", "dn2", "dn3", "dn4", "dn5"}');

CREATE INDEX ON nyc_taxi (trip_dist, pickup_date DESC);
CREATE INDEX ON nyc_taxi (fare_amt, pickup_date DESC);
CREATE INDEX ON nyc_taxi (tip_amt, pickup_date DESC);
CREATE INDEX ON nyc_taxi (dropoff_date, pickup_date DESC);
CREATE INDEX ON nyc_taxi (passenger_cnt, pickup_date DESC);
CREATE INDEX ON nyc_taxi (pickup_lon, pickup_date DESC);
CREATE INDEX ON nyc_taxi (pickup_lat, pickup_date DESC);

SELECT * FROM hypertable_detailed_size('nyc_taxi') ORDER BY node_name;
