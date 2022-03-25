#!/bin/sh
cd ~/
sudo apt update
wget https://github.com/Kitware/CMake/releases/download/v3.23.0-rc5/cmake-3.23.0-rc5-linux-x86_64.tar.gz

# Install cmake
tar -xzf cmake-3.23.0-rc5-linux-x86_64.tar.gz
sudo cp -r cmake-3.23.0-rc5-linux-x86_64/* /usr 
PATH=/usr/:$PATH

# Install Other Packages
sudo apt-get install -y libboost-test-dev  
sudo apt-get install -y  libboost-all-dev
sudo apt-get install -y  curl
sudo apt-get install -y  libssl-dev libcurl4-openssl-dev
sudo apt install -y libboost-thread-dev
sudo apt-get install -y libbz2-dev
sudo apt-get install -y python3-dev  # for python3.x installs

# Install Thrift
# wget https://dlcdn.apache.org/thrift/0.16.0/thrift-0.16.0.tar.gz
# tar -xf thrift-0.16.0.tar.gz
# cd thrift-0.16.0/
# ./bootstrap.sh
# ./configure
# make
# sudo make install

git clone -b 0.15.0 https://github.com/apache/thrift.git
cd thrift
./bootstrap.sh
./configure
make
sudo make install

cmake -DGENERATE_THRIFT=on ..