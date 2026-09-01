#!/bin/sh

unzip chall.zip > /dev/null
./case &
sleep 1
rm case default.jpg
