export PGPASSWORD=adifficultpassword && psql -h localhost -d defaultdb -U postgres -p 5432 -c "TRUNCATE TABLE tpch_macro"