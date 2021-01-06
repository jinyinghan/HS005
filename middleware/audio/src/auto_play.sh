#!/bin/sh

i=100

while [ $i -ge 0 ]
do
	echo i is $i 
	let tmp=100-i
    ./sample_play 16000 ding.pcm $tmp
    sleep 1
	let i=i-10
done
