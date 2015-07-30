import os
from numpy import correlate
from datetime import datetime, timedelta

import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
import matplotlib.image as mpimg
import numpy
import math
import json
import io

def evalMood():
	plt.clf()

	data = json.load(open("../run/saves/moodfun.json"))
	#print [x for x in open("../run/saves/moodfun.json", "r")]
	x_values = None
	#fig, ax = plt.subplots()
	colors = {
		"BUY" : "b",
		"SELL" : "r",
		"lika" : "#00ff00",
		"dumbo" : "#00ffff",
		"TA_RSI" : "#009999",
		"TA_TSI" : "#999999",
		"ajeet" : "w",
		"atama" : "#aa99aa",
		"bob" : "#660066"
	}

	for AI in data:
		if not x_values:
			x_values = range(len(data[AI]))
		linewidth = 1.0
		if AI in ["BUY", "SELL", "lika"]:
			linewidth = 2.0
		color = colors[AI] if AI in colors else "k"
		plt.plot(x_values, data[AI], color=color, label=AI, linewidth=linewidth)
		
		if AI == "lika":
			markers_xbuy = []
			markers_xsell = []
			for i in range(len(data[AI])):
				val = data[AI][i]
				if val >= 1.0:
					markers_xbuy.append(i)
				elif val <= -1.0:
					markers_xsell.append(i)
			plt.plot(markers_xbuy, [+1.1 for x in markers_xbuy], 'kv')
			plt.plot(markers_xsell, [-1.1 for x in markers_xsell], 'k^')
	plt.ylim([-1.5, +1.5])
	plt.legend()
	#yticks = [0.5 + x for x in range(0,len(stocks))]
	#ax.set_yticks(yticks)
	#ax.set_yticklabels(labels)
	plt.gcf().set_size_inches(40, 10)
	plt.tight_layout()
	
	buf = io.BytesIO()
	plt.gcf().savefig(buf, format="png")
	buf.seek(0)
	return buf
	
if __name__ == "__main__":
	data = evalMood()
	from PIL import Image
	im = Image.open(data)
	im.save("VM_mood.png")