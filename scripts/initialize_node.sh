#!/bin/sh

# Other configuration
git config --global user.name "MaoZiming"
git config --global user.email "ziming.mao@yale.edu"
cd /mntData2/
sudo apt update
sudo apt install htop

# Install cmake
wget -nc https://github.com/Kitware/CMake/releases/download/v3.23.0-rc5/cmake-3.23.0-rc5-linux-x86_64.tar.gz

# Install cmake
[ ! -d "cmake-3.23.0-rc5-linux-x86_64" ] && tar -xvf cmake-3.23.0-rc5-linux-x86_64.tar.gz
sudo cp -r cmake-3.23.0-rc5-linux-x86_64/* /usr 
PATH=/usr/:$PATH

# Install Other Packages for Trinity
sudo apt-get install -y libboost-test-dev  
sudo apt-get install -y  libboost-all-dev
sudo apt-get install -y  curl
sudo apt-get install -y  libssl-dev libcurl4-openssl-dev
sudo apt install -y libboost-thread-dev
sudo apt-get install -y libbz2-dev
sudo apt-get install -y python3-dev  # for python3.x installs
sudo apt-get install -y libevent-dev

#libevent
wget -nc  https://github.com/libevent/libevent/releases/download/release-2.1.10-stable/libevent-2.1.10-stable.tar.gz
[ ! -d "libevent-2.1.10-stable" ] && tar -xvf libevent-2.1.10-stable.tar.gz
cd libevent-2.1.10-stable
sudo ./configure --prefix=/usr 
sudo make
sudo make install
cd /mntData2/

# Install thrift
if [ ! -d "thrift"]; then
    git clone -b 0.15.0 https://github.com/apache/thrift.git
    cd thrift
    ./bootstrap.sh
    sudo ./configure
    sudo make
fi
cd /mntData2/thrift
sudo make install
cd /mntData2/

# Other Config / installation
sudo apt install -y python3-pip
pip3 install pandas
echo 'include /etc/ld.so.conf.d/*.conf /usr/local/lib' | sudo tee /etc/ld.so.conf
sudo /sbin/ldconfig
ulimit -n 16384

# Installing CLickhouse DB
sudo apt-get install -y apt-transport-https ca-certificates dirmngr
sudo apt-key adv --keyserver hkp://keyserver.ubuntu.com:80 --recv 8919F6BD2B48D754
echo "deb https://packages.clickhouse.com/deb stable main" | sudo tee /etc/apt/sources.list.d/clickhouse.list
sudo apt-get update
sudo DEBIAN_FRONTEND=noninteractive apt-get install -y clickhouse-server 
sudo apt-get install -y clickhouse-client
sudo timedatectl set-timezone America/New_York # Set time zone

# Configure ClickHouse
sudo cp /mntData2/md-trie/scripts/clickhouse_config.xml /etc/clickhouse-server/config.xml

# Start clickhouse
sudo service clickhouse-server start

# Set up Timescale DB
sudo dpkg --configure -a
sudo apt install -y gnupg postgresql-common apt-transport-https lsb-release wget
printf '\n' | sudo /usr/share/postgresql-common/pgdg/apt.postgresql.org.sh
curl -L https://packagecloud.io/timescale/timescaledb/gpgkey | sudo apt-key add -
sudo sh -c "echo 'deb https://packagecloud.io/timescale/timescaledb/ubuntu/ $(lsb_release -c -s) main' > /etc/apt/sources.list.d/timescaledb.list"
wget --quiet -O - https://packagecloud.io/timescale/timescaledb/gpgkey | sudo apt-key add -
sudo apt update
sudo apt install -y timescaledb-2-postgresql-14

# Set up psql stuff
sudo cp /mntData2/md-trie/scripts/postgresql.conf /etc/postgresql/14/main/postgresql.conf 
sudo cp /mntData2/md-trie/scripts/pg_hba.conf /etc/postgresql/14/main/pg_hba.conf
mkdir -p /mntData2/postgresql/14/main
sudo chown -R postgres:postgres /mntData2/postgresql/14/main
sudo -u postgres /usr/lib/postgresql/14/bin/initdb -D /mntData2/postgresql/14/main
sudo service postgresql stop
sudo service postgresql start

# No need whatever later
exit 0

# TimescaleDB stuff
sudo su - postgres
psql
SELECT pg_reload_conf();

# Create User
CREATE ROLE "Ziming";
ALTER ROLE "Ziming" WITH LOGIN;
ALTER USER "Ziming" with superuser;
\du;

# Create db and initialize timescaledb
CREATE database example;
\c example
CREATE EXTENSION IF NOT EXISTS timescaledb;

# Change Password
# https://stackoverflow.com/questions/14035742/pgadmin-gives-me-the-error-no-password-supplied
sudo -u postgres psql postgres
alter user postgres with password 'postgres';

# Distributed ClickHouse
# https://dev.to/zergon321/creating-a-clickhouse-cluster-part-i-sharding-4j20
CREATE TABLE IF NOT EXISTS distributed_test(
                          timestamp DateTime,
                          parameter String,
                          value Float64)
                          ENGINE = MergeTree()
                          PARTITION BY parameter
                          ORDER BY (timestamp, parameter)

CREATE TABLE IF NOT EXISTS distributed_test(
                      timestamp DateTime,
                      parameter String,
                      value Float64)
                      ENGINE = Distributed(test_trinity, default, distributed_test, rand())

INSERT INTO distributed_test (*) VALUES (toDateTime(1594806134000), 'elasticity', 38.9);
INSERT INTO distributed_test (*) VALUES (toDateTime(1594806134), 'gravity', 27.2);
INSERT INTO distributed_test (*) VALUES (toDateTime(15948061340), 'density', 19.8);

# Distributed Timescale DB
# https://docs.timescale.com/timescaledb/latest/how-to-guides/multinode-timescaledb/multinode-setup/#set-up-multi-node-on-self-hosted-timescaledb
CREATE database distributed_example;
\c distributed_example
CREATE EXTENSION IF NOT EXISTS timescaledb;

SELECT add_data_node('dn1', host => '128.110.219.152');
SELECT add_data_node('dn2', host => '128.110.219.148');

CREATE TABLE conditions_distributed (
   time        TIMESTAMPTZ       NOT NULL,
   location    TEXT              NOT NULL
);

SELECT create_distributed_hypertable('conditions_distributed', 'time', 'location',
    data_nodes => '{ "dn1", "dn2"}');
INSERT INTO conditions_distributed VALUES ('2020-12-14 13:45', 1, '1.2.3.4');

