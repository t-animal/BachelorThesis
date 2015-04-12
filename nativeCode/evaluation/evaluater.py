#!/usr/bin/python2

import datetime
import os
import itertools
import shutil
import subprocess
import yaml

DEVNULL = open(os.devnull, 'wb')

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
		yield curEnv

params.LINES_HOUGH_GAUSSKERNEL   =  range(3, 7, 2)
params.LINES_HOUGH_GAUSSSIGMA    =  range(2, 7, 1)
params.LINES_HOUGH_CANNYTHRESH1  =  range(30, 70, 5)
params.LINES_HOUGH_CANNYTHRESH2  =  range(180, 220, 5)
#params.LINES_HOUGH_CANNYAPERTURE =  range(2, 5, 1)

params.LINES_HOUGH_ANGLERES       = [90, 180, 270]
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

	subprocess.Popen("../../test/test.sh ../../test/files/03b0c66/unprocessed/*d.yml", shell=True, env=environment, stdout=DEVNULL, stderr=DEVNULL).wait()

	totalFiles = 0
	totalCorrect = 0
	totalAvailableIntersects = 0
	totalMatched = 0
	data = None
	for fn in os.listdir('.'):
		if os.path.isfile(fn) and fn.startswith("run_"):
			f = open(fn, 'r')
			f.readline()
			data = yaml.load(f.read())
			f.close()

			totalCorrect += data["intersectionCorrectness"]
			totalAvailableIntersects += data["availableIntersects"]
			totalMatched += data["matched"]
			totalFiles += 1

			shutil.move(fn, "old/"+fn)

	if totalFiles == 0:
		continue

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
