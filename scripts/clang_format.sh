#!/bin/bash

find ./libmdtrie -iname *.h -o -iname *.cpp | xargs clang-format -i
find ./librpc -iname *.h -o -iname *.cpp | xargs clang-format -i
