#!/usr/bin/python2

from evaluaterUtils import *

params = Storage()

params.COLORS_SPECKLESIZE =  range(0, 7, 1) #2   +- 2
params.COLORS_RECTSIZE    =  range(1, 41, 2) #20  +-20
params.COLORS_BLACKTHRESH =  range(40, 80, 4) #80  +-40
params.COLORS_WHITETHRESH =  range(180, 261, 4) #220 +-40

secret = """saimuu6ohxooRiob6ieyoocheiwiehootu6uic7nohVoh3Shie1reithu2aic8z
			u1iJo0paik3Apu5Phadei7iewae0waeZohpaaixau7Bee5nah4ahmugaol3phoo6ong6ooyae5engo7ie
			pahl4ul9juJae1cah8aih5ohC6aehiThae4yeingaemifee2gae3siehchieW5ii2pahgh3faetai9Ahm
			2vahn5shoobuv8biephoo6Sheef6selib4souk2ohghi4oohae4sah5bah8eigee2leeXaicoh4"""

main(secret, params)
