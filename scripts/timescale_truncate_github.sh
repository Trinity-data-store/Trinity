export PGPASSWORD=adifficultpassword && psql -h localhost -d defaultdb -U postgres -p 5432 -c "DROP TABLE github_events"
export PGPASSWORD=adifficultpassword && psql -h localhost -d defaultdb -U postgres -p 5432 -c "CREATE TABLE github_events (
    pkey           BIGINT             NOT NULL,
    events_count     INT             NOT NULL,
    authors_count     INT             NOT NULL,
    forks     INT             NOT NULL,
    stars     INT             NOT NULL,
    issues     INT             NOT NULL,
    pushes     INT             NOT NULL,
    pulls     INT             NOT NULL,
    downloads     INT             NOT NULL,
    start_date   TIMESTAMP NOT NULL,
    end_date TIMESTAMP NOT NULL,
    CONSTRAINT id_pk_g PRIMARY KEY (pkey)
);"

export PGPASSWORD=adifficultpassword && psql -h localhost -d defaultdb -U postgres -p 5432 -c "CREATE INDEX ON github_events (stars, start_date DESC);"
export PGPASSWORD=adifficultpassword && psql -h localhost -d defaultdb -U postgres -p 5432 -c "CREATE INDEX ON github_events (forks, start_date DESC);"
export PGPASSWORD=adifficultpassword && psql -h localhost -d defaultdb -U postgres -p 5432 -c "CREATE INDEX ON github_events (events_count, start_date DESC);"
export PGPASSWORD=adifficultpassword && psql -h localhost -d defaultdb -U postgres -p 5432 -c "CREATE INDEX ON github_events (issues, start_date DESC);"
export PGPASSWORD=adifficultpassword && psql -h localhost -d defaultdb -U postgres -p 5432 -c "CREATE INDEX ON github_events (end_date, start_date DESC);"