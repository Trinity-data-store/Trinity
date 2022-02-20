Follow: https://www.bmc.com/blogs/how-to-setup-mongodb-cluster/  

Check port in use:
sudo lsof -i -P -n | grep LISTEN

Config Server:
Start the process: sudo mongod --config /etc/mongodConfig.conf&
mongo 172.29.249.44:27019
> rs.initiate()

Configure Query Router:
sudo mongos --config /etc/mongoRouter.conf&

Start Data Shard:
sudo mongod --config /etc/mongodShard.conf&
sudo tail -f /var/log/mongodb/mongodShard.log
