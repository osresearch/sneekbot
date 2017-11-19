/** \file
 * LX16 debug/external interface.
 */

#include "ServoLX.h"

ServoLX servos(7,8);

void setup()
{
	servos.begin();
	Serial.begin(115200);

	servos.disable();
}

void loop()
{
	// try to see if anyone is out there
	while(Serial.read() == -1)
		;
	Serial.println("probe id:");
	int id = servos.getid();
	Serial.println(id);
	if (id < 0)
		return;

	int pos = servos.position(id);
	Serial.println(pos);
	return;

	// read all of the positions and write to the serial port

	Serial.print("position: ");

	for(int i = 0 ; i < 8 ; i++)
	{
		int pos = servos.position(i);
		Serial.print(" ");
		Serial.print(pos);
	}

	Serial.println();
}
