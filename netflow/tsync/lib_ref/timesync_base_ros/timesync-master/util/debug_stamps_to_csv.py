#!/usr/bin/env python
import rosbag
import rospy
import sys
import os

if len(sys.argv) < 3:
  print "Usage: {} <bag_name> <debug_info topic>".format(os.path.basename(sys.argv[0]))
  print "Output: bag_name.csv"
  exit()

input_bag_name = sys.argv[1]

try:
  input_bag = rosbag.Bag(input_bag_name)
  bag_loaded = True
except Exception:
  print "Could not open bag {}".format(input_bag_name)
  bag_loaded = False

if bag_loaded:
  name, ext = os.path.splitext(input_bag_name)

  out_file_name = "{name}.csv".format(name=name)
  print "Writing to {}".format(out_file_name)
  out_file = open(out_file_name, 'w')

  for topic, msg, t in input_bag.read_messages(topics=[sys.argv[2]]):
    out_file.write('{} {:.12f} {:.12f} {:.12f}\n'.format(msg.seq, msg.ros_time, msg.sensor_time, msg.corrected_timestamp))

  out_file.close()
  input_bag.close()
