import osmium
import sys
import pprint
import csv
pp = pprint.PrettyPrinter(indent=4)

jsons = []
csvfile = open('osm_us_northeast.csv', 'w', newline='')
# fieldnames = ['id', 'version', 'changeset', "uid", "timestamp-minute", "timestamp-hour", "timestamp-day", "timestamp-month", "timestamp-year", "tags-ele", "tags-gnis:id", "tags-gnis:ST_num", "gnis:County_num", "location-lon (abs, rounded)", "location-lat (abs, rounded)"]
# fieldnames = ['id', 'version', 'changeset', "uid", "timestamp-minute", "timestamp-hour", "timestamp-day", "timestamp-month", "timestamp-year",  "location-lon (abs, rounded)", "location-lat (abs, rounded)"]
fieldnames = ['id', 'version', "timestamp-minute", "timestamp-hour", "timestamp-day", "timestamp-month", "timestamp-year",  "location-lon (abs, rounded)", "location-lat (abs, rounded)"]
idx = 0
writer = csv.DictWriter(csvfile, fieldnames=fieldnames)
writer.writeheader()
total_processed = 0

class NamesHandler(osmium.SimpleHandler):
    def node(self, n):
        
        global total_processed
        total_processed += 1
        if total_processed % 100000 == 0:
            print("total_processed", total_processed)

        json = {}
        if n.id:
            json["id"] = int(n.id)
        else:
            print("id not found")
            return
        if n.version:
            json["version"] = int(n.version)
        else:
            print("version not found")
            return
        # if n.changeset:
        #     json["changeset"] = int(n.changeset)
        # else:
        #     print("changeset not found")
        #     return
        # if n.uid:
        #     json["uid"] = int(n.uid)
        # else:
        #     print("uid not found")
        #     return
        if n.timestamp:
            json["timestamp-minute"] = n.timestamp.minute
            json["timestamp-hour"] = n.timestamp.hour
            json["timestamp-day"] = n.timestamp.day
            json["timestamp-month"] = n.timestamp.month
            json["timestamp-year"] = n.timestamp.year
        else:
            print("timestamp not found")
            return
        if n.location:
            print(n.location.lon)
            exit(0)
            json["location-lon (abs, rounded)"] = round(abs(n.location.lon))
            json["location-lat (abs, rounded)"] = round(abs(n.location.lat))
        else:
            print("location not found")
            return
        # jsons.append(json)
        global idx
        idx += 1
        if idx % 10000 == 0:
            print("done: {}".format(idx))

        writer.writerow(json)
                

def main(osmfile):
    NamesHandler().apply_file(osmfile)
    return 0

if __name__ == '__main__':
    if len(sys.argv) != 2:
        print("Usage: python %s <osmfile>" % sys.argv[0])
        sys.exit(-1)

    exit(main(sys.argv[1]))