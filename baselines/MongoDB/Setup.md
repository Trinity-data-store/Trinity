Follow: https://www.bmc.com/blogs/how-to-setup-mongodb-cluster/  

Check port in use:
sudo lsof -i -P -n | grep LISTEN

Config Server:
Start the process: sudo mongod --config /etc/mongodConfig.conf&
mongo 172.29.249.30:27019
> rs.initiate()

Configure Query Router:
sudo mongos --config /etc/mongoRouter.conf&
mongo 172.29.249.49:27017

Start Data Shard:
sudo mongod --config /etc/mongodShard.conf&
sudo tail -f /var/log/mongodb/mongodShard.log

Useful MongoShell command:
```
mongo 172.29.249.30:27017
db.adminCommand({ listShards: 1 })
sh.addShard( "rs2/172.28.229.153:27018")
sh.addShard( "rs1/172.28.229.152:27018")
sh.addShard( "rs3/172.28.229.151:27018")
sh.addShard( "rs5/172.28.229.148:27018")
sh.addShard( "rs4/172.28.229.149:27018")
sh.status()
db.test.drop()
db.createCollection("test")
db.test.createIndex({_id:"hashed"})
sh.shardCollection("test.test", {_id:"hashed"})
db.test.getShardDistribution()
```