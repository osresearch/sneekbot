/** \file
 * LX16 debug/external interface.
 */

#include "ServoLX.h"
#include <math.h>

ServoLX servos(7,8);

#define NUM_SERVOS 8

uint16_t poses[][NUM_SERVOS] = {
 { 500,500,500,500,500,500,500,500, }, // center all servos
 { 566,310,418,499,548,552,423,522, }, // straight up
 { 613,336,414,499,593,551,423,524, }, // shift right
 { 635,319,331,499,505,552,423,523, }, // lean right
 { 636,318,330,337,505,551,422,700, }, // lift left foot
 { 636,351,423,336,620,369,582,703, }, // step left
 { 635,267,419,423,630,369,497,703, }, // weight left foot
 { 630,266,383,483,576,499,461,533, },  // lean left

 // 
 { 1078,267,368,491,519,516,491,486, }, // lean left
};


void pose(int ms, const uint16_t * const pose_cmd)
{
	for(unsigned i = 0 ; i < NUM_SERVOS ; i++)
	{
		uint16_t cmd = pose_cmd[i];
		Serial.print(" ");
		Serial.print(cmd);
		servos.move_raw(
			i + 1,
			cmd,
			ms,
			1
		);
	}

	servos.start();
}


void setup()
{
	servos.begin();
	Serial.begin(115200);

	servos.disable();
}


void report_position()
{
	Serial.print("p={");

	for(int i = 1 ; i <= 8 ; i++)
	{
		int pos = servos.position_raw(i);
		Serial.print(pos);
		Serial.print(",");
	}

	Serial.println("}");
}

uint32_t last_position;

void loop()
{
	// try to see if anyone is out there
	if(!Serial.available())
	{
		if (millis() - last_position < 500)
			return;
		last_position = millis();
		report_position();
		return;
	}

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

	if (cmd == 'e')
	{
		Serial.print("e=");
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
		servos.enable(id);
		return;
	}

	if (cmd == 'p')
	{
		report_position();
		return;
	}

	if (cmd == 'h')
	{
		Serial.print("hold:");
		for(uint8_t i = 1 ; i <= 8 ; i++)
		{
			float pos = servos.position(i);
			Serial.print(" ");

			if (pos == (int) SERVOLX_TIMEOUT)
			{
				Serial.print("!");
			} else {
				Serial.print(pos, 2);
				servos.move(i, pos);
			}
		}

		Serial.println();
	}

	if (cmd == ' ')
	{
		Serial.println("STOP");
		servos.stop();
		servos.disable();
		return;
	}

	if ('0' <= cmd && cmd <= '9')
	{

		servos.enable();

		Serial.print("pose ");
		Serial.print(cmd - '0');
		pose(1000, poses[cmd - '0']);
		Serial.println();

		servos.start();
		return;
	}



	Serial.println("?");
}
