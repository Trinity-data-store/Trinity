#!/bin/bash

find ./libmdtrie -iname *.h -o -iname *.cpp | xargs clang-format -i -style Mozilla
find ./librpc -iname *.h -o -iname *.cpp | xargs clang-format -i -style Mozilla
