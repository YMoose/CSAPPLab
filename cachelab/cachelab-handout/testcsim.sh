#! /bin/bash
sbit=1
Eline=2
bbit=1
testfile=./traces/yi2.trace
outputfile0=Test_csim.test
outputfile1=Test_csim-ref.test
echo "diff result from running csim and csim-ref $testfile"
./csim -v -s $sbit -E $Eline -b $bbit -t $testfile > $outputfile0
./csim-ref -v -s $sbit -E $Eline -b $bbit -t $testfile > $outputfile1
diff $outputfile0 $outputfile1
rm $outputfile0 $outputfile1
