Knopfor
by Armin Köfler

A quick and dirty hack that allows ESP's control over a self-made circuit.

The circuit that consists of a button (NC and NO pin) which gives singls to an ESP8266 which are translated into HTTP GET commands.

The NO pin initially powers up the ESP by activating a relay which is then held by the GPIO0 pin of the ESP8266. While active the HTTP command sending is done.
