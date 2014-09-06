import Queue
import sys
import socket
import traceback
import time
import select
import zmq
import random

from threading import Thread

# the meta trader bridge uses TCP sockets for the connection
class MetaTraderBridge:
	###
	# connection info
	###
	listen_port = 1995
	
	
	##
	# current message state
	## 
	
	# messages coming FROM meta trader
	messages_in = None
	# messages going TO meta trader
	messages_out = None
	# the current message, not yet terminated by NULL
	current_message_in = None
	
	##
	# management
	##
	
	# the listening socket
	metatrader_listener = None
	# the connection socket to metatrader
	metatraders = None
	
	def __init__(self):
		self.messages_in = Queue.Queue()
		self.messages_out = Queue.Queue()
		self.current_message_in = ""
		self.metatraders = []
	
	def setup(self):
		print "Creating the MetaTrader TCP server..."
		self.metatrader_listener = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
		self.metatrader_listener.settimeout(0.5)
		
		# enable KEEPALIVE option
		if not self.metatrader_listener.getsockopt(socket.SOL_SOCKET, socket.SO_KEEPALIVE):
			self.metatrader_listener.setsockopt(socket.SOL_SOCKET, socket.SO_KEEPALIVE, 1)
		# make sure to be able to reuse addresses
		if not self.metatrader_listener.getsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR):
			self.metatrader_listener.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
		# we want speed over bandwidth
		if not self.metatrader_listener.getsockopt(socket.IPPROTO_TCP, socket.TCP_NODELAY):
			self.metatrader_listener.setsockopt(socket.IPPROTO_TCP, socket.TCP_NODELAY, 1)
		
		# bind must be called after the socket options
		self.metatrader_listener.bind(('', self.listen_port)) # needs a 2-tuple. God knows why
		self.metatrader_listener.listen(1)
		
		
	def execute(self):
		self.listen()
		
		if self.metatraders:
			readers, writers, errors = select.select([], [], self.metatraders, 0.5)
			if errors:
				print "ERROR in socket:"
				print str(errors)
				self.onConnectionError(errors)
			
			self.receiveMessage()
			if self.metatraders: # might have died
				self.flushMessages()
			
	# when no client is connected, listen to new clients
	def listen(self):
		try:
			metatrader, client_address = self.metatrader_listener.accept()
			metatrader.settimeout(0.5)
			self.metatraders.append(metatrader)
			# at this point, filters (for the client address) could be implemented
		except socket.timeout:
			# everything OK
			return
		except:
			traceback.print_exc()
			return
		print "Metatrader connected. Now " + str(len(self.metatraders))
		
	
	# sends out all outgoing messages
	def flushMessages(self):
		assert self.metatraders
		
		try:
			while not self.messages_out.empty():
				msg = self.messages_out.get(block=False)
				if not msg:
					return
				print "@MT\t" + msg
				# always make sure to null-terminate messages
				msg = msg + '\00'
				for metatrader in self.metatraders:
					metatrader.sendall(msg)
		except Queue.Empty:
			return # ok
		# except socket.timeout: # not ok
		except:
			self.onConnectionError()
			traceback.print_exc()
	
	def onConnectionError(self, error_traders):
		for et in error_traders:
			self.metatraders.remove(et)
		print "Metatrader disconnected. Now " + str(len(self.metatraders))
	
	def isReadyToReceive(self):
		# check if socket is ready for reading
		readers, writers, errors = select.select(self.metatraders, [], [], 0.5)
		if not readers:
			return False
		return True
	
	# gets a new message part from meta trader
	def receiveMessage(self):
		assert self.metatraders
		readers, writers, errors = select.select(self.metatraders, [], [], 0.5)
		buffer_size = 8
		
		for metatrader in readers:
			try:
				while True: # allow for a stream of data
					if not self.isReadyToReceive():
						return
					data = metatrader.recv(buffer_size)
					if not data: # I don't know how this can happen
						self.onConnectionError([metatrader])
						return

					# check if this terminates a message
					terminating_position = data.find('\00')
					old_message_len = len(self.current_message_in)
					
					self.current_message_in += data
					
					# new complete message?
					if terminating_position != -1:
						truncation_position = old_message_len + terminating_position
						complete_message = self.current_message_in[:truncation_position]
						self.current_message_in = self.current_message_in[truncation_position+1:]
						# add the new message to the queue
						self.messages_in.put(complete_message)

			except socket.timeout:
				# that's alright
				return
			except:
				print "ERROR when receiving:"
				traceback.print_exc()
				self.onConnectionError([metatrader]) # something happened
				return
	
	# public interface
	def isConnected(self):
		return self.metatraders
	def hasMessage(self):
		return not self.messages_in.empty()
	def getMessage(self):
		return self.messages_in.get()
	def sendMessage(self, msg):
		self.messages_out.put(msg)

