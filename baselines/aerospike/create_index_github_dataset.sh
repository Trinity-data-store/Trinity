manage sindex create numeric stars_index ns macro_bench set github_macro bin stars
manage sindex create numeric forks_index ns macro_bench set github_macro bin forks
manage sindex create numeric events_count_index ns macro_bench set github_macro bin events_count
manage sindex create numeric issues_index ns macro_bench set github_macro bin issues
manage sindex create numeric start_date_index ns macro_bench set github_macro bin start_date
manage sindex create numeric end_date_index ns macro_bench set github_macro bin end_date

manage sindex delete adds_index ns tpch
manage sindex delete dels_index ns tpch
manage sindex delete add_del_ratio_index ns tpch
