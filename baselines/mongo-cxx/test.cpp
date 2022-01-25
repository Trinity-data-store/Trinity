#include <cstdint>
#include <iostream>
#include <vector>
#include <mongocxx/client.hpp>
#include <mongocxx/stdx.hpp>
#include <mongocxx/uri.hpp>
#include <mongocxx/instance.hpp>
#include <bsoncxx/builder/stream/helpers.hpp>
#include <bsoncxx/builder/stream/document.hpp>
#include <bsoncxx/builder/stream/array.hpp>
#include <sys/time.h>
#include <thread>
#include <mongocxx/pool.hpp>
#include <bsoncxx/types.hpp>
#include <bsoncxx/json.hpp>

using bsoncxx::builder::stream::close_array;
using bsoncxx::builder::stream::close_document;
using bsoncxx::builder::stream::document;
using bsoncxx::builder::stream::finalize;
using bsoncxx::builder::stream::open_array;
using bsoncxx::builder::stream::open_document;
using bsoncxx::builder::basic::make_document;
using bsoncxx::builder::basic::kvp;

typedef unsigned long long int TimeStamp;

TimeStamp GetTimestamp() {
  struct timeval now;
  gettimeofday(&now, nullptr);
  return now.tv_usec + (TimeStamp) now.tv_sec * 1000000;
}

std::vector<std::vector<int>> get_data_vector(){

    FILE *fp = fopen("/home/ziming/md-trie/libmdtrie/bench/data/sample_shuf.txt", "r");
    char *line = nullptr;
    size_t len = 0;
    ssize_t read;
    std::vector<std::vector<int>> return_vect;
    int n_lines = 14583357;
    int n_points = 0;

    while ((read = getline(&line, &len, fp)) != -1)
    {
        if (n_points % (n_lines / 15) == 0)
            std::cout << "n_points: " << n_points << std::endl;

        char *token = strtok(line, " ");
        char *ptr;
        std::vector<int> current_vect(7, 0);
        for (int i = 1; i <= 2; i ++){
            token = strtok(nullptr, " ");
        }

        for (int i = 0; i < 7; i++){
            token = strtok(nullptr, " ");
            current_vect[i] = strtoul(token, &ptr, 10);
        }

        return_vect.push_back(current_vect);
        n_points ++;
    }        
    return return_vect;

}

int main(){
    
    mongocxx::instance instance{}; // This should be done only once.
    mongocxx::uri uri("mongodb://172.29.249.30:27017");
    mongocxx::pool pool{uri};

    auto client = pool.acquire();
    mongocxx::database db = (*client)["test"];
    mongocxx::collection coll = db["test"];
    mongocxx::cursor cursor_throughput = coll.find({});

    std::vector<std::string> id_vector;

    for (auto doc : cursor_throughput) {
        id_vector.push_back(doc["_id"].get_oid().value.to_string());
    }
    std::cout << "done! " << "vector size: " << id_vector.size() << std::endl;
    int partition_num = 24;

    std::vector<std::thread> threads{};
    for (int i = 0; i < partition_num; i++) {

        auto run_find = [&](std::int64_t j) {

            // Each client and collection can only be used in a single thread.
            auto client = pool.acquire();
            auto coll = (*client)["test"]["test"];

            int start = id_vector.size() / partition_num * j;
            int end = id_vector.size() / partition_num * (j + 1) - 1;

            for (int k = start; k <= end; k ++){
                auto str = id_vector[k];

                auto cursor_id = coll.find_one
                    (document{} << "_id" << bsoncxx::oid(str)
                    << finalize);

                if ((k - start) % ((end - start) / 100) == 0)
                    std::cout << "thread: " << j << "finished: " << k - start << std::endl;
            }
        };
        std::thread runner{run_find, i};
        threads.push_back(std::move(runner));
    }    
    TimeStamp now = GetTimestamp();
    for (auto &th : threads) {
        th.join();
    }
    TimeStamp latency = GetTimestamp() - now;
    std::cout << "latency: " << latency << " Total: " << id_vector.size() << std::endl;

    return 0;
}