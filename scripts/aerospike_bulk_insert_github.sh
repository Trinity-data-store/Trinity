# /proj/trinity-PG0/dependencies/aerospike/aerospike-loader/run_loader -h 10.10.1.12 -n github -c /proj/trinity-PG0/Trinity/baselines/aerospike/column_github.json /mntData2/github/github_events_processed.csv
cd /proj/trinity-PG0/dependencies/aerospike/aerospike-loader/
sudo ./build
/proj/trinity-PG0/dependencies/aerospike/aerospike-loader/run_loader -h 10.10.1.12 -n github -c /proj/trinity-PG0/Trinity/baselines/aerospike/column_github_10D.json /mntData2/github/github_events_processed_9.csv