# collect ticks and add matplotlib local timestamp as float  

import zmq
from datetime import datetime
from matplotlib.dates import date2num

context = zmq.Context()
socket = context.socket(zmq.SUB)
binding_interface = "tcp://192.168.2.115:%s" % 1985
print "conn. on: '" + binding_interface + "'"
#socket.bind(binding_interface)
socket.connect("tcp://192.168.2.115:1986")

socket.setsockopt(zmq.SUBSCRIBE, "")


while True:
    string = socket.recv()
    print string + " " + str(date2num(datetime.now() ))   
    