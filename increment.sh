#!/bin/bash
oldnum=`cut -d ',' -f2 BuildNumberCache.txt`
newnum=`expr $oldnum + 1`
version="0.2.3 (type-rework assembly generator development)"
echo "$newnum" > BuildNumberCache.txt
echo "#ifndef INCREMENTED_VALUE" > Dodo-lang/src/Misc/Increment.hpp
echo "" >> Dodo-lang/src/Misc/Increment.hpp
echo "#define INCREMENTED_VALUE \"$version, build: $newnum (`date +%Y-%m-%d`)\"" >> Dodo-lang/src/Misc/Increment.hpp
echo "" >> Dodo-lang/src/Misc/Increment.hpp
echo "#endif" >> Dodo-lang/src/Misc/Increment.hpp