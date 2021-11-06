import osmium
import sys
import pprint
import csv
pp = pprint.PrettyPrinter(indent=4)

jsons = []
csvfile = open('osm.csv', 'wb')
# fieldnames = ['id', 'version', 'changeset', "uid", "timestamp-minute", "timestamp-hour", "timestamp-day", "timestamp-month", "timestamp-year", "tags-ele", "tags-gnis:id", "tags-gnis:ST_num", "gnis:County_num", "location-lon (abs, rounded)", "location-lat (abs, rounded)"]
fieldnames = ['id', 'version', 'changeset', "uid", "timestamp-minute", "timestamp-hour", "timestamp-day", "timestamp-month", "timestamp-year",  "location-lon (abs, rounded)", "location-lat (abs, rounded)"]
idx = 0
writer = csv.DictWriter(csvfile, fieldnames=fieldnames)
writer.writeheader()

class NamesHandler(osmium.SimpleHandler):
    def node(self, n):
        if 'name' in n.tags:
            
            json = {}
            json["id"] = int(n.id)
            json["version"] = int(n.version)
            json["changeset"] = int(n.changeset)
            json["uid"] = int(n.uid)

            json["timestamp-minute"] = n.timestamp.minute
            json["timestamp-hour"] = n.timestamp.hour
            json["timestamp-day"] = n.timestamp.day
            json["timestamp-month"] = n.timestamp.month
            json["timestamp-year"] = n.timestamp.year

            json["location-lon (abs, rounded)"] = round(abs(n.location.lon))
            json["location-lat (abs, rounded)"] = round(abs(n.location.lat))

            global idx
            print("done: {}".format(idx))
            idx += 1

            writer.writerow(json)
                

def main(osmfile):
    NamesHandler().apply_file(osmfile)
    return 0

if __name__ == '__main__':
    if len(sys.argv) != 2:
        print("Usage: python %s <osmfile>" % sys.argv[0])
        sys.exit(-1)

    exit(main(sys.argv[1]))