# collect ticks and add matplotlib local timestamp as float  

import zmq
#from datetime import datetime
#from matplotlib.dates import date2num
import time

context = zmq.Context()
socket = context.socket(zmq.SUB)

socket.connect("tcp://127.0.0.1:1985")

socket.setsockopt(zmq.SUBSCRIBE, "")


while True:
	msg = socket.recv()
	print msg  

    