#!/usr/bin/env python
import rosbag
import rospy
import sys
import os

from sensor_msgs.msg import Image

if len(sys.argv) < 4:
  print "Usage: {} <bag_name> <topic> <timeshift>".format(os.path.basename(sys.argv[0]))
  exit()

input_bag_name = sys.argv[1]
topic_name = sys.argv[2]
time_shift = rospy.Duration(float(sys.argv[3]))

try:
  input_bag = rosbag.Bag(input_bag_name)
  bag_loaded = True
except Exception:
  print "Could not open bag {}".format(input_bag_name)
  bag_loaded = False

if bag_loaded:
  name, ext = os.path.splitext(input_bag_name)
  output_bag_name = "{name}_timeshifted{ext}".format(name=name, ext=ext)
  output_bag = rosbag.Bag(output_bag_name, 'w')

  for topic, msg, t in input_bag.read_messages():
    if topic == topic_name:
      msg.header.stamp += time_shift
      output_bag.write(topic, msg, t)
    else:
      output_bag.write(topic, msg, t)

  output_bag.close()
  input_bag.close()
