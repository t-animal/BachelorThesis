#!/usr/bin/python2

from evaluaterUtils import *

params = Storage()

params.LINES_HOUGH_GAUSSKERNEL   =  range(3, 7, 2)
params.LINES_HOUGH_GAUSSSIGMA    =  range(2, 4, 1)
#params.LINES_HOUGH_CANNYTHRESH1  =  range(30, 70, 5)
#params.LINES_HOUGH_CANNYTHRESH2  =  range(180, 220, 5)
#params.LINES_HOUGH_CANNYAPERTURE =  range(2, 5, 1)

params.LINES_HOUGH_ANGLERES       = [180, 270, 360]
params.LINES_HOUGH_HOUGHTHRESH    = range(25, 65, 5)
params.LINES_HOUGH_HOUGHMINLENGTH = range(50, 90, 5)
params.LINES_HOUGH_HOUGHMAXGAP    = range(5, 15, 3)

secret = """saimuu6ohxooRiob6ieyoocheiwiehootu6uic7nohVoh3Shie1reithu2aic8z
			u1iJo0paik3Apu5Phadei7iewae0waeZohpaaixau7Bee5nah4ahmugaol3phoo6ong6ooyae5engo7ie
			pahl4ul9juJae1cah8aih5ohC6aehiThae4yeingaemifee2gae3siehchieW5ii2pahgh3faetai9Ahm
			2vahn5shoobuv8biephoo6Sheef6selib4souk2ohghi4oohae4sah5bah8eigee2leeXaicoh4"""

main(secret, params)
