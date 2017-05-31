#!/bin/bash/

#create ivExtractor.ndx
infile_adapt=lst/adapt.lst
infile_test=lst/test.lst
infile_train_ubm=lst/train_ubm.lst
ivExtractor=ndx/ivExtractor.ndx

[ -f $infile_adapt ] || echo "can not find $infile_adapt"
[ -f $infile_test ] || echo "can not find $infile_test"
[ -f $infile_train_ubm ] || echo "can not find $infile_train_ubm"
[ -f $ivExtractor ] && rm $ivExtractor
touch $ivExtractor

for i in `cat $infile_adapt`; do
	tar=$i" "$i
	echo $tar >> $ivExtractor
done

for i in `cat $infile_test`; do
	tar=$i" "$i
	echo $tar >> $ivExtractor
done

for i in `cat $infile_train_ubm`; do
	tar=$i" "$i
	echo $tar >> $ivExtractor
done

#create trainModel.ndx
trainModel=ndx/trainModel.ndx
[ -f $trainModel ] && rm $trainModel
touch $trainModel

spk=1
count=1
str="spk"$spk
for i in `cat $infile_adapt`; do
	if [ $count -eq 9 ]; then
		str=$str" "$i
		echo $str >> $trainModel
		((++spk))
		str="spk"$spk
		count=1
	else
		str=$str" "$i
		((++count))
	fi
done

#create ivTest_plda_target-seg.ndx
ivTest_plda_target=ndx/ivTest_plda_target-seg.ndx
[ -f $ivTest_plda_target ] && rm $ivTest_plda_target
touch $ivTest_plda_target

count=1
str=
for i in `cat $infile_test`; do
	str=$str" spk"$count
	((++count))
done

for i in `cat $infile_test`; do
	echo $i$str >> $ivTest_plda_target
done

#create Plda.ndx
Plda=ndx/Plda.ndx
[ -f $Plda ] && rm $Plda
touch $Plda

count=1
str=
for i in `cat $infile_train_ubm`; do
	if [ $count -eq 3 ]; then
		str=$str$i
		echo $str >> $Plda
		str=
		count=1
	else
		str=$str$i" "
		((++count))
	fi
done

