## Datasets

**File System (FS)**, a private dataset of metadata collected from a distributed file system. 

**OpenStreetMap (OSM)**, geographical records catalogued by the OpenStreetMap project. [Link](https://download.geofabrik.de/)   
To extract the data, use Python Osmium package and a sample script is provided. [Here](OSM/process_osm.py)  

**TPC-H**, a synthetic business dataset. We coalesce its lineitem and orders tables. [Link](https://docs.deistercloud.com/content/Databases.30/TPCH%20Benchmark.90/Data%20generation%20tool.30.xml?embedded=true/)

Place the datasets under /libmdtrie/bench/data