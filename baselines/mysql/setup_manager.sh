sudo pkill -f ndb_mgmd
sudo mkdir -p /var/lib/mysql-cluster
sudo cp config.ini /var/lib/mysql-cluster/config.ini
sudo ndb_mgmd -f /var/lib/mysql-cluster/config.ini
sudo pkill -f ndb_mgmd
sudo cp ndb_mgm.service /etc/systemd/system/ndb_mgmd.service
sudo systemctl daemon-reload
sudo systemctl enable ndb_mgmd
sudo systemctl start ndb_mgmd
sudo systemctl status ndb_mgmd