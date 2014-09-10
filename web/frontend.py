# Make sure your gevent version is >= 1.0
import gevent
from gevent.wsgi import WSGIServer
from gevent.queue import Queue
import zmq
from flask import Flask, Response, render_template

import time

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

if __name__ == "__main__":
	app.debug = True

	app.jinja_env.line_statement_prefix = '#'
	app.jinja_env.line_comment_prefix = '##'

	app.run("", 5000, threaded=True)
	# server.serve_forever()