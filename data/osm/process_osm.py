import osmium
import sys
import pprint
import csv
pp = pprint.PrettyPrinter(indent=4)

jsons = []
csvfile = open('osm_dataset.csv', 'w', newline='')
fieldnames = ['id', 'version', "timestamp", "lon", "lat"]
idx = 0
writer = csv.DictWriter(csvfile, fieldnames=fieldnames)
writer.writeheader()
total_processed = 0

has_tags = 0
total_count = 0

class NamesHandler(osmium.SimpleHandler):
    def node(self, n):
                        
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
        if n.timestamp:
            json["timestamp"] = n.timestamp.year * 10000 + n.timestamp.month * 100 + n.timestamp.day
        else:
            print("timestamp not found")
            return
        if n.location and n.location.lon and n.location.lat:
            json["lon"] = round(abs(n.location.lon) * 10000000)
            json["lat"] = round(abs(n.location.lat) * 10000000)
        else:
            print("location not found")
            return
            
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