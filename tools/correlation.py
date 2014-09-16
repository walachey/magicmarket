from converter import *
import os
from numpy import correlate
from datetime import datetime, timedelta

import matplotlib.pyplot as plt
import matplotlib.image as mpimg
import numpy
import math

import signal_smooth as smooth

def shiftCorrelate(x, y):
	result = [None] * len(x)

	midy = len(y) // 2
	righty = len(y) - midy
	range_left = midy
	range_right = righty
	
	if True:
		range_left = midy // 2
		range_right = midy // 2

	
	for i in range(len(x)):
		start_i = max(i - range_left, 0)
		end_i = min(i + range_right, len(x)-1)
		
		start_k = midy + (start_i-i)
		end_k = midy + (end_i-i)
		
		assert (end_k - start_k) == (end_i - start_i)
		
		seq_i = x[start_i:end_i]
		seq_k = y[start_k:end_k]
		
		result[i] = numpy.corrcoef([seq_i, seq_k])[0][1]
		if math.isnan(result[i]):
			result[i] = 0
	return result
	
class Stock:
	name = None
	data = None
	path = None
	sequence = None
	
	def __init__(self, name):
		self.data = []
		self.name = name
		
	def load(self, filename):
		self.path = filename
		
		with open(filename, "rb") as file:
			while True:
				tick = readTick(file)
				if not tick:
					break
				self.data.append(tick)
			
	def getEarliest(self):
		current = None
		for tick in self.data:
			if not current or tick.time < current.time:
				current = tick
		return current
	def getLatest(self):
		current = None
		for tick in self.data:
			if not current or tick.time > current.time:
				current = tick
		return current
		
	def prepareSequence(self, date_from, date_to, timestep):
		self.sequence = []
		
		current_index = 0
		while date_from <= date_to:
			while self.data[current_index].time < date_from:
				current_index += 1
			self.sequence.append(self.data[current_index-1])
			date_from += timestep
			
stocks = []

# get all stock folders
MON = 9
DAY = 11
stock_path = "../run/saves"
today_filename = "2014-" + str(MON) + "-" + str(DAY) + ".ticks"
for root, dirs, files in os.walk(stock_path):
	for dir in dirs:
		stock = Stock(dir)
		try:
			stock.load(stock_path + "/" + dir + "/" + today_filename)
			stocks.append(stock)
		except:
			print "Invalid stock: " + dir
# sanity check - stocks + tick count
print "####### LOADED STOCKS #######"
print "#NAME#\t#TICKS#\t#FROM#\t\t\t#TO#"
for stock in stocks:
	print stock.name + "\t" + str(len(stock.data)) + "\t" + str(stock.getEarliest().time) + "\t" + str(stock.getLatest().time)
	
# create good sequences for the dates
TIME_ZONE_OFFSET = timedelta(0, hours=3)
timestep = timedelta(0, 10)
start_time = datetime(2014, MON, DAY, 10, 0, 0) + TIME_ZONE_OFFSET
end_time = datetime(2014, MON, DAY, 16, 0, 0) + TIME_ZONE_OFFSET

print "-------------------------------------------"
print "SEQUENCE FROM " + str(start_time) + " TO " + str(end_time)
print "#NAME#\t#TICKS#"
for stock in stocks:
	stock.prepareSequence(start_time, end_time, timestep)
	print stock.name + "\t" + str(len(stock.sequence))
	
# do the cross correlation of the data with an convolution
print "--------------------------------------------"
print "CROSS CORRELATING...."
eurusd = [stock for stock in stocks if stock.name == "EURUSD"][0]

def getCorrData(stock):
	seq = [x.getMid() for x in stock.sequence]
	gradient = numpy.gradient(seq)
	#gradient = numpy.diff(seq,n=1)
	return gradient
eurusd_data = getCorrData(eurusd)

print "#NAMES#\t\t#MEAN#\t#MEDIAN#"
# setup data
correlations = []
coefficients = []
markers_min = [[],[]]
markers_max = [[], []]
total_min = 0
total_max = 0
stock_index = 0

# try to some better adjustment by using the EURUSD-self-correlation as the comparison
self_corr = numpy.correlate(eurusd_data, eurusd_data, mode="same")
self_min = numpy.min(self_corr)
self_max = numpy.max(self_corr)
print "SELFMINMAX\t" + str(self_min) + "\t" + str(self_max)
def signum(x):
	if x < 0: return -1.0
	if x > 0: return +1.0
	return +0.0
# next try
total_max = 0
total_min = 0
# aaaand correlate
for stock in stocks:
	comp_data = getCorrData(stock)
	#correlation = numpy.correlate(eurusd_data, comp_data, mode="same")
	correlation = shiftCorrelate(eurusd_data, comp_data)
	coeff = numpy.corrcoef([eurusd_data, comp_data])[0][1]
	
	# adjust for better comparability
	# correlation = [(x-self_min) / (self_max-self_min) for x in correlation]
	
	corr_max = numpy.max(correlation)
	corr_min = numpy.min(correlation)
	mean = numpy.mean(correlation)
	if mean > total_max: total_max = mean 
	if mean < total_min: total_min = mean 
	
	if abs(corr_max) > abs(corr_min):
		coeff = corr_max
	else:
		coeff = corr_min
	if stock.name == "USDDKK":
		print stock.data
	print stock.name
	#print "\tav\t" + str(numpy.mean(correlation)) + "\t" + str(numpy.median(correlation))
	print "\tmm\t(" + str(corr_min) + ")/(" + str(corr_max) + ")" 
	print "\tco\t" + str(coeff)
	
	# save markers
	max_ind = numpy.where(numpy.array(correlation) == corr_max)[0]
	min_ind = numpy.where(numpy.array(correlation) == corr_min)[0]
	if coeff > -0.2 or True:
		for m in max_ind:
			markers_max[0].append(m)
			markers_max[1].append(stock_index + 0.5)
	if coeff < +0.2 or True:
		for m in min_ind:
			markers_min[0].append(m)
			markers_min[1].append(stock_index + 0.5)
	
	# normalize for output
	normalized = correlation
	#normalized = [x for x in correlation]
	#normalized = [(x - min) / (max - min) for x in correlation]
	normalized = smooth.smooth(numpy.array(normalized), 5, "blackman")
	correlations.append(normalized)
	coefficients.append(coeff)

	stock_index += 1
	
# and generate an image
fig, ax = plt.subplots()
heatmap = ax.pcolormesh(numpy.array([x for x in reversed(correlations)]),
	#norm=True,
	#shading="gouraud",
	#edgecolors="face",
	#vmin=2*total_min, vmax=2*total_max,
	vmin=-1.0, vmax=1.0,
	cmap=plt.get_cmap("gray")
	)
labels = []
for i in range(len(stocks)):
	labels.append(str(round(coefficients[i],2)) + " " + stocks[i].name)
yticks = [0.5 + x for x in range(0,len(stocks))]
ax.set_yticks(yticks)
ax.set_yticklabels(labels)
ax.scatter(markers_min[0], markers_min[1])
ax.scatter(markers_max[0], markers_max[1], c="r")
plt.tight_layout()
plt.savefig("heatmap-" + str(MON) + "-" + str(DAY) + ".png")