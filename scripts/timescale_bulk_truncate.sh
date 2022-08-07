for i in {10..14}
do
    ssh -o StrictHostKeyChecking=no -i /proj/trinity-PG0/Trinity/scripts/key -l Ziming 10.10.1.$(($i + 2)) 'bash /proj/trinity-PG0/Trinity/scripts/timescale_truncate_github.sh'
done