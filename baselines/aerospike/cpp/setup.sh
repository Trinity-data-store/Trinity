
cd /mntData
sudo apt-get install libssl-dev
sudo wget https://download.aerospike.com/artifacts/aerospike-client-c/6.1.0/aerospike-client-c-libev-6.1.0.ubuntu18.04.x86_64.tgz
sudo tar xvf aerospike-client-c-libev-6.1.0.ubuntu18.04.x86_64.tgz
cd aerospike-client-c-libev-6.1.0.ubuntu18.04.x86_64
sudo dpkg -i aerospike-client-c-libev-6.1.0.ubuntu18.04.x86_64.deb
sudo dpkg -i aerospike-client-c-libev-devel-6.1.0.ubuntu18.04.x86_64.deb
cd ..
sudo wget http://dist.schmorp.de/libev/libev-4.33.tar.gz
sudo tar xvf libev-4.33.tar.gz
cd libev-4.33
sudo ./configure
sudo make
sudo make install
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/local/lib
sudo apt-get install -y libc6-dev libssl-dev autoconf automake libtool g++ zlib1g-dev
sudo apt-get install -y ncurses-dev


exit(0)
# cd /mntData/libev-4.33
# sudo make uninstall
