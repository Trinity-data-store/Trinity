#!/bin/sh

# Run from Cloudlab
# git clone https://github.com/MaoZiming/Trinity.git
# sh ~/Trinity/scripts/initialize_node_scratch.sh

# Update Github
cd /proj/trinity-PG0/Trinity/
git config --global user.name "MaoZiming"
git config --global user.email "ziming.mao@yale.edu"
git pull origin main

# Basic setup
sudo apt update
sudo apt install htop

cd ~/

# Install Cmake
wget -nc https://github.com/Kitware/CMake/releases/download/v3.23.0-rc5/cmake-3.23.0-rc5-linux-x86_64.tar.gz
[ ! -d "cmake-3.23.0-rc5-linux-x86_64" ] && tar -xvf cmake-3.23.0-rc5-linux-x86_64.tar.gz
sudo cp -r cmake-3.23.0-rc5-linux-x86_64/* /usr 
PATH=/usr/:$PATH
cd ~/

# Install libevent
wget -nc  https://github.com/libevent/libevent/releases/download/release-2.1.10-stable/libevent-2.1.10-stable.tar.gz
[ ! -d "libevent-2.1.10-stable" ] && tar -xvf libevent-2.1.10-stable.tar.gz
cd libevent-2.1.10-stable
sudo ./configure --prefix=/usr 
sudo make
sudo make install
cd ~/

# Install thrift
git clone -b 0.15.0 https://github.com/apache/thrift.git
cd thrift
./bootstrap.sh
sudo ./configure
sudo make
sudo make install
cd ~/

# Install Other Packages for Trinity
sudo apt-get install -y libboost-test-dev  
sudo apt-get install -y  libboost-all-dev
sudo apt-get install -y  curl
sudo apt-get install -y  libssl-dev libcurl4-openssl-dev
sudo apt install -y libboost-thread-dev
sudo apt-get install -y libbz2-dev
sudo apt-get install -y python3-dev  # for python3.x installs
sudo apt-get install -y libevent-dev

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

# Configure clickhouse DB
sudo cp ~/Trinity/scripts/clickhouse_config.xml /etc/clickhouse-server/config.xml
sudo cp ~/Trinity/scripts/clickhouse_users.xml /etc/clickhouse-server/users.xml

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
sudo cp ~/Trinity/scripts/postgresql.conf /etc/postgresql/14/main/postgresql.conf 
sudo cp ~/Trinity/scripts/pg_hba.conf /etc/postgresql/14/main/pg_hba.conf
mkdir -p ~/postgresql/14/main
sudo chown -R postgres:postgres ~/postgresql/14/main
sudo -u postgres /usr/lib/postgresql/14/bin/initdb -D ~/postgresql/14/main

exit 0

# Start clickhouse
sudo service clickhouse-server start

# Start postgresql
sudo service postgresql start