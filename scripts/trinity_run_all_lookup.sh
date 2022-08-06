echo "trinity start"
bash scripts/trinity_start.sh
sleep 10

echo "trinity insertion for nyc"
bash scripts/trinity_insert_nyc.sh
sleep 10

echo "trinity lookup for nyc"
bash scripts/trinity_lookup_nyc.sh
sleep 10

echo "trinity shutdown for nyc"
bash scripts/trinity_shutdown.sh

echo "trinity shutdown"
bash scripts/trinity_shutdown.sh
sleep 10

echo "trinity start"
bash scripts/trinity_start.sh
sleep 10

echo "trinity insertion for tpch"
bash scripts/trinity_insert_tpch.sh
sleep 10

echo "trinity lookup for tpch"
bash scripts/trinity_lookup_tpch.sh
sleep 10

echo "trinity shutdown"
bash scripts/trinity_shutdown.sh
sleep 10

echo "trinity start"
bash scripts/trinity_start.sh
sleep 10

echo "trinity insertion for github"
bash scripts/trinity_insert_github.sh
sleep 10

echo "trinity lookup for github"
bash scripts/trinity_lookup_github.sh
sleep 10

echo "trinity shutdown"
bash scripts/trinity_shutdown.sh
sleep 10

