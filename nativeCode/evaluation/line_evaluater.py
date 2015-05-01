#!/usr/bin/python2

from evaluaterUtils import *

params = Storage()

params.LINES_HOUGH_GAUSSKERNEL   =  range(1, 8, 2)      #3 +- 4
#params.LINES_HOUGH_GAUSSSIGMA    =  range(1, 5, 2) 		#2. +- 1
params.LINES_HOUGH_CANNYTHRESH1  =  range(10, 96, 15)	#50 +- 45
params.LINES_HOUGH_CANNYTHRESH2  =  range(160, 246, 15)	#200 +- 45
#params.LINES_HOUGH_CANNYAPERTURE =  range(2, 6, 1)		#3 +- 2

params.LINES_HOUGH_ANGLERES       = [180, 360]			#180 + 180
params.LINES_HOUGH_HOUGHTHRESH    = range(15, 61, 7)	#40 +- 25
params.LINES_HOUGH_HOUGHMINLENGTH = range(50, 91, 7)	#70 +- 20
params.LINES_HOUGH_HOUGHMAXGAP    = range(1, 21, 4)		#10 +- 10

secret = """saimuu6ohxooRiob6ieyoocheiwiehootu6uic7nohVoh3Shie1reithu2aic8z
			u1iJo0paik3Apu5Phadei7iewae0waeZohpaaixau7Bee5nah4ahmugaol3phoo6ong6ooyae5engo7ie
			pahl4ul9juJae1cah8aih5ohC6aehiThae4yeingaemifee2gae3siehchieW5ii2pahgh3faetai9Ahm
			2vahn5shoobuv8biephoo6Sheef6selib4souk2ohghi4oohae4sah5bah8eigee2leeXaicoh4"""

main(secret, params)
