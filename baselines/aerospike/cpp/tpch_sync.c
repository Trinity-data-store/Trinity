#include <aerospike/aerospike.h>
#include <aerospike/aerospike_batch.h>
#include <aerospike/aerospike_key.h>
#include <aerospike/as_event.h>
#include <aerospike/as_monitor.h>
#include <unistd.h>
#include <sys/time.h>

const char *name_list[] = {"quantity", "extendedprice", "discount", "tax", "shipdate", "commitdate", "recepitdate", "totalprice", "orderdate"};
static const char* g_host = "10.10.1.5";
static int g_port = 3000;
static const char* g_namespace = "tpch";
static const char* g_set = "tpch_macro";

int each_client_insert(char *file_address, uint32_t num_points_to_insert) {

    FILE *fp = fopen(file_address, "r");
    size_t len;

    as_error err;
    as_config config;
    as_config_init(&config);

    config.hosts[0] = { .addr = g_host, .port = g_port };

    aerospike as;
    aerospike_init(&as, &config);
    aerospike_connect(&as, &err);
    ssize_t read;
    uint32_t points_inserted = 0;
    while ((read = getline(&line, &len, fp)) != -1)
    {
        char *ptr;
        
        char *token = strtok(line, ","); // id
        if (!token) {
            printf("line: %s\n", line);
            return false;
        }
        id = strtoul(token, &ptr, 10);
        
        for (int i = 0; i < 9; i++){
            token = strtok(NULL, ","); // id
            if (!token) {
                printf("line: %s\n", line);
                return false;
            }
            int64_t num = strtoll(token, &ptr, 10);
            as_record_set_int64(&rec, name_list[i], num);
        }
        aerospike_key_put(&as, &err, NULL, &key, &rec);
        points_inserted += 1;
        if (points_inserted == num_points_to_insert)
            break;
    }
    aerospike_close(&as);
    aerospike_destroy(&as);
}

uint32_t total_client_insert(int start_split, int num_threads){

  std::vector<std::future<uint32_t>> threads; 
  threads.reserve(num_threads);

  int end_split = start_split + num_threads - 1;

  for (int i = start_split; i <= end_split; i++){

    char file_address[100];
    snprintf(file_address, sizeof(file_address), "/mntData/tpch_split/x%d", i);
    uint32_t num_points_to_insert = 2500000;
    threads.push_back(std::async(each_client_insert, file_address, num_points_to_insert));
  }  

  uint32_t total_throughput = 0;
  for (int i = 0; i < num_threads; i++){
    total_throughput += threads[i].get();
  } 
  return total_throughput;  
}

int main(int argc, char *argv[]) {

    if (argc != 4) {
        std::cerr << "wrong num of arg" << std::endl;
        exit(-1);
    }
    int start_split = atoi(argv[1]);
    int num_threads = atoi(argv[2]);
    uint32_t total_throughput = total_client_insert(start_split, num_threads);
    std::cout << "throughput: " << total_throughput << std::endl;
}