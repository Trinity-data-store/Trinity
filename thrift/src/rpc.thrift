
service MDTrieShard {   

    bool ping(1:i32 dataset_idx),

    i32 add(1:i32 num1, 2:i32 num2),

    i32 insert(1:list<i32> point),

    bool check(1:list<i32> point),

    list<i32> range_search(1:list<i32> start_range, 2:list<i32> end_range),

    list<i32> primary_key_lookup(1:i32 primary_key),

    i32 get_size(),

    void clear_trie(),
}
