#!/bin/bash

OUT_PATH=../fflib

./configure \
    --prefix=$OUT_PATH \
    --incdir=$OUT_PATH/include \
    --disable-static     \
    --enable-shared      \
    --disable-yasm \
    --disable-symver \
    --enable-gpl \
    --disable-ffmpeg \
    --disable-ffplay \
    --disable-ffprobe \
    --disable-doc \
    --disable-symver 

make -j18
make install

