#!/bin/bash
set -Eeuo pipefail

sudo apt update
sudo apt install -y htop
sudo apt install -y dstat
sudo apt install -y pkgconf
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
