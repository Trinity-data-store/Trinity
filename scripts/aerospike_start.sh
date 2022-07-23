for i in {10..14}
do
    # ssh -o StrictHostKeyChecking=no -i /proj/trinity-PG0/Trinity/scripts/key -l Ziming 10.10.1.$(($i + 2)) "sudo systemctl stop aerospike && sudo rm /mntData/aerospike/* && sudo systemctl start aerospike"
    ssh -o StrictHostKeyChecking=no -i /proj/trinity-PG0/Trinity/scripts/key -l Ziming 10.10.1.$(($i + 2)) "sudo systemctl stop aerospike && sudo systemctl start aerospike"

done