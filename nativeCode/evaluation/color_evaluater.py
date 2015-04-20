#!/usr/bin/python2

import commands
import datetime
import os
import sys
import itertools
import shutil
import socket
import subprocess
import tempfile
import yaml
import joblib
import lockfile

DEVNULL = open(os.devnull, 'wb')
HOME = os.path.expanduser("~")
PATH = "../../test/files/03b0c66/unprocessed/"
HOSTNAME = socket.gethostname()

class Storage():
	pass

params = Storage()

#float enabled range generator with max precision = 10.e-5
def myRange(start, stop, step):
	cur = int(start*10000)
	while cur < int(stop*10000):
		yield int(cur + step)/10000.
		cur = int(cur + step*10000)

def frange(start, stop, step):
	return [x for x in myRange(start, stop, step)]

def getAll(list):
	if len(list) == 1:
		varName, valList = list[0]
		return [[(varName, val)] for val in valList]

	varName, valList = list[0]
	rest = list[1:]

	result = []
	for subList in getAll(rest):
		for value in valList:
			result.append([(varName, value)]+subList)

	return result

def getAll_fast(perms):
	perm_list = perms.items()
	for perm in itertools.product(*[p[1] for p in perm_list]):
		yield dict((perm_list[i][0], perm[i]) for i in range(len(perm_list)))

def modifiedEnvironments():
	dictParams = {x: getattr(params, x) for x in dir(params) if not x.startswith("_")}

	for envVals in getAll_fast(dictParams):
		curEnv = os.environ.copy()
		for key, value in envVals.iteritems():
			curEnv[key] = str(value)
		curEnv["EVALUATION"] = "YES"
		curEnv["LD_LIBRARY_PATH"] = "/local/opencv/opencv-2.4.10/build/lib"
		yield curEnv

def assignedEnvironments(servers):
	i=0
	for env in modifiedEnvironments():
		yield (servers[i%len(servers)], env)
		i += 1

def handleServerMode(secret):
	s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
	s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
	s.bind(("", 8556))
	s.listen(5)

	while True:
		conn, addr = s.accept()
		if not secret == conn.recv(len(secret)):
			conn.close()

		length = int(conn.recv(8))
		env = conn.recv(length)
		while not len(env) == length:
			env += conn.recv(1)

		env = eval(env)

		print "Accepted incoming connection, evaluating"
		try:
			handleEnv(env)
		except KeyboardInterrupt:
			conn.send("NCK")
			raise KeyboardInterrupt
		except Exception:
			conn.send("NCK")
		else:
			conn.send("ACK")
		finally:
			conn.close()

def handleClientMode(secret, server, env):
	s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
	s.connect((server, 8556))

	s.send(secret)
	s.send(("000000000"+str(len(str(env))))[-8:])
	s.send(str(env))

	if not s.recv(4) == "ACK":
		print "Server "+server+ "malfunctioned"

	s.close()


def handleEnv(env, repeated=False):

	#execute in subshells, save output
	outputs = []
	processes = []
	for fn in os.listdir(PATH):
		if fn.endswith("unprocessed.yml"):
			f = tempfile.SpooledTemporaryFile(max_size=2048, mode="w+")
			outputs.append(f)
			processes.append(subprocess.Popen([HOME+"/BA/nativeCode/Debug/GoBoardReaderNative_evaluating", PATH+fn], env=env, stdout=f, stderr=None))

	#wait for all subshsells
	for p in processes:
		p.wait()

	totalFiles = 0
	totalCorrectPiecesPercent = 0
	totalAvailable = 0
	totalMatched = 0
	totalBogus = 0
	data = None
	parameters = {}
	try:
		for f in outputs:
			f.seek(0)
			f.readline()

			d = f.read()
			data = yaml.load(d)

			if not data:
				continue

			totalCorrectPiecesPercent += data["matched"]/data["available"] if not data["available"] == 0 else 0
			totalAvailable += data["available"]
			totalMatched += data["matched"]
			totalBogus += data["wrong"]
			totalFiles += 1

			if len(parameters) == 0:
				for key in data:
					if key.startswith("COLORS"):
						parameters[key] = data[key]

	except yaml.YAMLError:
		if repeated == True:
			print "Unrecoverable error"

		return handleEnv(env, True)
	finally:
		for f in outputs:
			f.close()

	if totalFiles == 0:
		print "No output"
		return

	newData = dict(parameters)

	newData["totalCorrectPiecesPercent"] = totalCorrectPiecesPercent/totalFiles
	newData["totalFiles"] = totalFiles
	newData["totalAvailable"] = totalAvailable
	newData["totalMatched"] = totalMatched
	newData["currentTime"] = str(datetime.datetime.now())
	newData["totalBogus"] = totalBogus
	newData["quality"] = totalMatched/totalBogus if not totalBogus == 0 else 999

	lock = lockfile.FileLock("zusammenfassung_"+HOSTNAME+".yml")
	lock.acquire(timeout=30)  #wenn kein lock, dann karpott

	f = open("zusammenfassung_"+HOSTNAME+".yml", "a")
	f.write("---\n")
	f.write(yaml.dump(newData, default_flow_style=False))
	f.flush()
	f.close()

	lock.release()

params.COLORS_SPECKLESIZE =  range(0, 5, 1) #2   +- 2
params.COLORS_RECTSIZE    =  range(5, 36, 2) #20  +-15
params.COLORS_BLACKTHRESH =  range(40, 80, 4) #80  +-40
params.COLORS_WHITETHRESH =  range(40, 80, 4) #220 +-40


totalParams = 1
for x in dir(params):
	if not x.startswith("_"):
		totalParams *= len(getattr(params, x))


if __name__ == "__main__":
	secret = """saimuu6ohxooRiob6ieyoocheiwiehootu6uic7nohVoh3Shie1reithu2aic8z
				u1iJo0paik3Apu5Phadei7iewae0waeZohpaaixau7Bee5nah4ahmugaol3phoo6ong6ooyae5engo7ie
				pahl4ul9juJae1cah8aih5ohC6aehiThae4yeingaemifee2gae3siehchieW5ii2pahgh3faetai9Ahm
				2vahn5shoobuv8biephoo6Sheef6selib4souk2ohghi4oohae4sah5bah8eigee2leeXaicoh4"""
	try:

		if len(sys.argv) > 1 and sys.argv[1] == "SERVE":
				handleServerMode(secret)

		else:

			servers = sys.argv[1:]

			print "Evaluating "+str(totalParams)+" sets of parameters on "+str(len(servers)+" machines"
			print "Expected calculation time: "+str(totalParams*3/len(servers)/60)+" minutes"

			joblib.Parallel(n_jobs=len(servers), verbose=100, pre_dispatch="2*n_jobs")(joblib.delayed(handleClientMode)(secret, server, env) for server, env in assignedEnvironments(servers))

	except KeyboardInterrupt:
		print "Shutting down"
