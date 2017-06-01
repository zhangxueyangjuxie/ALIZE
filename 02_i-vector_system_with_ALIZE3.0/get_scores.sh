#!/bin/bash/

#create ture_speaker_scores and imposter_scores
#by xueyang on 2017.05.31

file_name=scores_SphNorm_PLDA.txt
spk_num=100

infile=res/$file_name
score_path=speaker_scores/$file_name
true_scores=$score_path/true_speaker_scores
imposter_scores=$score_path/impostor_scores

[ -d $score_path ] && rm -r $score_path
mkdir $score_path


count=0
flag=0
while read line
do

	if [ $count -eq $spk_num ]; then
		count=0
		((++flag))
	fi
	score=`echo $line | cut -d " " -f5`
	if [ $count -eq $flag ]; then
		echo $score >> $true_scores
	else
		echo $score >> $imposter_scores
	fi
	((++count))

done < $infile




