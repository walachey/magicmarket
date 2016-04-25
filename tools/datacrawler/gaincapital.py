import zipfile
import requests
import shutil
import tempfile

import pytz
import datetime
import csv
import time
import io
import time
import calendar

#http://ratedata.gaincapital.com/2014/12%20December/EUR_USD_Week1.zip
base_url = "http://ratedata.gaincapital.com/"
currencies = ["EUR_USD"]#["GBP_USD", "USD_JPY", "USD_CHF", "EUR_CHF", "EUR_GBP"]
years = [str(year) for year in [2015]]
months = ["January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December"]
months = ["{:02d} {}".format(1 + index, month) for (index, month) in enumerate(months)]
#months = months[0:1]
skipheaders = 1

original_timezone = pytz.timezone("America/New_York")

class Tick:
	timestamp = None
	bid = None
	ask = None
	
	def to_list(self):
		return [self.timestamp, self.bid, self.ask]
		
def do_the_crawl(target, currency, year, month, week):
	request_url = "".join((base_url,
		year, "/",
		month, "/",
		currency, "_Week", str(week),
		".zip"))

	response = requests.get(request_url)
	
	if response.status_code != 200:
		print ("\tFail: ..." + request_url[-30:])
		return

	with zipfile.ZipFile(io.BytesIO(response.content)) as zf:
		filename = zf.namelist()[0]
		file     = zf.open(filename)
		reader   = csv.reader(file, delimiter=",")
		
		row_count = 0
		for row in reader:
			row_count += 1
			if row_count <= skipheaders: continue
			tick = Tick()
			stringtime = row[3]
			dt = None
			if "." in stringtime:
				dt = datetime.datetime.strptime(stringtime.rstrip('0'), "%Y-%m-%d %H:%M:%S.%f")
			else:
				dt = datetime.datetime.strptime(stringtime, "%Y-%m-%d %H:%M:%S")
			local_dt = original_timezone.localize(dt)
			gmt_time = local_dt.astimezone(pytz.utc)
			gmt_timestamp = calendar.timegm(gmt_time.timetuple()) + gmt_time.microsecond / 1000000.0 
			tick.timestamp = gmt_timestamp
			tick.bid = float(row[4])
			tick.ask = float(row[5])
			
			target.append(tick)
	time.sleep(1)

print ("Crawling...")

import gc
for currency in currencies:
	
	for year in years:
		target_name = currency + "_" + year
		target = []
		
		for index, month in enumerate(months):
			for week in range(5):
				do_the_crawl(target, currency, year, month, 1 + week)
			
			print (month + "\t->\t" + str(len(target)) + "ticks")
		
		print("..saving ", len(target), " ticks")
		with open('gaincapital_' + target_name + '.csv', 'wb') as fp:
			writer = csv.writer(fp, delimiter=',')
			writer.writerows([tick.to_list() for tick in target])
		# and clear memory
		target = None
		gc.collect()

print "Done!"

