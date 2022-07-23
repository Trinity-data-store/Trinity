CREATE TABLE github_events_url
(
    file_time DateTime,
    event_type Enum('CommitCommentEvent' = 1, 'CreateEvent' = 2, 'DeleteEvent' = 3, 'ForkEvent' = 4,
                    'GollumEvent' = 5, 'IssueCommentEvent' = 6, 'IssuesEvent' = 7, 'MemberEvent' = 8,
                    'PublicEvent' = 9, 'PullRequestEvent' = 10, 'PullRequestReviewCommentEvent' = 11,
                    'PushEvent' = 12, 'ReleaseEvent' = 13, 'SponsorshipEvent' = 14, 'WatchEvent' = 15,
                    'GistEvent' = 16, 'FollowEvent' = 17, 'DownloadEvent' = 18, 'PullRequestReviewEvent' = 19,
                    'ForkApplyEvent' = 20, 'Event' = 21, 'TeamAddEvent' = 22),
    actor_login LowCardinality(String),
    repo_name LowCardinality(String),
    created_at DateTime,
    updated_at DateTime,
    action Enum('none' = 0, 'created' = 1, 'added' = 2, 'edited' = 3, 'deleted' = 4, 'opened' = 5, 'closed' = 6, 'reopened' = 7, 'assigned' = 8, 'unassigned' = 9,
                'labeled' = 10, 'unlabeled' = 11, 'review_requested' = 12, 'review_request_removed' = 13, 'synchronize' = 14, 'started' = 15, 'published' = 16, 'update' = 17, 'create' = 18, 'fork' = 19, 'merged' = 20),
    comment_id UInt64,
    body String,
    path String,
    position Int32,
    line Int32,
    ref LowCardinality(String),
    ref_type Enum('none' = 0, 'branch' = 1, 'tag' = 2, 'repository' = 3, 'unknown' = 4),
    creator_user_login LowCardinality(String),
    number UInt32,
    title String,
    labels Array(LowCardinality(String)),
    state Enum('none' = 0, 'open' = 1, 'closed' = 2),
    locked UInt8,
    assignee LowCardinality(String),
    assignees Array(LowCardinality(String)),
    comments UInt32,
    author_association Enum('NONE' = 0, 'CONTRIBUTOR' = 1, 'OWNER' = 2, 'COLLABORATOR' = 3, 'MEMBER' = 4, 'MANNEQUIN' = 5),
    closed_at DateTime,
    merged_at DateTime,
    merge_commit_sha String,
    requested_reviewers Array(LowCardinality(String)),
    requested_teams Array(LowCardinality(String)),
    head_ref LowCardinality(String),
    head_sha String,
    base_ref LowCardinality(String),
    base_sha String,
    merged UInt8,
    mergeable UInt8,
    rebaseable UInt8,
    mergeable_state Enum('unknown' = 0, 'dirty' = 1, 'clean' = 2, 'unstable' = 3, 'draft' = 4),
    merged_by LowCardinality(String),
    review_comments UInt32,
    maintainer_can_modify UInt8,
    commits UInt32,
    additions UInt32,
    deletions UInt32,
    changed_files UInt32,
    diff_hunk String,
    original_position UInt32,
    commit_id String,
    original_commit_id String,
    push_size UInt32,
    push_distinct_size UInt32,
    member_login LowCardinality(String),
    release_tag_name String,
    release_name String,
    review_state Enum('none' = 0, 'approved' = 1, 'changes_requested' = 2, 'commented' = 3, 'dismissed' = 4, 'pending' = 5)
) ENGINE = URL('https://datasets.clickhouse.com/github_events_v2.native.xz', Native);


CREATE TABLE github_events_small ENGINE = MergeTree ORDER BY (event_type, repo_name, created_at) AS SELECT event_type, repo_name, actor_login, created_at, updated_at, action, number, commits, additions, deletions, changed_files, review_state FROM github_events;

pixz -d < github_events_v2.native.xz | clickhouse-client --query "INSERT INTO github_events FORMAT Native"

CREATE TABLE github_events_macro ENGINE = MergeTree ORDER BY (repos, year) AS SELECT
    repo_name,
    count() AS repos,
    sum(event_type = 'ForkEvent') AS forks,
    sum(event_type = 'WatchEvent') AS stars,
    sum(event_type = 'IssuesEvent') AS issues,
    sum(event_type = 'PullRequestEvent') AS prs,
    sum(additions) AS adds,
    sum(deletions) AS dels,
    adds / dels AS add_del_ratio,
    min(toYear(created_at)) AS year
WHERE event_type IN ('ForkEvent', 'WatchEvent', 'PullRequestEvent', 'IssuesEvent')
GROUP BY repo_name
FROM github_events_small;

