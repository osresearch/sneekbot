/** \file
 * LX-16A bus servo control library.
 *
 * Could use the hardware serial port on the Teensy, but we want
 * to be able to turn the pins on and off, so we're using software serial.
 */
#include "ServoLX.h"
#include <Arduino.h>
#include <stdint.h>


// Commands from the data sheet are in decimal, not hex
#define SERVOLX_ID_WRITE 13
#define SERVOLX_ID_READ 14
#define SERVOLX_POS_READ 28
#define SERVOLX_LOAD_OR_UNLOAD_WRITE 31


void ServoLX::begin()
{
	Serial1.begin(115200);
	pinMode(tx_pin, INPUT_PULLUP);
}


void ServoLX::send_raw(const uint8_t * bytes, size_t len)
{
	pinMode(tx_pin, OUTPUT);
	pinMode(11, OUTPUT);
	//digitalWrite(11, 1);
	Serial1.write(bytes, len);
	pinMode(tx_pin, INPUT_PULLUP);
	digitalWrite(tx_pin, 1);
	//digitalWrite(11, 0);
}

// look for a real packet
int ServoLX::recv_raw(uint32_t timeout_ms)
{
	const uint32_t start = millis();

	while(millis() - start < timeout_ms)
	{
		if (!Serial1.available())
			continue;
		int b = Serial1.read();
		if (b == -1)
			continue;

		return (uint8_t) b;
	}

	return -1;
}

int ServoLX::recv()
{
	// discard any previosly received data
	while(Serial1.available())
		Serial1.read();
	//Serial1.clear();

	uint8_t len = 255;
	uint8_t offset = 0;
	uint8_t checksum = 0;

	digitalWrite(11, 1);

	while(1)
	{
		const int rx = recv_raw(5);
		if (rx < 0)
		{
			digitalWrite(11, 0);
			Serial.println("timeout");
			return -1;
		}

		const uint8_t b = rx;
		if(0)
		{
			Serial.print(offset);
			Serial.print(" ");
			Serial.println(b, HEX);
		}

		// wait for a message start
		if (offset == 0)
		{
			if (b == 0x55)
				buf[offset++] = b;
			continue;
		}
		if (offset == 1)
		{
			if (b == 0x55)
				buf[offset++] = b;
			else
				offset = 0;
			continue;
		}

		// inside the body, store the byte and checksum
		buf[offset] = b;
		checksum += b;

		// if this is the length byte, record how many we should get
		if (offset == 3)
		{
			len = b + 3;
			//Serial.print("len="); Serial.println(len);
		}

		// if we have received the length byte, and we have all
		// of the data, then we're done
		if (++offset == len)
			break;
	}

	digitalWrite(11, 0);

	if (checksum != 0xFF)
	{
		Serial.print(checksum, HEX);
		Serial.println(" checksum fail");
		return -1;
	}
	
	//Serial.print(len);
	//Serial.println(" bytes ok");
	return len;
}


void ServoLX::send(uint8_t id, uint8_t cmd, const uint8_t * raw_bytes, size_t len)
{
	uint8_t bytes[len + 6];

	bytes[0] = 0x55;
	bytes[1] = 0x55;
	bytes[2] = id;
	bytes[3] = len + 3; // length byte + data + checksum byte
	bytes[4] = cmd;

	uint8_t checksum = bytes[2] + bytes[3] + bytes[4];

	for(size_t i = 0 ; i < len ; i++)
	{
		uint8_t c = raw_bytes[i];
		bytes[5+i] = c;
		checksum += c;
	}
	bytes[5 + len] = ~checksum;

	send_raw(bytes, sizeof(bytes));
}

void ServoLX::enable(uint8_t which, uint8_t mode)
{
	send(which, SERVOLX_LOAD_OR_UNLOAD_WRITE, &mode, 1);
}


int ServoLX::position(uint8_t which)
{
	send(which, SERVOLX_POS_READ);

	while(1)
	{
		if (recv() < 0)
			return -1;

		if (msg->cmd != SERVOLX_POS_READ)
			continue;

		// ignore our query
		if (msg->len == 3)
			continue;

		return (msg->buf[1] << 8) | msg->buf[0];
	}
}

int ServoLX::getid()
{
	send(SERVOLX_BROADCAST, SERVOLX_ID_READ);

	while(1)
	{
		if (recv() < 0)
			return -1;

		if (msg->cmd != SERVOLX_ID_READ)
			continue;
		if (msg->id == SERVOLX_BROADCAST)
			continue;

		return msg->buf[0];
	}
}

void ServoLX::setid(uint8_t which)
{
	send(SERVOLX_BROADCAST, SERVOLX_ID_WRITE, &which, 1);
}

#if 0
	// set the id of a SINGLE connected servo
	void setid(int id);

	// move a single servo
	void move(int ms, int which, int position);

	// move all servos in a synchronized fashion
	void move(int ms, const int positions[]);

	// read the temperature, volts or position of all servos
	void temp(int temps[]);
	void volts(int volts[]);
	void position(int positions[]);

	// read the position of one servo
	int position(int which);

	// enable all, one or specific servos
	void enable(void);
	void enable(int which);
	void enable(const int which[]);

	// disable all or one specific servos
	void disable(void);
	void disable(int which);

private:
	const int num_servos;
};
#endif
