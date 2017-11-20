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
	if(!Serial.available())
		return;

	const int cmd = Serial.read();
	if (cmd == 'i')
	{
		Serial.print("i=");
		int id = servos.getid();
		Serial.println(id);
		return;
	}

	if (cmd == 'I')
	{
		Serial.print("I=");
		while(!Serial.available())
			;
		int id = Serial.read();
		if ('0' <= id && id <= '9')
			id -= '0' + 0x0;
		else
		if ('a' <= id && id <= 'f')
			id -= 'a' + 0xA;
		else
		{
			Serial.println("?");
			return;
		}

		Serial.println(id, HEX);
		servos.setid(id);
		return;
	}

	if (cmd == 'p')
	{
		Serial.print("p=");

		for(int i = 1 ; i <= 8 ; i++)
		{
			int pos = servos.position(i);
			Serial.print(" ");
			Serial.print(pos);
		}

		Serial.println();
		return;
	}

	Serial.println("?");
}
