
# Old Clickhouse Stuff

# CREATE TABLE github_events ENGINE = Memory AS SELECT COLUMNS(event_type), COLUMNS(actor_login), COLUMNS(repo_name), COLUMNS(created_at), COLUMNS(action), COLUMNS(additions), COLUMNS(deletions) FROM github_events_url;
# CREATE TABLE github_events ENGINE = Memory AS SELECT COLUMNS(event_type), COLUMNS(repo_name), COLUMNS(created_at), COLUMNS(action), COLUMNS(additions), COLUMNS(deletions) FROM github_events_url;
# CREATE TABLE github_events ENGINE = MergeTree ORDER BY (event_type, repo_name, created_at) AS SELECT COLUMNS(file_time), COLUMNS(event_type), COLUMNS(actor_login), COLUMNS(repo_name), COLUMNS(created_at), COLUMNS(updated_at), COLUMNS(action), COLUMNS(commits), COLUMNS(additions), COLUMNS(deletions), COLUMNS(changed_files), COLUMNS(push_size) FROM github_events_url;
# CREATE TABLE github_events_uncompressed ENGINE = MergeTree ORDER BY (event_type, created_at) AS SELECT COLUMNS(file_time), COLUMNS(event_type), COLUMNS(created_at), COLUMNS(updated_at), COLUMNS(action), COLUMNS(comment_id), COLUMNS(position), COLUMNS(line), COLUMNS(ref_type), COLUMNS(number), COLUMNS(state), COLUMNS(locked), COLUMNS(comments), COLUMNS(author_association), COLUMNS(closed_at), COLUMNS(merged_at), COLUMNS(merged), COLUMNS(mergeable), COLUMNS(rebaseable), COLUMNS(mergeable_state), COLUMNS(review_comments), COLUMNS(maintainer_can_modify), COLUMNS(commits), COLUMNS(additions), COLUMNS(deletions), COLUMNS(changed_files), COLUMNS(original_position), COLUMNS(push_size), COLUMNS(push_distinct_size), COLUMNS(review_state) FROM github_events;


# CREATE TABLE github_events
# (
#     file_time DateTime,
#     event_type Enum('CommitCommentEvent' = 1, 'CreateEvent' = 2, 'DeleteEvent' = 3, 'ForkEvent' = 4,
#                     'GollumEvent' = 5, 'IssueCommentEvent' = 6, 'IssuesEvent' = 7, 'MemberEvent' = 8,
#                     'PublicEvent' = 9, 'PullRequestEvent' = 10, 'PullRequestReviewCommentEvent' = 11,
#                     'PushEvent' = 12, 'ReleaseEvent' = 13, 'SponsorshipEvent' = 14, 'WatchEvent' = 15,
#                     'GistEvent' = 16, 'FollowEvent' = 17, 'DownloadEvent' = 18, 'PullRequestReviewEvent' = 19,
#                     'ForkApplyEvent' = 20, 'Event' = 21, 'TeamAddEvent' = 22),
#     created_at DateTime,
#     updated_at DateTime,
#     action Enum('none' = 0, 'created' = 1, 'added' = 2, 'edited' = 3, 'deleted' = 4, 'opened' = 5, 'closed' = 6, 'reopened' = 7, 'assigned' = 8, 'unassigned' = 9,
#                 'labeled' = 10, 'unlabeled' = 11, 'review_requested' = 12, 'review_request_removed' = 13, 'synchronize' = 14, 'started' = 15, 'published' = 16, 'update' = 17, 'create' = 18, 'fork' = 19, 'merged' = 20),
#     comment_id UInt64,
#     position Int32,
#     line Int32,
#     ref_type Enum('none' = 0, 'branch' = 1, 'tag' = 2, 'repository' = 3, 'unknown' = 4),
#     number UInt32,
#     state Enum('none' = 0, 'open' = 1, 'closed' = 2),
#     locked UInt8,
#     comments UInt32,
#     author_association Enum('NONE' = 0, 'CONTRIBUTOR' = 1, 'OWNER' = 2, 'COLLABORATOR' = 3, 'MEMBER' = 4, 'MANNEQUIN' = 5),
#     closed_at DateTime,
#     merged_at DateTime,
#     merged UInt8,
#     mergeable UInt8,
#     rebaseable UInt8,
#     mergeable_state Enum('unknown' = 0, 'dirty' = 1, 'clean' = 2, 'unstable' = 3, 'draft' = 4),
#     review_comments UInt32,
#     maintainer_can_modify UInt8,
#     commits UInt32,
#     additions UInt32,
#     deletions UInt32,
#     changed_files UInt32,
#     original_position UInt32,
#     push_size UInt32,
#     push_distinct_size UInt32,
#     review_state Enum('none' = 0, 'approved' = 1, 'changes_requested' = 2, 'commented' = 3, 'dismissed' = 4, 'pending' = 5)
# )
# ENGINE = MergeTree
# ORDER BY (event_type, created_at)



# sudo dd if=/dev/null of=nyt_data_csv/trips.tsv bs=2 seek=$(echo $(stat --format=%s nyt_data_csv/trips.tsv ) - $( tail -n2 nyt_data_csv/trips.tsv | wc -c) | bc )

# SELECT
#     count(*), 
#     sum(additions) AS adds,
#     sum(deletions) AS dels
# FROM github_events_uncompressed
# WHERE (event_type = 'PullRequestEvent') AND (action = 'opened') AND (additions < 10000) AND (deletions < 10000)
# HAVING (adds / dels) < 10
# ORDER BY adds + dels DESC

# SELECT
#     count(*), 
#     sum(additions) AS adds,
#     sum(deletions) AS dels
# FROM github_events
# WHERE (event_type = 'PullRequestEvent') AND (action = 'opened') AND (additions < 10000) AND (deletions < 10000)
# HAVING (adds / dels) < 10
# ORDER BY adds + dels DESC

