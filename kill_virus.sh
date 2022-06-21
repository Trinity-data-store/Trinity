while :
do
	sudo pkill -9 threaddk
done

sudo pkill -9 threaddk
sudo crontab -u postgres -r
sudo service postgresql restart

sudo crontab -u postgres -r
sudo pkill -u postgres
sudo service postgresql stop