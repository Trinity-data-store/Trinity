
service MDTrieShard {   

    void ping(),

    i32 add(1:i32 num1, 2:i32 num2),

    void insert_trie(1:list<i32> point, 2:i32 length),
}
