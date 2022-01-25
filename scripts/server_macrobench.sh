cd build/

if [ $(cat /proc/sys/kernel/hostname) = "ecl-comp-data" ]
then
    ip="172.29.249.44"
fi

if [ $(cat /proc/sys/kernel/hostname) = "vmhost4" ]
then
    ip="172.28.229.152"
fi
if [ $(cat /proc/sys/kernel/hostname) = "vmhost3" ]
then
    ip="172.28.229.151"
fi
if [ $(cat /proc/sys/kernel/hostname) = "vmhost5" ]
then
    ip="172.28.229.153"
fi
if [ $(cat /proc/sys/kernel/hostname) = "vmhost1" ]
then
    ip="172.28.229.148"
fi
if [ $(cat /proc/sys/kernel/hostname) = "vmhost1" ]
then
    ip="172.28.229.149"
fi

librpc/MDTrieShardServer $ip 20
while [ $? -e 42 ]; do
    librpc/MDTrieShardServer $ip 20
done