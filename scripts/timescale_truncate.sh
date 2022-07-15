export PGPASSWORD=adifficultpassword && psql -h localhost -d defaultdb -U postgres -p 5432 -c "TRUNCATE TABLE tpch_macro"

export PGPASSWORD=adifficultpassword && psql -h localhost -d defaultdb -U postgres -p 5432 -c "CREATE INDEX ON tpch_macro (quantity, shipdate DESC);"
export PGPASSWORD=adifficultpassword && psql -h localhost -d defaultdb -U postgres -p 5432 -c "CREATE INDEX ON tpch_macro (discount, shipdate DESC);"
export PGPASSWORD=adifficultpassword && psql -h localhost -d defaultdb -U postgres -p 5432 -c "CREATE INDEX ON tpch_macro (commitdate, shipdate DESC);"
export PGPASSWORD=adifficultpassword && psql -h localhost -d defaultdb -U postgres -p 5432 -c "CREATE INDEX ON tpch_macro (receiptdate, shipdate DESC);"
export PGPASSWORD=adifficultpassword && psql -h localhost -d defaultdb -U postgres -p 5432 -c "CREATE INDEX ON tpch_macro (orderdate, shipdate DESC);
"