class ZmqBridge:
	
	# subscribing port
	port_in = 1985
	# publishing port
	port_out = 1986
	# zmq sockets
	publisher = None
	subscriber = None
	context = None
	
	# thread hack workaround to get this work on windows...
	subscriber_thread = None
	message_queue = None
	
	def setup(self):
		print "Setting up ZMQ bridge..."
		self.context = zmq.Context()
		
		self.subscriber = self.context.socket(zmq.SUB)
		self.subscriber.bind("tcp://*:%s" % self.port_in)
		self.subscriber.setsockopt(zmq.SUBSCRIBE, "")
		
		self.publisher = self.context.socket(zmq.PUB)
		self.publisher.bind("tcp://*:%s" % self.port_out)
		
		self.poller = zmq.Poller()
		self.poller.register(self.subscriber, zmq.POLLIN)
		
		# threadhack
		self.message_queue = Queue.Queue()
		self.subscriber_thread = Thread(target=self.receiveMessages)
		self.subscriber_thread.start()
	
	def receiveMessages(self): # in a thread
		while True:
			msg = self.subscriber.recv()
			self.message_queue.put(msg)
		
	def getMessage(self):
		try:
			msg = self.message_queue.get_nowait()
			return msg
		except Queue.Empty:
			return None
	
	def sendMessage(self, msg):
		self.publisher.send(msg, zmq.NOBLOCK)
		
if __name__ == "__main__":
	metatrader = MetaTraderBridge()
	metatrader.setup()
	station = ZmqBridge()
	station.setup()
	print "Running..."
	while True:
		metatrader.execute()

		while metatrader.hasMessage():
			msg = metatrader.getMessage()
			#station.sendMessage(msg)
			print "Msg: '" + msg + "'"
		if metatrader.isConnected():
			#metatrader.sendMessage("cmd|David|testuid maek magic")
			pass
		faketick = True
		if faketick:
			low = 0.75 + random.random()
			high = low + random.random() / 2
			msg = "tick David EURUSD " + str(low) + " " + str(high) + " 1409321828"
			
			station.sendMessage('tick David EURUSD 1.29497 1.29518 1409960066')
			station.sendMessage('orders David [{"pair":"EURGBP", "type":"0", "ticket_id":"40564195", "open_price":"0.81645", "take_profit":"0", "stop_loss":"0", "open_time":"1400194678","expire_time":"0", "lots":"0.01", "profit":"-38.39"}, {"pair":"EURUSD", "type":"0", "ticket_id":"40760473", "open_price":"1.37141", "take_profit":"0", "stop_loss":"0", "open_time":"1400669257", "expire_time":"0", "lots":"0.01", "profit":"-76.44"}, {"pair":"EURUSD", "type":"0", "ticket_id":"42929335", "open_price":"1.35437", "take_profit":"0", "stop_loss":"0", "open_time":"1405512967", "expire_time":"0", "lots":"0.01", "profit":"-59.4"}]')
			time.sleep(4.0)
		msg = ""
		msg = station.getMessage()
		if msg:
			print "ZMQ says:\n\t" + msg
			station.sendMessage(msg)
		print ".",
		time.sleep(0.5)