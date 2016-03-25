# Python script to get unix time of a date
import time
import datetime
import sys

for seconds in range(0,60):
	i = '03 10 2016 9 04 %02d' % seconds

	#i = raw_input()
	out = time.mktime(datetime.datetime.strptime(i, '%m %d %Y %H %M %S').timetuple())
	print(hex(int(out)))
