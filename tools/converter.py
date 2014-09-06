import struct
from datetime import datetime

class Tick:
	time = None
	bid = None
	ask = None
	
	def __repr__(self):
		return "[" + str(self.time) + "\t" + str(self.bid) + "\t" + str(self.ask) + "]"
		
	def getMid(self):
		return (self.bid + self.ask) / 2.0
	
def readTick(file):
	data = file.read(1)
	if not data:
		return None
	version = struct.unpack("B", data)[0]
	tick = Tick()

	if version == 1:
		tick.time = datetime.fromtimestamp(struct.unpack("Q", file.read(8))[0])
		tick.bid = struct.unpack("d", file.read(8))[0]
		tick.ask = struct.unpack("d", file.read(8))[0]
		
	if not tick.time or not tick.bid or not tick.ask:
		return None
	return tick

if __name__ == "__main__":
	filename = "../MagicMarket2/run/saves/EURUSD/2014-9-1.ticks"
	with open(filename, "rb") as file:
		print "Reading file.."
		while True:
			tick = readTick(file)
			if not tick:
				break
			print str(tick)