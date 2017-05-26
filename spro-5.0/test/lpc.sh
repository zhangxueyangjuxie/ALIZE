#!/bin/sh -x
#
# Test slpc program
#
# $Author: ggravier $
# $Date: 2007-01-19 17:35:24 +0100 (Fri, 19 Jan 2007) $
# $Revision: 132 $
#


prog=slpc

echo "checking ${prog}:"

if test ! -x ../${prog}; then
  echo "  you should probably build ${prog} before running test..."
  exit 1
fi

mkdir -p log

#
# check usage 
#
../${prog} --version > log/${prog}.stdout 2> log/${prog}.stderr
# diff log/${prog}.stdout ref/${prog}.usage > /dev/null 2> /dev/null
if test $? -eq 0 -a x`awk '{print NR}' log/${prog}.stderr | tail -1` = x; then
  echo "  build test passed"
else
  echo "  build test failed (see test/log/${prog}.stderr for details)"
  exit 1
fi
\rm -f log/*

#
# run test  
#
../${prog} dat/know1.lin log/lpc.1 > log/${prog}.stdout 2> log/${prog}.stderr
../scompare -q log/lpc.1 ref/lpc.1 > /dev/null 2> /dev/null
if test $? -eq 0 -a x`awk '{print NR}' log/${prog}.stderr | tail -1` = x; then
  echo "  run test passed"
else
  echo "  run test failed (see test/log/${prog}.stderr for details)"
  exit 1
fi
\rm -f log/*

rmdir log
