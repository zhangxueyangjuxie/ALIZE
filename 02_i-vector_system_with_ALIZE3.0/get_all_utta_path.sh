#!/bin/bash
#descript : get dataset base information
#author : xueyang
#data : 2017-05-08

#create path file
if [ -f "data/data.lst" ]; then
	rm data/data.lst
fi
touch data/data.lst

#iterator find .wav files
function get_wav()
{
	for element in `ls $1`
	do
		dir_or_file=$1"/"$element
		if [ -d $dir_or_file ]; then
			get_wav $dir_or_file
		else
			if [ "${dir_or_file##*.}"x = "wav"x ]; then
				echo $dir_or_file >> data/data.lst
			fi
		fi
	done	
}
get_wav $1
