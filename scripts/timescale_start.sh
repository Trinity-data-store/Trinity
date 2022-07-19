for i in {1..14}
do
    ssh -o StrictHostKeyChecking=no -i /proj/trinity-PG0/Trinity/scripts/key -l Ziming 10.10.1.$(($i + 2)) "sudo cp /proj/trinity-PG0/Trinity/baselines/timescaleDB/postgresql.conf /etc/postgresql/14/main/postgresql.conf && sudo cp /proj/trinity-PG0/Trinity/baselines/timescaleDB/pg_hba.conf /etc/postgresql/14/main/pg_hba.conf && sudo cp /proj/trinity-PG0/Trinity/baselines/timescaleDB/sysctl.conf /etc/sysctl.conf && sudo service postgresql start" &
done

sudo cp /proj/trinity-PG0/Trinity/baselines/timescaleDB/postgresql.conf /etc/postgresql/14/main/postgresql.conf
sudo cp /proj/trinity-PG0/Trinity/baselines/timescaleDB/pg_hba.conf /etc/postgresql/14/main/pg_hba.conf
sudo cp /proj/trinity-PG0/Trinity/baselines/timescaleDB/sysctl.conf /etc/sysctl.conf
sudo service postgresql restart