# /proj/trinity-PG0/dependencies/aerospike/aerospike-loader/run_loader -h 10.10.1.12 -n github -c /proj/trinity-PG0/Trinity/baselines/aerospike/column_github.json /mntData2/github/github_events_processed.csv
cd /proj/trinity-PG0/dependencies/aerospike/aerospike-loader/
sudo ./build
/proj/trinity-PG0/dependencies/aerospike/aerospike-loader/run_loader -h 10.10.1.12 -n github -c /proj/trinity-PG0/Trinity/baselines/aerospike/column_nyc.json /mntData2/nyc_taxi/nyc_taxi_processed_675.csv