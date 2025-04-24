#!/usr/bin/env python
import rosbag
import rospy
import sys
import os
import PyKDL

if len(sys.argv) < 4:
  print "Usage: {} <bag_name> <parent frame> <child frame>".format(os.path.basename(sys.argv[0]))
  print "Output file: bag_name.csv - each line: <stamp> <yaw angle>"
  exit()

input_bag_name = sys.argv[1]
parent_frame = sys.argv[2]
child_frame = sys.argv[3]

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

  counter = 0
  for topic, msg, t in input_bag.read_messages(topics=['tf']):
    for t in msg.transforms:
      if t.header.frame_id == parent_frame and t.child_frame_id == child_frame:
        counter += 1
        quat = PyKDL.Rotation.Quaternion(*[ getattr(t.transform.rotation, i) for i in ['x', 'y', 'z', 'w'] ])
        yaw = quat.GetRPY()[2]
        out_file.write('{:.12f} {:.12f}\n'.format(t.header.stamp.to_sec(), yaw))

  out_file.close()
  input_bag.close()
  print("Wrote {} entries.".format(counter))
