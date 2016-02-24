import zmq
import random
import sys
import time
import six
import pandas
import Expert_pb2

class ExternalAgent(object):
	
	required_variables = None

	def makeReply(self, type):
		reply = Expert_pb2.ExpertMessage()
		reply.type = type
		return reply
		
	def send(self, message):
		self.socket.send(message.SerializeToString())

	def run(self, port):
		self.context = zmq.Context()
		self.socket = self.context.socket(zmq.REP)
		self.socket.bind("tcp://*:%s" % str(port))
		
		print ("Waiting for connection...")
		sys.stdout.flush()
		
		while True:
			message_string = self.socket.recv()
			message = Expert_pb2.ExpertMessage()
			message.ParseFromString(message_string)
			
			if message.type == Expert_pb2.ExpertMessage.getPrediction:
				x = message.variables
				y = self.predict(x)
				
				reply = self.makeReply(Expert_pb2.ExpertMessage.getPrediction)
				reply.estimation.mood = y[0]
				reply.estimation.certainty = y[1]
				self.send(reply)
			elif message.type == Expert_pb2.ExpertMessage.getName:
				print ("Connection established. PONGing name.")
				sys.stdout.flush()
				
				name = self.getName()
				reply = self.makeReply(Expert_pb2.ExpertMessage.getName)
				reply.name = name
				self.send(reply)
			elif message.type == Expert_pb2.ExpertMessage.getRequiredVariables:
				variables = self.getRequiredVariables()
				reply = self.makeReply(Expert_pb2.ExpertMessage.getRequiredVariables)
				for var_name in variables:
					reply.variableNames.append(var_name)
				self.send(reply)
			elif message.type == Expert_pb2.ExpertMessage.getProvidedVariables:
				variables = self.getProvidedVariables()
				reply = self.makeReply(Expert_pb2.ExpertMessage.getProvidedVariables)
				for var_name in variables:
					reply.variableNames.append(var_name)
				self.send(reply)
			elif message.type == Expert_pb2.ExpertMessage.informations:
				reply = self.makeReply(Expert_pb2.ExpertMessage.informations)
				reply.information.isExecutive = self.isExecutive()
				reply.information.noPrediction = self.predict == ExternalAgent.predict
				self.send(reply)
			elif message.type == Expert_pb2.ExpertMessage.reset:
				self.send(self.makeReply(Expert_pb2.ExpertMessage.reset)) # echo
				self.reset()
			elif message.type == Expert_pb2.ExpertMessage.shutdown:
				self.send(self.makeReply(Expert_pb2.ExpertMessage.shutdown)) # echo
				print ("Shutdown signal received.")
				if not self.queryRejectShutdown():
					break
			else:
				print ("Invalid message received!")
	# Called once for every new day.
	def reset(self):
		pass
	def getName(self):
		return "Dummy"
	# Called once on init and returns whether this agent's opinion matters.
	def isExecutive(self):
		return True
	# Called when receiving shutdown signal. True -> do not stop main loop.
	def queryRejectShutdown(self):
		return False
	# Called once on init and returns the required variables.
	def getRequiredVariables(self):
		return self.required_variables or []
	def predict(self, x):
		return (0.0, 0.0)
	# Updates the provided variables using the required va
	def update(self, x):
		pass
	def getProvidedVariables(self):
		pass
	def setRequiredVariables(self, names, variable_mapping):
		if isinstance(variable_mapping, six.string_types):
			variable_mapping = pandas.read_csv(variable_mapping, sep="\t")
		mapping = variable_mapping.to_dict(orient="list")
		
		self.required_variables = []
		
		for name in names:
			mapping_idx = mapping["name"].index(name)
			mapped = mapping["description"][mapping_idx]
			self.required_variables.append(mapped)
		
		
		
		
