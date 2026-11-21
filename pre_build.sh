#!/bin/bash

# Build version tracking
python BuildTracking/IncrementBuild.py

# Flex and Bison
SCRIPT_CACHE_DIR=".script-cache"

FLEX_HASH_FILE="$SCRIPT_CACHE_DIR/flex-hash"
BISON_HASH_FILE="$SCRIPT_CACHE_DIR/bison-hash"

SRC_DIR="src"
FLEX_SRC_DIR="$SRC_DIR/flex"
FLEX_IN_FILE="$FLEX_SRC_DIR/dodo_lang.lex"
FLEX_OUT_FILE="$FLEX_SRC_DIR/flex.c"
FLEX_OUT_HEADER="$FLEX_SRC_DIR/flex.h"
BISON_SRC_DIR="$SRC_DIR/bison"
BISON_IN_FILE="$BISON_SRC_DIR/dodo_lang.y"
BISON_OUT_FILE="$BISON_SRC_DIR/bison.c"
BISON_OUT_HEADER="$BISON_SRC_DIR/bison.h"

STRINGS_PATH="$SRC_DIR/strings"
LANGUAGE_LINK="$STRINGS_PATH/selected"

if [[ ! -n $1 ]];
then
    LANGUAGE="EN"
else
    LANGUAGE=$1
fi

echo "Selected language: $LANGUAGE"

HASH_COMMAND="sha512sum"

if [ ! -e $LANGUAGE_LINK ]; then
    echo "Creating language symlink: $LANGUAGE_LINK to: $LANGUAGE"
    ln -s $LANGUAGE $LANGUAGE_LINK
fi

if [[ $(readlink $LANGUAGE_LINK) != "$LANGUAGE" ]]; then
    echo "Changing language symlink: $LANGUAGE_LINK to: $LANGUAGE"
    rm $LANGUAGE_LINK
    ln -s $LANGUAGE $LANGUAGE_LINK
fi

if [ ! -d $SCRIPT_CACHE_DIR ]; then
    mkdir $SCRIPT_CACHE_DIR
fi

RUN_FLEX="F"
RUN_BISON="F"

if [ ! -e $FLEX_HASH_FILE ] || [ ! -e $FLEX_HASH_FILE ] || [ ! -e $FLEX_OUT_FILE ]; then
    RUN_FLEX="T"
    echo "Flex files missing"
else
    FLEX_SUM=$($HASH_COMMAND $FLEX_IN_FILE)
    if [[ "$FLEX_SUM" != "$(cat $FLEX_HASH_FILE)" ]]; then
        RUN_FLEX="T"
        echo "Flex files outdated"
    fi
fi

if [ ! -e $BISON_HASH_FILE ] || [ ! -e $BISON_HASH_FILE ] || [ ! -e $BISON_OUT_FILE ]; then
    RUN_BISON="T"
    echo "Bison files missing"
else
    BISON_SUM=$($HASH_COMMAND $BISON_IN_FILE)
    if [[ "$BISON_SUM" != "$(cat $BISON_HASH_FILE)" ]]; then
        RUN_BISON="T"
        echo "Bison files outdated"
    fi
fi

if [[ "$RUN_FLEX" == "T" ]]; then
    echo "Running flex..."
    flex -o $FLEX_OUT_FILE --header-file=$FLEX_OUT_HEADER $FLEX_IN_FILE
    sed -i "s/yy_cp += (int) (dest - source)/yy_cp += (dest - source)/g" $FLEX_OUT_FILE
    sed -i "s/yy_bp += (int) (dest - source)/yy_bp += (dest - source)/g" $FLEX_OUT_FILE
    $HASH_COMMAND $FLEX_IN_FILE > $FLEX_HASH_FILE
    echo "Flex finished running, input file hash saved"
fi

if [[ "$RUN_BISON" == "T" ]]; then
    echo "Running bison..."
    bison -o $BISON_OUT_FILE --header=$BISON_OUT_HEADER --language=C $BISON_IN_FILE
    $HASH_COMMAND $BISON_IN_FILE > $BISON_HASH_FILE
    echo "Bison finished running, input file hash saved"
fi

echo "Pre run script done!"

