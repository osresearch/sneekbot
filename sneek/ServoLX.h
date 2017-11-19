/** \file
 * LX-16A bus servo control library.
 *
 * Uses the hardware serial port on the Arduino.
 */
#pragma once

#include <SoftwareSerial.h>

#define SERVOLX_BROADCAST 0xFE

#define SERVOLX_TIMEOUT 0x8000


typedef struct {
	uint8_t hdr0; // 0x55
	uint8_t hdr1; // 0x55
	uint8_t id;
	uint8_t len;
	uint8_t cmd;
	uint8_t buf[];
} servolx_msg_t;


class ServoLX
{
public:
	ServoLX(int rx, int tx) :
		rx_pin(rx),
		tx_pin(tx),
		serial(rx,tx),
	 	msg((servolx_msg_t*) buf)
	{}

	~ServoLX() {}

	void begin();

	// set the id of a SINGLE connected servo
	void setid(uint8_t id);
	int getid();

	// move a single servo or queue a movement
	void move(uint8_t which, int ms, int position);
	void move_wait(uint8_t which, int ms, int position);
	void move_start();

	// read the temperature, volts or position of a servos
	int temp(uint8_t which);
	int volts(uint8_t which);
	int position(uint8_t which);

	// enable/disable the servos
	void enable(uint8_t which = SERVOLX_BROADCAST, uint8_t mode = 1);
	void disable(uint8_t which = SERVOLX_BROADCAST) {
		enable(which, 0);
	}

private:
	const int rx_pin;
	const int tx_pin;
	SoftwareSerial serial;

	// receive buffer for the incoming messages
	uint8_t buf[64];
	servolx_msg_t * const msg;

	// receive a byte from the serial port within timeout_ms or -1
	int recv_raw(uint32_t timeout_ms);

	// receive a packet from the serial port, returns the length
	// or -1 if nothing was received
	int recv();

	// send a raw string to the serial port; header and crc must be
	// already included in the buffer
	void send_raw(const uint8_t * buf, size_t len);

	// send a message, adding in the header and crc
	void send(uint8_t id, uint8_t cmd, const uint8_t * buf = NULL, size_t len = 0);
};
