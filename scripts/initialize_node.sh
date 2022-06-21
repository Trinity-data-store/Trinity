#!/usr/bin/bash 

# Other configuration
cd /mntData2/md-trie
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
pip3 install clickhouse-driver[lz4]
sudo apt-get install -y apt-transport-https ca-certificates dirmngr
sudo apt-key adv --keyserver hkp://keyserver.ubuntu.com:80 --recv 8919F6BD2B48D754
echo "deb https://packages.clickhouse.com/deb stable main" | sudo tee /etc/apt/sources.list.d/clickhouse.list
sudo apt-get update
sudo DEBIAN_FRONTEND=noninteractive apt-get install -y clickhouse-server 
sudo apt-get install -y clickhouse-client
sudo timedatectl set-timezone America/New_York # Set time zone

# Configure ClickHouse
sudo cp /mntData2/md-trie/scripts/clickhouse_config.xml /etc/clickhouse-server/config.xml
sudo cp /mntData2/md-trie/scripts/clickhouse_users.xml /etc/clickhouse-server/users.xml

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
# sudo service postgresql stop
# sudo service postgresql start

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

# ClickHouse Table Size command:

SELECT
    database,
    table,
    formatReadableSize(sum(data_compressed_bytes) AS size) AS compressed,
    formatReadableSize(sum(data_uncompressed_bytes) AS usize) AS uncompressed,
    round(usize / size, 2) AS compr_rate,
    sum(rows) AS rows,
    count() AS part_count
FROM system.parts
WHERE (active = 1) AND (table LIKE '%') AND (database LIKE '%')
GROUP BY
    database,
    table
ORDER BY size DESC;

# CLIENT
CREATE TABLE IF NOT EXISTS distributed_test(
                          timestamp DateTime,
                          parameter String,
                          value Float64)
                          ENGINE = MergeTree()
                          PARTITION BY parameter
                          ORDER BY (timestamp, parameter)

# MASTER
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

SELECT add_data_node('dn1', host => '10.254.254.253');
SELECT add_data_node('dn2', host => '10.254.254.229');

CREATE TABLE conditions_distributed (
   time        TIMESTAMPTZ       NOT NULL,
   location    TEXT              NOT NULL
);

SELECT create_distributed_hypertable('conditions_distributed', 'time', 'location',
    data_nodes => '{ "dn1", "dn2"}');
INSERT INTO conditions_distributed VALUES ('2020-12-14 13:45', 1, '1.2.3.4');

# Github Events (ClickHouse):

