#!/bin/bash/
#split dataset into train_ubm_dataset adapt_dataset and test_datset

dataset=data/name.lst
train_ubm_dataset=lst/train_ubm.lst
adapt_dataset=lst/adapt.lst
test_dataset=lst/test.lst
imp_adapt_dataset=lst/imp_adapt.lst
imp_test_dataset=lst/imp_test.lst
train_num=5300
imp_num=1000


#create file

[ -f $train_ubm_dataset ] && rm $train_ubm_dataset
touch $train_ubm_dataset

[ -f $adapt_dataset ] && rm $adapt_dataset
touch $adapt_dataset

[ -f $test_dataset ] && rm $test_dataset
touch $test_dataset

[ -f $imp_adapt_dataset ] && rm $imp_adapt_dataset
touch $imp_adapt_dataset

[ -f $imp_test_dataset ] && rm $imp_test_dataset
touch $imp_test_dataset

count=0
flag=0
imp_flag=0
for i in `cat $dataset`;do
	
	if [ $count -lt $train_num ]; then
		if [ $count -lt $imp_num ]; then
			if [ $imp_flag -lt 9 ];then
				echo $i >> $imp_adapt_dataset
				((++imp_flag))
			else
				echo $i >> $imp_test_dataset
				((imp_flag=0))
			fi
		fi
		echo $i >> $train_ubm_dataset
	elif [ $flag -lt 9 ]; then
		echo $i >> $adapt_dataset
		((++flag))
	else
		echo $i >> $test_dataset
		((flag=0))
	fi
	((count++))
done


