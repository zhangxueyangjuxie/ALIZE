#!/bin/bash
# descript : transform score format for plot DET curve
# author : xueyang
# date : 2017-5-5

in_out_path=./res
in_file=$in_out_path/scores_SphNorm_PLDA.txt 
out_file=$in_out_path/scores_SphNorm_PLDA_DET.txt
num=40

#get last column
cat $in_file | awk '{print $5}' > score_temp.txt

#create score label
for i in $(seq 1 $num)
do
	for j in $(seq 1 $num)
	do
		if [ $i == $j ]; then
			echo 1 >> label_temp.txt
		else
			echo 0 >> label_temp.txt
		fi
	done
done

#combining label and score into out_file
paste score_temp.txt label_temp.txt > $out_file

#delete temp file
rm score_temp.txt label_temp.txt

	