CREATE TABLE github_events_distributed
(
    file_time DateTime,
    event_type Enum('CommitCommentEvent' = 1, 'CreateEvent' = 2, 'DeleteEvent' = 3, 'ForkEvent' = 4,
                    'GollumEvent' = 5, 'IssueCommentEvent' = 6, 'IssuesEvent' = 7, 'MemberEvent' = 8,
                    'PublicEvent' = 9, 'PullRequestEvent' = 10, 'PullRequestReviewCommentEvent' = 11,
                    'PushEvent' = 12, 'ReleaseEvent' = 13, 'SponsorshipEvent' = 14, 'WatchEvent' = 15,
                    'GistEvent' = 16, 'FollowEvent' = 17, 'DownloadEvent' = 18, 'PullRequestReviewEvent' = 19,
                    'ForkApplyEvent' = 20, 'Event' = 21, 'TeamAddEvent' = 22),
    created_at DateTime,
    updated_at DateTime,
    action Enum('none' = 0, 'created' = 1, 'added' = 2, 'edited' = 3, 'deleted' = 4, 'opened' = 5, 'closed' = 6, 'reopened' = 7, 'assigned' = 8, 'unassigned' = 9,
                'labeled' = 10, 'unlabeled' = 11, 'review_requested' = 12, 'review_request_removed' = 13, 'synchronize' = 14, 'started' = 15, 'published' = 16, 'update' = 17, 'create' = 18, 'fork' = 19, 'merged' = 20),
    comment_id UInt64,
    position Int32,
    line Int32,
    ref_type Enum('none' = 0, 'branch' = 1, 'tag' = 2, 'repository' = 3, 'unknown' = 4),
    number UInt32,
    state Enum('none' = 0, 'open' = 1, 'closed' = 2),
    locked UInt8,
    comments UInt32,
    author_association Enum('NONE' = 0, 'CONTRIBUTOR' = 1, 'OWNER' = 2, 'COLLABORATOR' = 3, 'MEMBER' = 4, 'MANNEQUIN' = 5),
    closed_at DateTime,
    merged_at DateTime,
    merged UInt8,
    mergeable UInt8,
    rebaseable UInt8,
    mergeable_state Enum('unknown' = 0, 'dirty' = 1, 'clean' = 2, 'unstable' = 3, 'draft' = 4),
    review_comments UInt32,
    maintainer_can_modify UInt8,
    commits UInt32,
    additions UInt32,
    deletions UInt32,
    changed_files UInt32,
    original_position UInt32,
    push_size UInt32,
    push_distinct_size UInt32,
    review_state Enum('none' = 0, 'approved' = 1, 'changes_requested' = 2, 'commented' = 3, 'dismissed' = 4, 'pending' = 5)
)
# ENGINE = MergeTree  # For data nodes
ENGINE = Distributed(test_trinity, default, github_events_distributed, rand())  # For Server Nodes
# ORDER BY (event_type, created_at)  # For data nodes

# Insert into table (ClickHouse)
INSERT INTO github_events_distributed SELECT * FROM github_events;

# TPC-H

DROP TABLE IF EXISTS tpch_distributed;

CREATE TABLE IF NOT EXISTS tpch_distributed (
    ID UInt32,
    QUANTITY UInt8,
    EXTENDEDPRICE UInt32,
    DISCOUNT UInt8,
    TAX UInt8,
    SHIPDATE UInt32,
    COMMITDATE UInt32,
    RECEIPTDATE UInt32,
    TOTALPRICE UInt32,
    ORDERDATE UInt32
# ) Engine = MergeTree ORDER BY (ID)
) ENGINE = Distributed(test_trinity, default, tpch_distributed, rand())  # For Server Nodes

cat /mntData2/tpch-dbgen/data_500/orders_lineitem_merged_indexed.csv | clickhouse-client --query="INSERT INTO tpch_distributed FORMAT CSV";

# cat /mntData2/tpch-dbgen/data_200/orders_lineitem_merged_by_chunk_indexed.csv | clickhouse-client --query="INSERT INTO tpch_distributed SELECT col1, toInt32(col2 * 100), toInt32(col3 * 100), toInt32(col4 * 100), col5, col6, col7, toInt32(col8 * 100), col9 FROM input('col1 UInt8, col2 Float32, col3 Float32, col4 Float32, col5 UInt32, col6 UInt32, col7 UInt32, col8 Float32, col9 UInt32')   FORMAT CSVWithNames";

# tail -n +2 /mntData2/tpch-dbgen/data_200/orders_lineitem_merged_by_chunk_5.csv | clickhouse-client --query="INSERT INTO tpch_distributed SELECT col1, toInt32(col2 * 100), toInt32(col3 * 100), toInt32(col4 * 100), col5, col6, col7, toInt32(col8 * 100), col9 FROM input('col1 UInt8, col2 Decimal32, col3 Decimal32, col4 Decimal32, col5 UInt32, col6 UInt32, col7 UInt32, col8 Decimal32, col9 UInt32')   FORMAT CSV";

SELECT * 
FROM tpch_distributed
WHERE SHIPDATE BETWEEN 19940101 AND 19950101  
AND DISCOUNT BETWEEN 5 AND 7
AND QUANTITY <= 24;
