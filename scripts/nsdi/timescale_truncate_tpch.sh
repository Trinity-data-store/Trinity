export PGPASSWORD=adifficultpassword && psql -h localhost -d defaultdb -U postgres -p 5432 -c "DROP TABLE tpch_macro"

export PGPASSWORD=adifficultpassword && psql -h localhost -d defaultdb -U postgres -p 5432 -c "CREATE TABLE tpch_macro (
   ID           BIGINT             NOT NULL,
   QUANTITY     SMALLINT             NOT NULL,
    EXTENDEDPRICE  INT       NOT NULL,
    DISCOUNT    SMALLINT    NOT NULL,
    TAX SMALLINT    NOT NULL,
    SHIPDATE   TIMESTAMP NOT NULL,
    COMMITDATE TIMESTAMP NOT NULL,
    RECEIPTDATE TIMESTAMP NOT NULL,
    TOTALPRICE  INT NOT NULL,
    ORDERDATE   TIMESTAMP NOT NULL,
    CONSTRAINT id_pk PRIMARY KEY (ID)
);"

export PGPASSWORD=adifficultpassword && psql -h localhost -d defaultdb -U postgres -p 5432 -c "CREATE INDEX ON tpch_macro (quantity, shipdate DESC);"
export PGPASSWORD=adifficultpassword && psql -h localhost -d defaultdb -U postgres -p 5432 -c "CREATE INDEX ON tpch_macro (discount, shipdate DESC);"
export PGPASSWORD=adifficultpassword && psql -h localhost -d defaultdb -U postgres -p 5432 -c "CREATE INDEX ON tpch_macro (commitdate, shipdate DESC);"
export PGPASSWORD=adifficultpassword && psql -h localhost -d defaultdb -U postgres -p 5432 -c "CREATE INDEX ON tpch_macro (receiptdate, shipdate DESC);"
export PGPASSWORD=adifficultpassword && psql -h localhost -d defaultdb -U postgres -p 5432 -c "CREATE INDEX ON tpch_macro (orderdate, shipdate DESC);
"