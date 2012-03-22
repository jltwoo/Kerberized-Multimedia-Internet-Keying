#!/bin/sh

sed '$d' < "$1" > "$1.tmp" ; mv "$1.tmp" "$1"
