/*
  ArduHA - ArduixPL - xPL library for Arduino(tm)
  Copyright (c) 2012/2014 Mathieu GRENET.  All right reserved.

  This file is part of ArduHA / ArduixPL.

    ArduixPL is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    ArduixPL is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with ArduixPL.  If not, see <http://www.gnu.org/licenses/>.

	  Modified 2014-3-14 by Mathieu GRENET 
	  mailto:mathieu@mgth.fr
	  http://www.mgth.fr
*/
/*****************************************************************************
GLOBALS OPTIONS
*****************************************************************************/
// Used by ha_eeprom.h short this pin to ground at startup to avoid config loading
#define HA_RESET_PIN 5


#define HA_FLOAT

/*****************************************************************************
DEBUG OPTIONS
*****************************************************************************/
// To print debug info
#define DEBUG 3
// to print memory related debug info
#define DEBUG_MEM

// to print debug info on LCD
//(you should include Wire.h & LiquidCrystal_I2C.h)
//#define DEBUG_LCD

//uncomment to reduce main loop speed, mainly useful to know whitch module hang
//#define SLOWDEBUG 200


