#!/bin/bash
oldnum=`cut -d ',' -f2 BuildNumberCache.txt`
newnum=`expr $oldnum + 1`
version="0.2.2 (type-rework bytecode development)"
echo "$newnum" > BuildNumberCache.txt
echo "#ifndef INCREMENT_HPP" > Dodo-lang/src/Misc/Increment.hpp
echo "#define INCREMENT_HPP" >> Dodo-lang/src/Misc/Increment.hpp
echo "" >> Dodo-lang/src/Misc/Increment.hpp
echo "#define INCREMENTED_VALUE \"$version, build: $newnum (`date +%d-%m-%y`)\"" >> Dodo-lang/src/Misc/Increment.hpp
echo "" >> Dodo-lang/src/Misc/Increment.hpp
echo "#endif" >> Dodo-lang/src/Misc/Increment.hpp