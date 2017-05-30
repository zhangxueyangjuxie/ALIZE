#!/bin/bash/


###################################
#create trainModel.ndx

infile=lst/adapt.lst
outfile=ndx/trainModel.ndx

[ -f $outfile ] && rm $outfile
touch $outfile

count=1
flag=1
str="spk"${count}
for i in `cat $infile`; do

	str=${str}" "${i}
	if [ $flag -eq 9 ]; then
		echo $str >> $outfile
		((flag=1))
		((count++))
		str="spk"${count}
	else 
		((flag++))	
	fi
done

###################################
	
#create computetest_gmm_target_seg.ndx

infile=lst/test.lst
outfile=ndx/computetest_gmm_target-seg.ndx
test_num=100

[ -f $outfile ] && rm $outfile
touch $outfile

str=
for ((i=1; i < test_num; ++i)); do
	str=${str}"spk"${i}" "
done
str=${str}"spk"${test_num}

for i in `cat $infile`; do
	temp=${i}" "${str}
	echo $temp >> $outfile
done




		
		


	
