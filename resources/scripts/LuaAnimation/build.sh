#!/bin/bash
script=json2lua.lua
p=$1
f=${p##*/}
r=${p%/*}
cd $r
lua $script $f
exit