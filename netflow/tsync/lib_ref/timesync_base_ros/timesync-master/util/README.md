# Utils

Run a node which uses this timesync class. Record the debug messages in a log file (using stdin redirection).

Then, parse the log with parse_stamp_log.sh, or use debug_stamps_to_csv.py. The resulting log file may be opened in Matlab with load_matlab.m for drift examination.

Be sure to compile the mex functions swallow_csv.cpp and medfilt.cpp (in Matlab, enter: mex <cpp_file>).
