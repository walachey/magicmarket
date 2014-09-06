import zmq
import random
import sys
import time

port = "1986"
if len(sys.argv) > 1:
    port =  sys.argv[1]
    port = int(port)

context = zmq.Context()
socket = context.socket(zmq.PUB)
socket.bind("tcp://*:%s" % port)
#socket.setsockopt(zmq.SUBSCRIBE, "")

sub = context.socket(zmq.SUB)
sub.bind("tcp://*:1985")
sub.setsockopt(zmq.SUBSCRIBE, "")

while True:
	
	#string = socket.recv()
	socket.send("TEST TEST ???")
	print "..heartbeat"
	string = ""
	try:
		string = sub.recv(zmq.NOBLOCK)
	except:
		time.sleep(1)
	if string:
		print "RECEIVED:\n\t" + string
	#print "got: \n\t" + string