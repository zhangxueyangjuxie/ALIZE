#!/bin/bash/
#clear data/lbl/*  data/prm/*  gmm/*  log/*  lst/*  ndx/*   res/*  speaker_scores/*
#data/data.lst  data/name.lst

[ -d data/lbl ] && rm -r data/lbl
mkdir data/lbl

[ -d data/prm ] && rm -r data/prm
mkdir data/prm

[ -d gmm ] && rm -r gmm
mkdir gmm

[ -d log ] && rm -r log
mkdir log

[ -d  lst ] && rm -r  lst
mkdir  lst

[ -d  ndx ] && rm -r  ndx
mkdir  ndx

[ -d  res ] && rm -r  res
mkdir  res

[ -d  speaker_scores ] && rm -r  speaker_scores
mkdir  speaker_scores

[ -f  data/data.lst ] && rm data/data.lst
[ -f  data/name.lst ] && rm data/name.lst