
#include <iostream>

#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/transport/TSocket.h>
#include <thrift/transport/TTransportUtils.h>

#include "MDTrieShardClient.h"
#include <future>

using namespace std;
using namespace apache::thrift;
using namespace apache::thrift::protocol;
using namespace apache::thrift::transport;


int main(){

  if (argc == 3){
    MDTrieClient(atoi(argv[1]), atoi(argv[2]));
    return 0;
  }

  MDTrieClient(9090, 10);
  return 0;

}