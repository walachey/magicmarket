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
	
	
	##
	# management
	##
	
	# the listening socket
	metatrader_listener = None
	# the connection socket to metatrader
	metatraders = None
	# every metatrader might have a partial message
	current_messages_in = None
	def __init__(self):
		self.messages_in = Queue.Queue()
		self.messages_out = Queue.Queue()
		self.metatraders = []
		self.current_messages_in = {}
	
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
			self.receiveMessage()
			if self.metatraders: # might have died
				self.flushMessages()
			
	# when no client is connected, listen to new clients
	def listen(self):
		try:
			metatrader, client_address = self.metatrader_listener.accept()
			metatrader.settimeout(0.05)
			# we want speed over bandwidth
			if not metatrader.getsockopt(socket.IPPROTO_TCP, socket.TCP_NODELAY):
				metatrader.setsockopt(socket.IPPROTO_TCP, socket.TCP_NODELAY, 1)
			self.current_messages_in[metatrader] = ""
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
		error_traders = []
		_, ready_traders, _ = select.select([], self.metatraders, [], 0.01)
		if not ready_traders:
			return
			
		try:
			while not self.messages_out.empty():
				msg = self.messages_out.get(block=False)
				if not msg:
					return
				#print "@MT\t" + msg
				# always make sure to null-terminate messages
				msg = msg + '\00'
				for metatrader in ready_traders:
					try:
						metatrader.sendall(msg)
					except socket.timeout:
						# that's allrighty
						continue
					except:
						error_traders.append(metatrader)
						traceback.print_exc()
						continue
		except Queue.Empty:
			return # ok
		if error_traders:
			self.onConnectionError(error_traders)
		# except socket.timeout: # not ok
		
	
	def onConnectionError(self, error_traders):
		for et in error_traders:
			self.current_messages_in[et] = None
			try:
				self.metatraders.remove(et)
				et.shutdown(socket.SHUT_RDWR)
				et.close()
			except Exception as e:
				print "ERROR when removing Metatrader from list!"
				print str(e)
				traceback.print_exc()
		print "Metatrader disconnected. Now " + str(len(self.metatraders))
	
	def isReadyToReceive(self):
		# check if socket is ready for reading
		readers, writers, errors = select.select(self.metatraders, [], [], 0.05)
		if not readers:
			return False
		return True
	
	# gets a new message part from meta trader
	def receiveMessage(self):
		assert self.metatraders
		readers, writers, errors = select.select(self.metatraders, [], self.metatraders, 0.05)
		if errors:
			self.onConnectionError(errors)
		buffer_size = 256
		error_traders = []
		for metatrader in readers:
			current_msg = self.current_messages_in[metatrader]
			try:
				if True: # allow for a stream of data
					data = metatrader.recv(buffer_size)
					if not data: # I don't know how this can happen
						print "\thow can this happen"
						self.onConnectionError([metatrader])
						return
					
					current_msg += data
					
					while True:
						# check if this terminates a message
						terminating_position = current_msg.find('\00',0)
						if terminating_position == -1:
							break
						# new complete message?
						complete_message = current_msg[:terminating_position]
						current_msg = current_msg[terminating_position+1:]
						# add the new message to the queue
						self.messages_in.put(complete_message)

			except socket.timeout:
				print "timeout, but ignoring.."
				# that's alright
				continue
			except:
				print "ERROR when receiving:"
				traceback.print_exc()
				error_traders.append(metatrader)
				continue
			finally:
				self.current_messages_in[metatrader] = current_msg
				
		if error_traders:
			self.onConnectionError(error_traders)
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
		received = False
		
		while metatrader.hasMessage():
			received = True
			msg = metatrader.getMessage()
			station.sendMessage(msg)
			print "<- MT: " + msg


		while True:
			msg = station.getMessage()
			if not msg: break
		
			received = True
			
			# pipe to metatrader
			metatrader.sendMessage(msg)
			#print "-> MT: " + msg
			
			# route to all connected things, too
			station.sendMessage(msg)
		#print ".",
		if not received:
			time.sleep(0.01)

			