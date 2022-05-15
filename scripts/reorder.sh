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