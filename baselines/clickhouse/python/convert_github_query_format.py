import re 
import sys

datapath = "/proj/trinity-PG0/Trinity/queries/github/github_query"
outfile = open("/proj/trinity-PG0/Trinity/queries/github/github_query_converted", "w")  # write mode

# [events_count, authors_count, forks, stars, issues, pushes, pulls, downloads, adds, dels, add_del_ratio, start_date, end_date]
if __name__ == "__main__": 

    regex_template_1 = re.compile(r"where start_date <= (?P<start_date_end>[0-9.]+?) AND end_date >= (?P<end_date_start>[0-9.]+?) AND stars >= (?P<stars_start>[0-9.]+?),")
    regex_template_2 = re.compile(r"where stars >= (?P<stars_start>[0-9.]+?) and forks >= (?P<forks_start>[0-9.]+?),")
    regex_template_3 = re.compile(r"where events_count <= (?P<events_count_end>[0-9.]+?) and issues >= (?P<issues_start>[0-9.]+?) AND stars >= (?P<stars_start>[0-9.]+?),")
    regex_template_4 = re.compile(r"where events_count <= (?P<events_count_end>[0-9.]+?) and issues >= (?P<issues_start>[0-9.]+?) AND stars >= (?P<stars_start>[0-9.]+?),")

    with open("{}".format(datapath)) as ifile:

        for line in ifile:

            m = regex_template_1.search(line)
            if m:
                out_line = "11,{},{},12,{},{},3,{},{}".format(-1, m.group("start_date_end"), m.group("end_date_start"), -1, m.group("stars_start"), -1)
                outfile.write(out_line + "\n")
                continue
            m = regex_template_2.search(line)
            if m:
                out_line = "3,{},{},2,{},{}".format(m.group("stars_start"), -1, m.group("forks_start"), -1)
                outfile.write(out_line + "\n")
                continue
            m = regex_template_3.search(line)
            if m:
                out_line = "0,{},{},4,{},{},3,{},{}".format(-1, m.group("events_count_end"), m.group("issues_start"), -1, m.group("stars_start"), -1)
                outfile.write(out_line + "\n")
                continue

    outfile.close()