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
from thread import start_new_thread

DEVNULL = open(os.devnull, 'wb')
HOME = os.path.expanduser("~")
PATH = "../../test/files/train/unprocessed/"
HOSTNAME = socket.gethostname()

class Storage():
	pass

failedEnvs = []
assignedEnvs = {}

def finishedNotifiee():
	s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
	s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
	s.bind(("localhost", 8557))
	s.listen(10)

	global assignedEnvs
	while True:
		conn, addr = s.accept()
		server = conn.recv(1024)

		assignedEnvs[server] -= 1


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

def modifiedEnvironments(params):
	dictParams = {x: getattr(params, x) for x in dir(params) if not x.startswith("_")}

	for envVals in getAll_fast(dictParams):
		curEnv = os.environ.copy()
		for key, value in envVals.iteritems():
			curEnv[key] = str(value)
		curEnv["EVALUATION"] = "YES"
		curEnv["LD_LIBRARY_PATH"] = "/local/opencv/opencv-2.4.10/build/lib"
		yield curEnv

def assignedEnvironments(servers, params):
	i=0
	for env in modifiedEnvironments(params):

		while True:
			server = servers[i%len(servers)]
			if assignedEnvs[server] <= 8:
				break
			i += 1

		assignedEnvs[server] += 1
		yield (server, env)
		i += 1

def handleServerMode(secret, prefix):
	s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
	s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
	s.bind(("", 8556))
	s.listen(10)

	processed = 0
	while True:
		conn, addr = s.accept()
		if not secret == conn.recv(len(secret)):
			conn.close()

		length = int(conn.recv(8))
		env = conn.recv(length)
		while not len(env) == length:
			env += conn.recv(1)

		env = eval(env)

		print "\rAccepted incoming connection #"+str(processed)+", evaluating",
		sys.stdout.flush()
		processed += 1

		try:
			handleEnv(env, prefix)
		except KeyboardInterrupt:
			conn.send("NCK")
			raise KeyboardInterrupt
		except Exception as e:
			print "Encountered an exception, sending nack"
			print e
			conn.send("NCK")
		else:
			conn.send("ACK")
		finally:
			conn.close()
	print ""

def handleClientMode(secret, server, env):
	try:
		s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
		s.connect((server, 8556))

		s.send(secret)
		s.send(("000000000"+str(len(str(env))))[-8:])
		s.send(str(env))

		if not s.recv(4) == "ACK":
			failedEnvs.append(env)
			print "Server "+server+ " malfunctioned"

	except Exception as e:
		failedEnvs.append(env)
		print "Error in connection to server "+server
		print str(e)
	finally:
		s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
		s.connect(("localhost", 8557))
		s.send(server)
		s.close()


def handleEnv(env, prefix, repeated=False):
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

	#readout data
	data = None
	totalFiles = 0
	parameters = {}
	values = {}
	try:
		for f in outputs:
			f.seek(0)
			f.readline()

			d = f.read()
			data = yaml.load(d)

			if not data:
				continue

			#sum up all values
			for key in data:
				if key in ["filename", "runTime", "usedParams"] or key[0].isupper():
					continue
				if key not in values:
					values[key] = 0
				values[key] += data[key]

			totalFiles += 1

			#save the parameters once
			if len(parameters) == 0:
				for key in data:
					if key.startswith(prefix):
						parameters[key] = data[key]

	except yaml.YAMLError:
		if repeated == True:
			print "Unrecoverable error"

		return handleEnv(env, prefix, True)
	finally:
		for f in outputs:
			f.close()


	if totalFiles == 0:
		print "No output"
		return


	#build averages over all files
	for key in values:
		if key.startswith("avg_"):
			values[key] /= totalFiles

	values["totalFiles"] = totalFiles
	values["currentTime"] = str(datetime.datetime.now())
	values.update(parameters)


	lock = lockfile.FileLock("zusammenfassung_"+HOSTNAME+".yml")
	lock.acquire(timeout=30)  #wenn kein lock, dann karpott

	f = open("zusammenfassung_"+HOSTNAME+".yml", "a")
	f.write("---\n")
	f.write(yaml.dump(values, default_flow_style=False))
	f.flush()
	f.close()

	lock.release()

def main(secret, params):
	totalParams = 1
	for x in dir(params):
		if not x.startswith("_"):
			totalParams *= len(getattr(params, x))

	try:

		if len(sys.argv) > 2 and sys.argv[1] == "SERVE":
				handleServerMode(secret, sys.argv[2])

		else:
			servers = sys.argv[1:]
			for server in servers:
				assignedEnvs[server] = 0

			print "Evaluating "+str(totalParams)+" sets of parameters on "+str(len(servers))+" machines"
			print "Expected calculation time: "+str(totalParams*4.58/len(servers)/60)+" minutes"

			#can't see another way to notify this thread of finished other threads :(
			start_new_thread(finishedNotifiee,())

			joblib.Parallel(n_jobs=len(servers), verbose=1, pre_dispatch="2*n_jobs")(joblib.delayed(handleClientMode)(secret, server, env) for server, env in assignedEnvironments(servers, params))

			f.open("failedEnvs", "w")
			for env in failedEnvs:
				f.write(str(env))
				f.write("==================")
			f.close()

	except KeyboardInterrupt:
		print "Shutting down"
