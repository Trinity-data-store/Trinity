
echo "aerospike clear db for new experiment"
bash scripts/aerospike_start.sh
sleep 10

echo "aerospike insertion for tpch"
bash scripts/aerospike_insert_tpch.sh
sleep 10

echo "aerospike lookup for tpch"
bash scripts/aerospike_lookup_tpch.sh
sleep 10

echo "aerospike clear db for new experiment"
bash scripts/aerospike_start.sh
sleep 10

echo "aerospike insertion for github"
bash scripts/aerospike_insert_github.sh
sleep 10

echo "aerospike lookup for github"
bash scripts/aerospike_lookup_github.sh
sleep 10

echo "aerospike clear db for new experiment"
bash scripts/aerospike_start.sh
sleep 10

echo "aerospike insertion for nyc"
bash scripts/aerospike_insert_nyc.sh
sleep 10

echo "aerospike lookup for nyc"
bash scripts/aerospike_lookup_nyc.sh
sleep 10

echo "aerospike shutdown for nyc"
bash scripts/aerospike_shutdown.sh
