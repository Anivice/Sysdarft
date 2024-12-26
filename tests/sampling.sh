#!/usr/bin/env bash

AVERAGE=0;
ALL=30;
for ((i=0;i<$ALL;i++));
do
    sum=0;
    count=0;

    for DELAY in $(./test.processor | awk ' { print $3 } ' | grep ns);
    do
        ((sum += ${DELAY/ns/}));
        ((count += 1));
    done;

    AVERAGE=$(echo "scale=10;$AVERAGE + "$(echo "scale=10; $sum / $count" | bc -q) / $ALL | bc -q);
    echo "Current average is $(echo "scale=10;" $sum / $count | bc -q)";
done;

echo "All tested instances with the $AVERAGE"
