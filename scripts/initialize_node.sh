#!/bin/bash

# Run from Cloudlab
# git clone https://github.com/MaoZiming/Trinity.git
# bash /proj/Trinity/scripts/initialize_node_scratch.sh

# Update Github
git config --global user.name "MaoZiming"
git config --global user.email "ziming.mao@yale.edu"

# Path
trinity_path="/proj/trinity-PG0/Trinity"
dependencies_path="/proj/trinity-PG0/dependencies"
local_path="/mntData"
data_dir="/mntData2"

cd "$trinity_path"
git pull origin main

# Basic setup
sudo apt update
sudo apt install htop
sudo apt-get install dstat
cd $dependencies_path

# Install Cmake
if [ ! -d "cmake-3.23.0-rc5-linux-x86_64" ]; then
    wget -nc https://github.com/Kitware/CMake/releases/download/v3.23.0-rc5/cmake-3.23.0-rc5-linux-x86_64.tar.gz
    tar -xvf cmake-3.23.0-rc5-linux-x86_64.tar.gz
fi
sudo cp -r cmake-3.23.0-rc5-linux-x86_64/* /usr 
PATH=/usr/:$PATH
cd $dependencies_path

# Install libevent
if [ ! -d "libevent-2.1.10-stable" ]; then
    wget -nc  https://github.com/libevent/libevent/releases/download/release-2.1.10-stable/libevent-2.1.10-stable.tar.gz
    tar -xvf libevent-2.1.10-stable.tar.gz
    cd libevent-2.1.10-stable
    sudo ./configure --prefix=/usr 
    sudo make -j
    cd ..
fi
cd libevent-2.1.10-stable
sudo make install
cd $dependencies_path

# Install thrift
if [ ! -d "thrift"]; then
    git clone -b 0.15.0 https://github.com/apache/thrift.git
    cd thrift
    ./bootstrap.sh
    sudo ./configure
    sudo make -j
    cd ..
fi
cd thrift
sudo make install
cd $dependencies_path

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

# Golang
cd $dependencies_path
cd go
if [ ! -d "go1.18.3.linux-amd64.tar.gz" ]; then
    wget https://go.dev/dl/go1.18.3.linux-amd64.tar.gz
fi
sudo tar -C /usr/bin -xzf go1.18.3.linux-amd64.tar.gz
export PATH=$PATH:/usr/bin/go/bin
cd $dependencies_path
ulimit -n 16384
sudo apt-get install maven


sudo apt-get install -y python3-pip
exit 0
