include "shared.thrift"

service MDTrieShard extends shared.SharedService {   

    void ping(),

    i32 add(1:i32 num1, 2:i32 num2),

}
