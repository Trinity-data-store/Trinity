cd /mntData2
sudo rm *.tar.gz
sudo mkdir dependencies
sudo mv cmake-3.23.0-rc5-linux-x86_64 dependencies
sudo mv libevent-2.1.10-stable dependencies
sudo mv thrift dependencies

sudo rm -r md-trie
git clone https://github.com/MaoZiming/Trinity.git


# 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

sudo bash /mntData2/Trinity/scripts/initialize_node_scratch.sh

sudo bash /proj/trinity-PG0/Trinity/scripts/initialize_node_scratch.sh



wget -nc https://github.com/Kitware/CMake/releases/download/v3.23.0-rc5/cmake-3.23.0-rc5-linux-x86_64.tar.gz
tar -xvf cmake-3.23.0-rc5-linux-x86_64.tar.gz
wget -nc  https://github.com/libevent/libevent/releases/download/release-2.1.10-stable/libevent-2.1.10-stable.tar.gz
tar -xvf libevent-2.1.10-stable.tar.gz
cd libevent-2.1.10-stable
sudo ./configure --prefix=/usr 
sudo make -j
cd ..
git clone -b 0.15.0 https://github.com/apache/thrift.git
cd thrift
./bootstrap.sh
sudo ./configure
sudo make -j
cd ..