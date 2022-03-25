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
sudo apt-get install -y libevent-dev

# Install Thrift
# wget https://dlcdn.apache.org/thrift/0.16.0/thrift-0.16.0.tar.gz
# tar -xf thrift-0.16.0.tar.gz
# cd thrift-0.16.0/
# ./bootstrap.sh
# ./configure
# make
# sudo make install

cd ~/
wget https://github.com/libevent/libevent/releases/download/release-2.1.10-stable/libevent-2.1.10-stable.tar.gz
tar xfz libevent-2.1.10-stable.tar.gz
cd libevent-2.1.10-stable
sudo ./configure --prefix=/usr 
sudo make
sudo make install
# libtool --finish /usr/local/libevent/lib
cd ~/

git clone -b 0.15.0 https://github.com/apache/thrift.git
cd thrift
./bootstrap.sh
sudo ./configure
sudo make
sudo make install
cd ~/

# cmake -DGENERATE_THRIFT=on ..