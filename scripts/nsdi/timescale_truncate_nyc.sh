export PGPASSWORD=adifficultpassword && psql -h localhost -d defaultdb -U postgres -p 5432 -c "DROP TABLE nyc_taxi"
export PGPASSWORD=adifficultpassword && psql -h localhost -d defaultdb -U postgres -p 5432 -c "CREATE TABLE nyc_taxi (
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
    CONSTRAINT id_pkey PRIMARY KEY (pkey)
);"

export PGPASSWORD=adifficultpassword && psql -h localhost -d defaultdb -U postgres -p 5432 -c "CREATE INDEX ON nyc_taxi (trip_dist, pickup_date DESC);"
export PGPASSWORD=adifficultpassword && psql -h localhost -d defaultdb -U postgres -p 5432 -c "CREATE INDEX ON nyc_taxi (fare_amt, pickup_date DESC);"
export PGPASSWORD=adifficultpassword && psql -h localhost -d defaultdb -U postgres -p 5432 -c "CREATE INDEX ON nyc_taxi (tip_amt, pickup_date DESC);"
export PGPASSWORD=adifficultpassword && psql -h localhost -d defaultdb -U postgres -p 5432 -c "CREATE INDEX ON nyc_taxi (dropoff_date, pickup_date DESC);"
export PGPASSWORD=adifficultpassword && psql -h localhost -d defaultdb -U postgres -p 5432 -c "CREATE INDEX ON nyc_taxi (passenger_cnt, pickup_date DESC);"
export PGPASSWORD=adifficultpassword && psql -h localhost -d defaultdb -U postgres -p 5432 -c "CREATE INDEX ON nyc_taxi (pickup_lon, pickup_date DESC);"
export PGPASSWORD=adifficultpassword && psql -h localhost -d defaultdb -U postgres -p 5432 -c "CREATE INDEX ON nyc_taxi (pickup_lat, pickup_date DESC);"
