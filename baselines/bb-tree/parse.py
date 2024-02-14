import re

def extract_latency(line):
    pattern = r'latency \(ms\): (\d+)'
    match = re.search(pattern, line)
    if match:
        return int(match.group(1))
    else:
        return None

def main():
    # Read the content of the document from the 'results' file
    with open('results_trinity', 'r') as file:
        document = file.read()

    # Split the document into lines and process each line
    lines = document.split('\n')
    latency_values = []

    for line in lines:
        latency = extract_latency(line)
        if latency is not None:
            latency_values.append(latency)

    # Calculate the average latency
    if latency_values:
        average_latency = sum(latency_values) / len(latency_values)
        print("Average End-to-End Latency: {:.2f} ms".format(average_latency))

        # Calculate P99 latency
        latency_values.sort()
        p99_index = int(0.99 * len(latency_values))
        p99_latency = latency_values[p99_index]
        print("P99 Latency: {} ms".format(p99_latency))
    else:
        print("No valid latency values found in the document.")

if __name__ == "__main__":
    main()
