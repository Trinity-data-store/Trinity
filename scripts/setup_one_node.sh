#!/bin/bash
set -Eeuo pipefail

# Run from Cloudlab
# git clone https://github.com/MaoZiming/Trinity.git
# bash /proj/trinity-PG0/Trinity/scripts/setup_one_node.sh

# Path
dependencies_path="/proj/trinity-PG0/dependencies"

# Basic setup
sudo apt update
# sudo apt upgrade
sudo apt install -y htop
sudo apt install -y dstat
sudo apt install -y pkgconf  # making thrift requires this
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
if [ ! -d "thrift" ]; then
    wget -nc https://dlcdn.apache.org/thrift/0.20.0/thrift-0.20.0.tar.gz
    tar -xvf thrift-0.20.0.tar.gz
    mv thrift-0.20.0 thrift
    cd thrift
    ./bootstrap.sh
    sudo ./configure --without-erlang
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
sudo apt-get install -y clang-format
sudo apt-get install -y flex bison
