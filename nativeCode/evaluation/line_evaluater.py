#!/usr/bin/python2

from evaluaterUtils import *

params = Storage()

# params.LINES_HOUGH_GAUSSKERNEL   =  range(1, 8, 2)      #3 +- 4
# #params.LINES_HOUGH_GAUSSSIGMA    =  range(1, 5, 2) 		#2. +- 1
# params.LINES_HOUGH_CANNYTHRESH1  =  range(10, 96, 15)	#50 +- 45
# params.LINES_HOUGH_CANNYTHRESH2  =  range(160, 246, 15)	#200 +- 45
# #params.LINES_HOUGH_CANNYAPERTURE =  range(2, 6, 1)		#3 +- 2

# params.LINES_HOUGH_ANGLERES       = [180, 360]			#180 + 180
# params.LINES_HOUGH_HOUGHTHRESH    = range(15, 61, 7)	#40 +- 25
# params.LINES_HOUGH_HOUGHMINLENGTH = range(50, 91, 7)	#70 +- 20
# params.LINES_HOUGH_HOUGHMAXGAP    = range(1, 21, 4)		#10 +- 10

# params.LINES_LSD_GAUSSKERNEL	=	range(3, 12, 4)      #7 +- 4
# params.LINES_LSD_GAUSSSIGMA		=	range(3, 8, 2)       #5 +- 2
# params.LINES_LSD_SCALE			=  frange(0.1, 1.0, 0.1) #0.5 +- 0.5
# params.LINES_LSD_SIGMA			=  frange(0.85, 1.65, 0.1) #1.25 +-0.3
# params.LINES_LSD_ANGLETHRESH	=	range(1, 31, 3)      #10.0 +- 20
# params.LINES_LSD_DENSITYTHRESH	=  frange(0.1, 1.0, 0.1) # 0.5 +- 0.5


params.INTERSECT_FAST_GAUSSKERNEL 	= range(1, 6, 2) # 3 +- 2
params.INTERSECT_FAST_GAUSSSIGMA 	= range(1, 5, 1) # 2 +- 2
params.INTERSECT_FAST_THRESHOLD 	= range(1, 41, 2) # 20 +- 20
params.INTERSECT_FAST_NONMAXSUPP 	= [0, 1] # boolean
params.INTERSECT_FAST_TYPE 			= [0, 1, 2] # enum

secret = """saimuu6ohxooRiob6ieyoocheiwiehootu6uic7nohVoh3Shie1reithu2aic8z
			u1iJo0paik3Apu5Phadei7iewae0waeZohpaaixau7Bee5nah4ahmugaol3phoo6ong6ooyae5engo7ie
			pahl4ul9juJae1cah8aih5ohC6aehiThae4yeingaemifee2gae3siehchieW5ii2pahgh3faetai9Ahm
			2vahn5shoobuv8biephoo6Sheef6selib4souk2ohghi4oohae4sah5bah8eigee2leeXaicoh4"""

main(secret, params)
