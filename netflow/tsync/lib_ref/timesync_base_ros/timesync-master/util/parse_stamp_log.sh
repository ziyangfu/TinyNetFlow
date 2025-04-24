#!/bin/bash
	if [  $# -le 0 ] 
	then 
    echo "Usage: $0 <log to be parsed>"
		exit 1
	fi 

OUT=$(echo "$1" | sed 's/\.[^.]*$/_parsed&/')
perl -0777 -pe 's/\e\[0m//g' $1 > $OUT
echo "1/6"
perl -i -pe 's/.+\]: //g' $OUT
echo "2/6"
perl -0777 -i -pe 's/\nSENSOR_TIMESTAMP/ SENSOR_TIMESTAMP/g' $OUT
echo "3/6"
perl -0777 -i -pe 's/\nROS_TIME/ ROS_TIME/g' $OUT
echo "4/6"
perl -i -pe 's/\,/\./g' $OUT
echo "5/6"
sed -i '/delta_sensor/!d' $OUT
echo "6/6"
