#!/usr/bin/python2

import commands
import datetime
import os
import itertools
import shutil
import socket
import subprocess
import tempfile
import yaml

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

def handleEnv(env, repeated=False):

	#execute in subshells, save output
	outputs = []
	processes = []
	for fn in os.listdir(PATH):
		if fn.endswith("unprocessed.yml"):
			f = tempfile.SpooledTemporaryFile(max_size=1024*1024, mode="w+")
			outputs.append(f)
			processes.append(subprocess.Popen(HOME+"/BA/nativeCode/Debug/GoBoardReaderNative_evaluating "+PATH+fn, shell=True, env=environment, stdout=f, stderr=DEVNULL))

	#wait for all subshsells
	for p in processes:
		p.wait()

	totalFiles = 0
	totalCorrect = 0
	totalAvailable = 0
	totalMatched = 0
	totalBogus = 0
	data = None
	try:
		for f in outputs:
			f.seek(0)
			f.readline()

			d = f.read()
			data = yaml.load(d)

			if not data:
				continue

			totalCorrect += data["correctPiecesPercent"]
			totalAvailable += data["availablePieces"]
			totalMatched += data["matched"]
			totalBogus += data["bogus"]
			totalFiles += 1

	except yaml.YAMLError:
		if repeated == True:
			print "Unrecoverable error"

		return handleEnv(env, True)
	finally:
		for f in outputs:
			f.close()

	if totalFiles == 0 or data == None:
		print "No output"
		return

	newData = {}
	for key in data:
		if key.startswith("PIECES"):
			newData[key] = data[key]

	newData["totalCorrect"] = totalCorrect/totalFiles
	newData["totalFiles"] = totalFiles
	newData["totalAvailable"] = totalAvailable
	newData["totalMatched"] = totalMatched
	newData["currentParam"] = currentParam-1
	newData["currentTime"] = str(datetime.datetime.now())
	newData["totalBogus"] = totalBogus
	newData["quality"] = totalMatched/totalBogus


	f = open("zusammenfassung_"+HOSTNAME+".yml", "a")
	f.write("---\n")
	f.write(yaml.dump(newData, default_flow_style=False))
	f.flush()
	f.close()

params.PIECES_GAUSS_V       =  range(7, 18, 2)   #13 +-5
#params.PIECES_GAUSS_S       =  range(19, 30, 2)  #25 +-5

params.PIECES_THRESH_V      =  range(50, 91, 5)  #70 +-20
#params.PIECES_THRESH_S      = frange(0.02, 0.33, 3) #0.17 +-15
#params.PIECES_THRESH_H      =  range(50, 91, 5)  #70 +-20

#params.PIECES_SPECKLES      =  range(3, 8, 1)    # 5 +-2
#params.PIECES_SPECKLES      = [3] #faui00a
#params.PIECES_SPECKLES      = [4] #faui00b
#params.PIECES_SPECKLES      = [5] #faui00c
#params.PIECES_SPECKLES      = [6] #faui00d
params.PIECES_SPECKLES      = [7] #faui00e

params.PIECES_MINDIST_DARK  =  range(10, 21, 2)  #15 +- 5
params.PIECES_MINRAD_DARK   =  range(15, 26, 2)  #20 +- 5
params.PIECES_MAXRAD_DARK   =  range( 6, 17, 2)  #11 +- 5

#params.PIECES_MINDIST_LIGHT =  range(10, 21, 2)  #13 +- 5
#params.PIECES_MINRAD_LIGHT  =  range(25, 36, 2)  #30 +- 5
#params.PIECES_MAXRAD_LIGHT  =  range( 6, 17, 2)  #11 +- 5


totalParams = 1
for x in dir(params):
	if not x.startswith("_"):
		totalParams *= len(getattr(params, x))

currentParam = 0
for environment in modifiedEnvironments():
	print str(currentParam)+"/"+str(totalParams)
	currentParam += 1

	handleEnv(environment)
