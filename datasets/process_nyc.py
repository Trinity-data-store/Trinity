processed_lines = 0

with open("/mntData2/dataset_csv/nyc_dataset_large.csv", 'a') as outfile:
    with open("/mntData2/old/nyc_taxi/nyc_taxi_processed_ch.csv") as infile:
        for line in infile:
                
            line_split = line.strip().split(",")
            
            line_split[4] = str(int(abs(float(line_split[4]) * 10)))
            line_split[6] = str(int(abs(float(line_split[6]) * 10)))
            
            line_split[7] = str(int(abs(float(line_split[7]))))
            line_split[8] = str(int(abs(float(line_split[8]) * 10)))
            line_split[9] = str(int(abs(float(line_split[9]) * 10)))
            line_split[10] = str(int(abs(float(line_split[10]) * 10)))
            line_split[11] = str(int(abs(float(line_split[11]) * 10)))
            line_split[12] = str(int(abs(float(line_split[12]) * 10)))
            line_split[13] = str(int(abs(float(line_split[13]))))
            line_split[14] = str(int(abs(float(line_split[14]) * 10)))
            line_split[15] = str(int(abs(float(line_split[15]) * 10)))

            line_split = [str(processed_lines)] + line_split[1:]
            outfile.write(",".join(line_split) + '\n')

            if processed_lines % 1000000 == 0:
                print(processed_lines)
            
            processed_lines += 1
            
print(processed_lines)