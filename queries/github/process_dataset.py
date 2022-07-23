processed_lines = 0

# // [pkey, events_count, authors_count, forks, stars, issues, pushes, pulls, downloads, adds, dels, add_del_ratio, start_date, end_date]
with open("/mntData2/github/github_events_processed_9.csv", 'a') as outfile:
    with open("/mntData2/github/github_events.csv") as infile:
        for line in infile:
            line_split = line.strip().split(",")
            line_split[-1] = "".join(line_split[-1][1:-1].split("-"))
            line_split[-2] = "".join(line_split[-2][1:-1].split("-"))
            # line_split[11] = "{:.2f}".format(float(line_split[11]))
            line_split = line_split[:9] + line_split[12:]
            outfile.write(",".join(line_split) + '\n')


            if processed_lines % 1000000 == 0:
                print(processed_lines)
            
            processed_lines += 1