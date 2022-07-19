for i in {1..14}
do
    ssh -o StrictHostKeyChecking=no -i /proj/trinity-PG0/Trinity/scripts/key -l Ziming 10.10.1.$(($i + 2)) "sudo rm -r /etc/security/limits.d/ && sudo cp /proj/trinity-PG0/Trinity/scripts/limits.conf  /etc/security/limits.conf && sudo cp /proj/trinity-PG0/Trinity/scripts/user.conf /etc/systemd/user.conf && sudo cp /proj/trinity-PG0/Trinity/scripts/system.conf  /etc/security/system.conf" &
done

sudo rm -r /etc/security/limits.d/
sudo cp /proj/trinity-PG0/Trinity/scripts/limits.conf /etc/security/limits.conf
sudo cp /proj/trinity-PG0/Trinity/scripts/user.conf /etc/systemd/user.conf