sudo pkill -f ndbd
sudo rm -r /usr/local/mysql/data
sudo mkdir -p /usr/local/mysql/data
sudo ndbd
sudo cp ndbd.service /etc/systemd/system/ndbd.service
sudo systemctl daemon-reload
sudo systemctl enable ndbd
sudo systemctl start ndbd
sudo systemctl status ndbd