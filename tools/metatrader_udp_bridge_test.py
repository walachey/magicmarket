# simple server test for the MetaTrader MQL4 bridge.
import socket
import struct

print "#######################################"
print "########### CENTRAL STATION ###########"
print "#######################################"
print "................ IS GO ................"

UDP_IP = "0.0.0.0"
UDP_PORT = 5005

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
sock.bind((UDP_IP, UDP_PORT))

class MessageTypes:
	MM_BRIDGE_UP = 1
	MM_BRIDGE_DOWN = 2
	MM_BRIDGE_TICK = 3
	MM_BRIDGE_ACCOUNTINFO = 4
	MM_BRIDGE_ORDERS = 5
	MM_BRIDGE_ERROR = 6

print ("Listening...")
while True:
	data, addr = sock.recvfrom(1024) # buffer size is 1024 bytes
	type, = struct.unpack("=c", data[:1])
	type = ord(type)

	if type == MessageTypes.MM_BRIDGE_UP:
		pair, timestamp = struct.unpack("=7si", data[1:])
		pair = pair[:-1]
		print ("[{:d}] Metatrader connected for pair {:s}!".format(timestamp, pair));
	elif type == MessageTypes.MM_BRIDGE_DOWN:
		pair, timestamp = struct.unpack("=7si", data[1:])
		pair = pair[:-1]
		print ("[{:d}] Metatrader DOWN for pair {:s}!".format(timestamp, pair));
	elif type == MessageTypes.MM_BRIDGE_TICK:
		pair, bid, ask, timestamp = struct.unpack("=7sddi", data[1:])
		pair = pair[:-1]
		print ("[{:d}] {:s} - {:10f} {:10f}".format(timestamp, pair, bid, ask));
	elif type == MessageTypes.MM_BRIDGE_ERROR:
		len, = struct.unpack("=i", data[1:(1 + 4)])
		msg, = struct.unpack("={}s", data[(1 + 4):])
		
		print ("\tERROR: {}".format(msg));
	elif type == MessageTypes.MM_BRIDGE_ACCOUNTINFO:
		leverage, balance, margin, free_margin = struct.unpack("=dddd", data[1:])
		print ("Account: [Margin {:f} free of {:f}] [Balance {:f}] [Leverage {:f}]".format(free_margin, margin, balance, leverage))
	elif type == MessageTypes.MM_BRIDGE_ACCOUNTINFO:
		one_msg_len = 56
		data_len = len(data)
		current = 1
		orders = 0
		total_profit = 0.0
		while current < data_len:
			
			pair, type, ticket_id, open_price, take_profit, stop_loss, timestamp_open, timestamp_expire, lots, profit = \
				struct.unpack("=7siidddiidd", data[current:(current + one_msg_len)])
			pair = pair[:-1]
			current += one_msg_len
			orders += 1
			total_profit += profit
		print ("\tOrders: {} open with total profit of {}".format(orders, total_profit))
	else:
		print ("Received unknown message.")