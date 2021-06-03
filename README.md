# md-trie
Project Slides (Ziming): https://docs.google.com/presentation/d/1rN2j5ozQGbtemTO2X1gEs6-Cveluh2og6-FMKkGvgdQ/edit?usp=sharing  
bitmap.h adapted from ds-lib: https://github.com/anuragkh/ds-lib  
CMake Tutorial: https://google.github.io/googletest/quickstart-cmake.html#next-steps  
Catch2 Tutorial: https://github.com/catchorg/Catch2/blob/devel/docs/tutorial.md#top  
Old trie files with comment: https://github.com/MaoZiming/md-trie/tree/cdc8c989e50ebedc0269df94438c3f14c03f7115/k2-trie-with-comments  
Old kd trie implementation Github: https://github.com/darroyue/k2-dyn-tries  
tqdm progress bar: https://github.com/aminnj/cpptqdm

### Building

```bash
mkdir build
cd build
cmake ..
make
```

### Running tests

```bash
make test
```

### Running benchmark

```bash
./build/libmdtrie/mdtrie_bench
```