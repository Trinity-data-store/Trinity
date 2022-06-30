#!/bin/bash

# Path
trinity_path="/proj/trinity-PG0/Trinity"
dependencies_path="/proj/trinity-PG0/dependencies"
local_path="/mntData"
data_dir="/mntData2"

# Installing CLickhouse DB
pip3 install clickhouse-driver[lz4]
sudo apt-get install -y apt-transport-https ca-certificates dirmngr
sudo apt-key adv --keyserver hkp://keyserver.ubuntu.com:80 --recv 8919F6BD2B48D754
echo "deb https://packages.clickhouse.com/deb stable main" | sudo tee /etc/apt/sources.list.d/clickhouse.list
sudo apt-get update
sudo DEBIAN_FRONTEND=noninteractive apt-get install -y clickhouse-server 
sudo apt-get install -y clickhouse-client
sudo timedatectl set-timezone America/New_York # Set time zone


# Set up clickhouse data
sudo mkdir -p $local_path/clickhouse/
sudo chown -R clickhouse:clickhouse $local_path/clickhouse/

# Configure clickhouse DB
sudo cp $trinity_path/baselines/clickhouse/clickhouse_config.xml /etc/clickhouse-server/config.xml
sudo cp $trinity_path/baselines/clickhouse/clickhouse_users.xml /etc/clickhouse-server/users.xml
sudo service clickhouse-server restart


exit 0

# Start clickhouse
sudo service clickhouse-server start
sudo service clickhouse-server stop

sudo cp /proj/trinity-PG0/Trinity/baselines/clickhouse/clickhouse_config.xml /etc/clickhouse-server/config.xml
sudo cp /proj/trinity-PG0/Trinity/baselines/clickhouse/clickhouse_users.xml /etc/clickhouse-server/users.xml
sudo service clickhouse-server restart


# ClickHouse TPCH (Client Node)
clickhouse-client --database=default --query="DROP TABLE IF EXISTS tpch_macro";
clickhouse-client --database=default --query="CREATE TABLE IF NOT EXISTS tpch_macro (ID UInt32, QUANTITY UInt32, EXTENDEDPRICE UInt32, DISCOUNT UInt32, TAX UInt32, SHIPDATE UInt32, COMMITDATE UInt32, RECEIPTDATE UInt32, TOTALPRICE UInt32, ORDERDATE UInt32) ENGINE = Distributed(test_trinity, default, tpch_macro, rand())";

# ClickHouse TPCH (Data Node)
clickhouse-client --database=default --query="DROP TABLE IF EXISTS tpch_macro";
clickhouse-client --database=default --query="CREATE TABLE IF NOT EXISTS tpch_macro (ID UInt32, QUANTITY UInt32, EXTENDEDPRICE UInt32, DISCOUNT UInt32, TAX UInt32, SHIPDATE UInt32, COMMITDATE UInt32, RECEIPTDATE UInt32, TOTALPRICE UInt32, ORDERDATE UInt32) Engine = MergeTree ORDER BY (ID)";


clickhouse-client --database=default --query="TRUNCATE TABLE tpch_macro";
clickhouse-client --database=default --query="SELECT COUNT(*) FROM tpch_macro";


# Clickhouse TPCH (insert Data)
cat /mntData2/tpch/data_300/tpch_processed_1B.csv | clickhouse-client --query="INSERT INTO tpch_macro FORMAT CSV";