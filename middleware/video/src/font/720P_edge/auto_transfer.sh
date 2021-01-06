#!/bin/sh

file=`ls ./ |grep bmp |grep -v bmp2font`

echo $file 

for i in $file
do
	
	new_file=`ls $i |tr -d ".bmp"`.font
	echo "start transfer $new_file..."
	../bmp2font $i $new_file 1
done
