#!/bin/bash

find ./libmdtrie -name *.h -o -name *.cpp -o -name *.hpp | xargs clang-format -i -style Mozilla
find ./librpc -name *.h -o -name *.cpp -o -name *.hpp | xargs clang-format -i -style Mozilla
