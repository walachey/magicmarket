# Make sure your gevent ver#sion is >= 1.0
import gevent
from gevent.wsgi import WSGIServer
from gevent.queue import Queue
import zmq
from flask import Flask, Response, render_template
from flask import send_file

from flask import make_response
from functools import wraps, update_wrapper
from datetime import datetime
 
def nocache(view):
	@wraps(view)
	def no_cache(*args, **kwargs):
		response = make_response(view(*args, **kwargs))
		response.headers['Last-Modified'] = datetime.now()
		response.headers['Cache-Control'] = 'no-store, no-cache, must-revalidate, post-check=0, pre-check=0, max-age=0'
		response.headers['Pragma'] = 'no-cache'
		response.headers['Expires'] = '-1'
		return response
	return update_wrapper(no_cache, view)

import time

import imp
import io
vm_eval = imp.load_source("vm_eval", "../tools/vm_eval.py")

class ZMQTicker:
	subscriptions = None
	context = None
	subscriber = None
	
	def __init__(self, host):
		self.subscriptions = []
		
		self.context = zmq.Context()
		self.subscriber = self.context.socket(zmq.SUB)
		self.subscriber.setsockopt(zmq.SUBSCRIBE, "")
		self.subscriber.connect(host)
		
	def get(self):
		msg = self.subscriber.recv()
		return msg

# SSE "protocol" is described here: http://mzl.la/UPFyxY
class ServerSentEvent(object):

    def __init__(self, data):
        self.data = data
        self.event = None
        self.id = None
        self.desc_map = {
            self.data : "data",
            self.event : "event",
            self.id : "id"
        }

    def encode(self):
        if not self.data:
            return ""
        lines = ["%s: %s" % (v, k) 
                 for k, v in self.desc_map.iteritems() if k]
        
        return "%s\n\n" % "\n".join(lines)

app = Flask(__name__)

@app.route("/")
def index():
    return(render_template('frontend.html'))

@app.route("/debug")
def debug():
    return "Currently %d subscriptions" % len(subscriptions)


@app.route("/ticker")
def subscribe():
	def gen():
		ticker = ZMQTicker("tcp://127.0.0.1:1986")
		try:
			while True:
				result = ticker.get()
				ev = ServerSentEvent(str(result))
				yield ev.encode()
		except GeneratorExit: # Or maybe use flask signals
			pass
	
	response = Response(gen(), mimetype="text/event-stream")
	#response.headers.add('content-length', "10000")
	return response

@app.route("/moodgraph.png")
@nocache
def generate_moodgraph():
	data = vm_eval.evalMood()
	return send_file(data, as_attachment=False)
	
if __name__ == "__main__":
	app.debug = True

	app.jinja_env.line_statement_prefix = '#'
	app.jinja_env.line_comment_prefix = '##'

	app.run("", 5000, threaded=True)
	# server.serve_forever()