DROP TABLE github_events_macro;
CREATE TABLE github_events_macro ENGINE = MergeTree ORDER BY (repo_count, authors) AS SELECT
    rowNumberInAllBlocks() AS repo,
    count() AS repo_count,
    uniq(actor_login) AS authors,
    sum(number) AS numbers,
    sum(event_type = 'ForkEvent') AS forks,
    sum(event_type = 'WatchEvent') AS stars,
    sum(event_type = 'IssuesEvent') AS issues,
    sum(event_type = 'PullRequestEvent') AS prs,
    sum(additions) AS adds,
    sum(deletions) AS dels,
    adds / dels AS add_del_ratio,
    min(created_at) AS min_created,
    max(created_at) AS max_created
FROM github_events_small
WHERE event_type IN ('ForkEvent', 'WatchEvent', 'PullRequestEvent', 'IssuesEvent')
GROUP BY repo_name;

rowNumberInAllBlocks() AS repo,

CREATE TABLE github_events_macro ENGINE = MergeTree ORDER BY (events_count, authors_count) AS SELECT
    count() AS events_count,
    uniq(actor_login) AS authors_count,
    sum(event_type = 'ForkEvent') AS forks,
    sum(event_type = 'WatchEvent') AS stars,
    sum(event_type = 'IssuesEvent') AS issues,
    sum(event_type = 'PushEvent') AS pushes,
    sum(event_type = 'PullRequestEvent') AS pulls,
    sum(event_type = 'DownloadEvent') AS downloads,
    sum(additions) AS adds,
    sum(deletions) AS dels,
    if(dels != 0, adds / dels, 0) AS add_del_ratio,
    toDate(min(created_at)) AS earliest_event_date,
    toDate(max(created_at)) AS latest_event_date
FROM github_events_small
GROUP BY repo_name;

INSERT INTO github_events_macro SELECT
    count() AS events_count,
    uniq(actor_login) AS authors_count,
    sum(event_type = 'ForkEvent') AS forks,
    sum(event_type = 'WatchEvent') AS stars,
    sum(event_type = 'IssuesEvent') AS issues,
    sum(event_type = 'PushEvent') AS pushes,
    sum(event_type = 'PullRequestEvent') AS pulls,
    sum(event_type = 'DownloadEvent') AS downloads,
    sum(additions) AS adds,
    sum(deletions) AS dels,
    if(dels != 0, adds / dels, 0) AS add_del_ratio,
    toDate(min(created_at)) AS earliest_event_date,
    toDate(max(created_at)) AS latest_event_date
FROM github_events_small
GROUP BY repo_name;


CREATE TABLE github_events_final ENGINE = MergeTree ORDER BY (events_count, authors_count) AS SELECT
    rowNumberInAllBlocks() AS pkey,
    *
FROM github_events_macro;

# sum(number) AS numbers,
Select COUNT(*) from github_events_final where stars > 100 and forks > 100
Select COUNT(*) from github_events_final where adds < 10000 and dels < 10000 AND add_del_ratio < 10
Select COUNT(*) from github_events_final where events_count < 10000 and issues > 1 AND stars > 1
# Select COUNT(*) from github_events_final where toYear(earliest_event_date) >= 2015
Select COUNT(*) from github_events_final where earliest_event_date <= '2019-01-01' AND latest_event_date >= '2020-06-01' AND stars >= 1000
# Select COUNT(*) from github_events_macro where numbers > 10 and authors >= 10


SELECT * FROM github_events_final INTO OUTFILE '/mntData2/github/github_events.csv' FORMAT CSV


(pkey UInt32, events_count UInt32, authors_count UInt32, forks UInt32, stars UInt32, issues UInt32, pushes UInt32, pulls UInt32, downloads UInt32, adds UInt32, dels UInt32, add_del_ratio Float32, start_date UInt32, end_date UInt32)


SELECT
                              exp10(floor(log10(stars))) AS stars,
                              uniq(pkey)
                          FROM github_events_final
                          GROUP BY stars
                          ORDER BY stars ASC


CREATE TABLE github_events_testing ENGINE = MergeTree ORDER BY (events_count, authors_count) AS SELECT
    rowNumberInAllBlocks() AS pkey,
    count() AS events_count,
    uniq(actor_login) AS authors_count,
    sum(event_type = 'ForkEvent') AS forks,
    sum(event_type = 'WatchEvent') AS stars,
    sum(event_type = 'IssuesEvent') AS issues,
    sum(event_type = 'PushEvent') AS pushes,
    sum(event_type = 'PullRequestEvent') AS pulls,
    sum(event_type = 'DownloadEvent') AS downloads,
    sum(additions) AS adds,
    sum(deletions) AS dels,
    if(dels != 0, adds / dels, 0) AS add_del_ratio,
    toDate(min(created_at)) AS start_date,
    toDate(max(created_at)) AS end_date
FROM github_events_small
GROUP BY repo_name;