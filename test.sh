#!/bin/bash -x

#timeout 5 "./a.out > output.log"
timeout 5 "/home/oskim/workspace/temp/check > output.log"
RETVAL=$?

if [ $RETVAL -eq 0 ]
then
    echo "ok"
    echo $ret >> return_message.log
    #exit 0
elif [ $RETVAL -eq 124 ]
then
    echo "timeout error"
else
    #echo "not ok"
    echo "abnormal error" >> critical_error.log
    #exit 1
fi
