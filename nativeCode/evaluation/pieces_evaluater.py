#!/usr/bin/python2

from evaluaterUtils import *

params = Storage()

#params.PIECES_GAUSS_V       =  range(7, 18, 2)   #13 +-5
#params.PIECES_GAUSS_V       =  [7, 9]   #faui00a
#params.PIECES_GAUSS_V       =  [11, 13] #faui00b
params.PIECES_GAUSS_V       =  [15, 17] #faui00c
params.PIECES_GAUSS_S       =  range(19, 30, 2)  #25 +-5

params.PIECES_THRESH_V      =  range(50, 91, 5)  #70 +-20
params.PIECES_THRESH_S      = frange(0.02, 0.33, 3) #0.17 +-15
params.PIECES_THRESH_H      =  range(50, 91, 5)  #70 +-20

params.PIECES_SPECKLES      =  range(4, 7, 1)    # 5 +-1

params.PIECES_MINDIAMETER   =  range(0, 21, 3)   #10 +- 10
params.PIECES_MINDIAMETER   =  range(20, 41, 3)   #30 +- 10

#params.PIECES_MINDIST_DARK  =  range(10, 21, 2)  #15 +- 5
#params.PIECES_MINRAD_DARK   =  range(15, 26, 2)  #20 +- 5
#params.PIECES_MAXRAD_DARK   =  range( 6, 17, 2)  #11 +- 5

#params.PIECES_MINDIST_LIGHT =  range(10, 21, 2)  #13 +- 5
#params.PIECES_MINRAD_LIGHT  =  range(25, 36, 2)  #30 +- 5
#params.PIECES_MAXRAD_LIGHT  =  range( 6, 17, 2)  #11 +- 5

secret = """saimuu6ohxooRiob6ieyoocheiwiehootu6uic7nohVoh3Shie1reithu2aic8z
			u1iJo0paik3Apu5Phadei7iewae0waeZohpaaixau7Bee5nah4ahmugaol3phoo6ong6ooyae5engo7ie
			pahl4ul9juJae1cah8aih5ohC6aehiThae4yeingaemifee2gae3siehchieW5ii2pahgh3faetai9Ahm
			2vahn5shoobuv8biephoo6Sheef6selib4souk2ohghi4oohae4sah5bah8eigee2leeXaicoh4"""

main(secret, params)
