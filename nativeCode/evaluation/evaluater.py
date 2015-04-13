#!/usr/bin/python2

import datetime
import os
import itertools
import shutil
import subprocess
import tempfile
import yaml

DEVNULL = open(os.devnull, 'wb')
HOME = os.path.expanduser("~")
PATH = "../../test/files/03b0c66/unprocessed/"

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
	totalAvailableIntersects = 0
	totalMatched = 0
	totalBogus = 0
	data = None
	try:
		for f in outputs:
			f.seek(0)
			f.readline()

			data = yaml.load(f.read())

			if not data:
				continue

			totalCorrect += data["intersectionCorrectness"]
			totalAvailableIntersects += data["availableIntersects"]
			totalMatched += data["matched"]
			totalBogus += data["bogus"]
			totalFiles += 1

	except yaml.YAMLError:
		if repeated == True:
			print "Unrecoverable error"

		handleEnv(env, True)
	finally:
		for f in outputs:
			f.close()

	if totalFiles == 0:
		return

	newData = {}
	for key in data:
		if key[0].isupper():
			newData[key] = data[key]

	newData["totalCorrect"] = totalCorrect/totalFiles
	newData["totalFiles"] = totalFiles
	newData["totalAvailableIntersects"] = totalAvailableIntersects
	newData["totalMatched"] = totalMatched
	newData["currentParam"] = currentParam-1
	newData["currentTime"] = str(datetime.datetime.now())


	f = open("zusammenfassung.yml", "a")
	f.write("---\n")
	f.write(yaml.dump(newData, default_flow_style=False))
	f.close()

params.LINES_HOUGH_GAUSSKERNEL   =  range(3, 7, 2)
params.LINES_HOUGH_GAUSSSIGMA    =  range(2, 4, 1)
#params.LINES_HOUGH_CANNYTHRESH1  =  range(30, 70, 5)
#params.LINES_HOUGH_CANNYTHRESH2  =  range(180, 220, 5)
#params.LINES_HOUGH_CANNYAPERTURE =  range(2, 5, 1)

params.LINES_HOUGH_ANGLERES       = [180, 270, 360]
params.LINES_HOUGH_HOUGHTHRESH    = range(25, 65, 5)
params.LINES_HOUGH_HOUGHMINLENGTH = range(50, 90, 5)
params.LINES_HOUGH_HOUGHMAXGAP    = range(5, 15, 3)

totalParams = 1
for x in dir(params):
	if not x.startswith("_"):
		totalParams *= len(getattr(params, x))

currentParam = 0
for environment in modifiedEnvironments():
	print str(currentParam)+"/"+str(totalParams)
	currentParam += 1

	handleEnv(environment)
