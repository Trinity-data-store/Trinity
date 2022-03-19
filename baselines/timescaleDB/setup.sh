# https://docs.timescale.com/install/latest/self-hosted/installation-debian/#install-self-hosted-timescaledb-on-debian-based-systems
sudo apt install gnupg postgresql-common apt-transport-https lsb-release wget
sudo /usr/share/postgresql-common/pgdg/apt.postgresql.org.sh
sudo curl -L https://packagecloud.io/timescale/timescaledb/gpgkey | sudo apt-key add -
sudo sh -c "echo 'deb https://packagecloud.io/timescale/timescaledb/ubuntu/ $(lsb_release -c -s) main' > /etc/apt/sources.list.d/timescaledb.list"
wget --quiet -O - https://packagecloud.io/timescale/timescaledb/gpgkey | sudo apt-key add -
sudo apt update
sudo apt-get -y install postgresql
sudo apt install timescaledb-2-postgresql-14

# Add 
# shared_preload_libraries = 'timescaledb'
# to the back
# listen_addresses = '*' 
# max_prepared_transactions = 150
sudo nano /etc/postgresql/14/main/postgresql.conf

# host    all             all             0.0.0.0/0               trust
# local  all   all   trust
sudo nano /etc/postgresql/14/main/pg_hba.conf
sudo service postgresql restart
sudo -u postgres psql

CREATE database example;
\c example
CREATE EXTENSION IF NOT EXISTS timescaledb;





sudo -u postgres psql -d example

# https://github.com/timescale/timescaledb/issues/550
# This will help with setting up postgres

SELECT delete_data_node('dn1');

SELECT add_data_node('dn5', host => '172.28.229.153', database => 'dn5');
SELECT add_data_node('dn3', host => '172.28.229.151', database => 'dn3');
SELECT add_data_node('dn2', host => '172.28.229.149', database => 'dn2');
SELECT add_data_node('dn1', host => '172.28.229.148', database => 'dn1');
SELECT add_data_node('dn4', host => '172.28.229.152', database => 'dn4');

DROP TABLE example;

CREATE TABLE IF NOT EXISTS example (
   id int,
   time TIMESTAMP WITHOUT TIME ZONE NOT NULL,
   latitude int NULL,
   longitude int NULL,
   version int NULL
);

# By default, the replication factor is set to 1, so there is no native replication. You can increase this number when you create the hypertable
# For example, to replicate the data across a total of three data nodes:
# https://docs.timescale.com/timescaledb/latest/how-to-guides/multinode-timescaledb/multinode-ha/#native-replication
SELECT create_distributed_hypertable('example', 'time', 'id', chunk_time_interval => INTERVAL '1 day', data_nodes => '{ "dn1", "dn2", "dn3", "dn4", "dn5" }');

# (id, time, latitude, longitude, version)
INSERT INTO example VALUES (1, '2020-12-14 13:45', 1014241, 12041241, 1);

# https://docs.timescale.com/timescaledb/latest/overview/core-concepts/hypertables-and-chunks/#hypertable-benefits

