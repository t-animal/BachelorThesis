#!/usr/bin/python2

from evaluaterUtils import *

params = Storage()

## both:
#params.PIECES_GAUSS_V			=  range(5, 10, 2)		#7 -+ 2
#params.PIECES_GAUSS_S			=  range(23, 28, 2)		#25 -+ 2

#params.PIECES_THRESH_V			=  range(50, 91, 3)		#70 +- 20
#params.PIECES_THRESH_S			= frange(0.02, 0.33, 0.2)		#0.17 +- 15
#params.PIECES_THRESH_H			=  range(150, 250, 3)		#70 -+ 20

#params.PIECES_SPECKLES			=  range(4, 9, 1)		#6 +- 2

## contour
# params.PIECES_MINDIAMETER		=  range(2, 21, 2)		#9 +- 10
# params.PIECES_MAXDIAMETER		=  range(22, 43, 2)		#32L +- 10

# params.PIECES_MAXRATIO			= frange(1.0, 2.1, 0.2) 		#1.5 +-5
# params.PIECES_MINRATIO			= frange(0.1, 1.1, 0.2) 		#0.5 +-5

# params.PIECES_SPLITDIFFERENCE	=  range(1, 11, 1)		#5L +- 5

## hough
# params.PIECES_MINDIST_DARK	 	= range(22, 43, 3) #32 +- 10
# params.PIECES_MINRAD_DARK	 	= range(14, 35, 3) #24 +- 10
# params.PIECES_MAXRAD_DARK	 	= range(35, 56, 3) #45 +- 10

params.PIECES_MINDIST_LIGHT	 	= range(26, 47, 3) #36 +- 5
params.PIECES_MINRAD_LIGHT	 	= range(6, 27, 3) #16 +- 5
params.PIECES_MAXRAD_LIGHT	 	= range(35, 55, 3) #45 +- 10

params.PIECES_ACCUTHRESH_LIGHT	= [999]#range(40, 61, 3) #50 +- 10
params.PIECES_ACCUTHRESH_DARK	= range(40, 61, 3) #50 +- 10


secret = """saimuu6ohxooRiob6ieyoocheiwiehootu6uic7nohVoh3Shie1reithu2aic8z
			u1iJo0paik3Apu5Phadei7iewae0waeZohpaaixau7Bee5nah4ahmugaol3phoo6ong6ooyae5engo7ie
			pahl4ul9juJae1cah8aih5ohC6aehiThae4yeingaemifee2gae3siehchieW5ii2pahgh3faetai9Ahm
			2vahn5shoobuv8biephoo6Sheef6selib4souk2ohghi4oohae4sah5bah8eigee2leeXaicoh4"""

main(secret, params)
