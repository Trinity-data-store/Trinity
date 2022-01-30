sudo pkill -f ndbd
sudo cp my.cnf /etc/my.cnf
sudo rm -r /usr/local/mysql/data
sudo mkdir -p /usr/local/mysql/data
# sudo cp /usr/share/doc/util-linux/examples/securetty /etc/securetty
sudo ndbd
sudo pkill -f ndbd
# sudo cp ndbd.service /etc/systemd/system/ndbd.service
sudo systemctl daemon-reload
sudo systemctl enable ndbd
sudo systemctl start ndbd
sudo systemctl status ndbd


# sudo systemctl status ndbd kill mongod