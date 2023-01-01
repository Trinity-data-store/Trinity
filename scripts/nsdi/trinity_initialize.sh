for i in {1..14}
do
    ssh -o StrictHostKeyChecking=no -i /proj/trinity-PG0/Trinity/scripts/key -l Ziming 10.10.1.$(($i + 2)) "sudo rm -r /etc/security/limits.d/ && sudo cp /proj/trinity-PG0/Trinity/scripts/limits.conf  /etc/security/limits.conf && sudo cp /proj/trinity-PG0/Trinity/scripts/user.conf /etc/systemd/user.conf && sudo cp /proj/trinity-PG0/Trinity/scripts/system.conf  /etc/security/system.conf" &
    ssh -o StrictHostKeyChecking=no -i /proj/trinity-PG0/Trinity/scripts/key -l Ziming 10.10.1.$(($i + 2)) "cd /proj/trinity-PG0/dependencies/thrift && sudo make install && echo 'include /etc/ld.so.conf.d/*.conf /usr/local/lib' | sudo tee /etc/ld.so.conf && sudo /sbin/ldconfig" &
done

sudo rm -r /etc/security/limits.d/
sudo cp /proj/trinity-PG0/Trinity/scripts/limits.conf /etc/security/limits.conf
sudo cp /proj/trinity-PG0/Trinity/scripts/user.conf /etc/systemd/user.conf
cd /proj/trinity-PG0/dependencies/thrift
sudo make install
echo 'include /etc/ld.so.conf.d/*.conf /usr/local/lib' | sudo tee /etc/ld.so.conf
sudo /sbin/ldconfig