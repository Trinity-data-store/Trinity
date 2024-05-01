#!/bin/bash
set -Eeuo pipefail

# Run from Cloudlab
# git clone https://github.com/MaoZiming/Trinity.git
# bash /proj/trinity-PG0/Trinity/scripts/setup_one_node.sh

# Path
dependencies_path="/proj/trinity-PG0/dependencies"

# Basic setup
sudo apt update
sudo apt install -y htop
sudo apt install -y dstat
sudo apt install -y pkgconf  # making thrift requires this
sudo apt install -y cmake
sudo apt install -y build-essential
sudo apt install -y libboost-test-dev  
sudo apt install -y  libboost-all-dev
sudo apt install -y  libssl-dev libcurl4-openssl-dev
sudo apt install -y libboost-thread-dev
sudo apt install -y libbz2-dev
sudo apt install -y libevent-dev
sudo apt install -y clang-format
sudo apt install -y flex bison
sudo apt install -y npm
curl -sL https://deb.nodesource.com/setup_18.x | sudo -E bash -
sudo apt install -y nodejs            
sudo apt install -y ant 
sudo apt install -y libatk1.0-dev libatk-bridge2.0-dev
sudo apt-get install libx11-xcb1 libxcomposite1 libxcursor1 libxdamage1 libxi6 libxtst6 libnss3 libcups2 libxss1 libxrandr2 libgconf-2-4 libasound2 libpango1.0-0 libatk1.0-0 libatk-bridge2.0-0 libgtk-3-0 libgdk-pixbuf2.0-0 libpangocairo-1.0-0

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

