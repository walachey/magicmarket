# collect ticks and add matplotlib local timestamp as float  

import zmq

print "#######################################"
print "########### CENTRAL STATION ###########"
print "#######################################"
print "................ IS GO ................"

context = zmq.Context()
subs = context.socket(zmq.SUB)

binding_interface = "tcp://*:%s" % 1985
print "Binding on: '" + binding_interface + "'"
subs.bind(binding_interface)

pubs = context.socket(zmq.PUB)
pubs.bind("tcp://*:1986")

subs.setsockopt(zmq.SUBSCRIBE, "")


while True:
	string = subs.recv()
	print "Piping: \n\t" + string
	pubs.send(string)
    