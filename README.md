Arduino Mediotext LED display driver
====================================

Building
--------

	$ wget https://github.com/dnet/arscons/raw/master/SConstruct
	$ scons build

Uploading
---------

### Default config ###

	$ scons upload

### Example for ATmega168-based boards ###

	$ ARDUINO_BOARD=atmega168 scons upload

Dependencies for arscons build process
--------------------------------------

 - Arduino 1.0+ (if you have an older version, rename `*.{ino -> pde}`)
 - Scons (Debian/Ubuntu package: `scons`)
