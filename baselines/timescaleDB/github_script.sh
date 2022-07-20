sudo -u postgres psql postgres
alter user postgres with password 'adifficultpassword';

\c defaultdb
DROP TABLE github_events;
CREATE TABLE github_events (
    pkey           BIGINT             NOT NULL,
    events_count     INT             NOT NULL,
    authors_count     INT             NOT NULL,
    forks     INT             NOT NULL,
    stars     INT             NOT NULL,
    issues     INT             NOT NULL,
    pushes     INT             NOT NULL,
    pulls     INT             NOT NULL,
    downloads     INT             NOT NULL,
    adds     INT             NOT NULL,
    dels     INT             NOT NULL,
    add_del_ratio     float8             NOT NULL,
    start_date   TIMESTAMP NOT NULL,
    end_date TIMESTAMP NOT NULL,
    PRIMARY KEY (start_date, pkey)
);

SELECT add_data_node('dn1', host => '10.10.1.12');
SELECT add_data_node('dn2', host => '10.10.1.13');
SELECT add_data_node('dn3', host => '10.10.1.14');
SELECT add_data_node('dn4', host => '10.10.1.15');
SELECT add_data_node('dn5', host => '10.10.1.16');

SELECT create_distributed_hypertable('github_events', 'start_date', 'pkey', 
    data_nodes => '{ "dn1", "dn2", "dn3", "dn4", "dn5"}');

CREATE INDEX ON github_events (stars, start_date DESC);
CREATE INDEX ON github_events (forks, start_date DESC);
CREATE INDEX ON github_events (adds, start_date DESC);
CREATE INDEX ON github_events (dels, start_date DESC);
CREATE INDEX ON github_events (add_del_ratio, start_date DESC);
CREATE INDEX ON github_events (events_count, start_date DESC);
CREATE INDEX ON github_events (issues, start_date DESC);
CREATE INDEX ON github_events (end_date, start_date DESC);

SELECT * FROM hypertable_detailed_size('github_events') ORDER BY node_name;

# SELECT * FROM approximate_row_count('github_events');