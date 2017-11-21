/** \file
 * LX16 debug/external interface.
 */

#include "ServoLX.h"
#include <math.h>

ServoLX servos(7,8);

void setup()
{
	servos.begin();
	Serial.begin(115200);

	servos.disable();
}


void report_position()
{
	Serial.print("p");

	for(int i = 1 ; i <= 8 ; i++)
	{
		float pos = servos.position(i);
		Serial.print(" ");
		Serial.print(pos, 2);
	}

	Serial.println();
}

uint32_t last_position;

/*
 * Undulating motion.
 * Wavelength for the wave and phase shift for each joint
 */
void wave()
{
	float t = 0;
	float mag = 60;
	float phase_step = 30 * M_PI / 180; // rad/joint
	float rate = 90 * M_PI / 180; // rad/sec
	float last_step = millis();

	while(1)
	{
		if (Serial.available())
		{
			Serial.read();
			servos.stop();
			return;
		}

		Serial.print("c");
		for(uint8_t i = 1 ; i <= 8 ; i+=1)
		{
			float angle = mag * sin(t + i * phase_step);
			if(0 && (i/2) % 2 == 0)
				angle = -angle;
			servos.move(i, angle, 0, 1);
			Serial.print(" ");
			Serial.print(angle, 2);
		}
		Serial.println();

		uint32_t now = millis();
		float dt = (now - last_step) / 1000.0;
		last_step = now;

		t += rate * dt;
		if (t > 2 * M_PI)
			t = 0;

		servos.start();

		report_position();

		//delay(dt);
	}
}


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

	if (cmd == '0')
	{
		Serial.println("STRAIGHT");
		for(uint8_t i = 1 ; i <= 8 ; i++)
			servos.move(i, 0, 2000, true);
		servos.enable();
		servos.start();
		return;
	}

	if (cmd == '9')
	{
		Serial.println("CURL");
		for(uint8_t i = 1 ; i <= 8 ; i++)
			servos.move(i, -90, 5000, true);
		servos.enable();
		servos.start();
		return;
	}

	if (cmd == '8')
	{
		Serial.println("CURL2");
		for(uint8_t i = 1 ; i <= 8 ; i++)
			servos.move(i, i % 2 ? 90 : -90, 5000, true);
		servos.enable();
		servos.start();
		return;
	}
		
	if (cmd == '7')
	{
		Serial.println("WRAP");
		for(uint8_t i = 1 ; i <= 8 ; i++)
			servos.move(i, (i % 3 == 2) ? 90 : -90, 5000, true);
		servos.enable();
		servos.start();
		return;
	}

	if (cmd == '6')
	{
		Serial.println("WRAP2");
		for(uint8_t i = 1 ; i <= 8 ; i++)
			servos.move(i, (i % 3 == 1) ? 90 : -90, 5000, true);
		servos.enable();
		servos.start();
		return;
	}

	if (cmd == 'w')
	{
		Serial.println("wave");
		wave();
		return;
	}


	Serial.println("?");
}
