/* Warning: This file is automatically generated by AX-RadioLAB.
	Manual changes are overwritten! */

#include "../COMMON/axradio.h"


const struct axradio_address __code remoteaddr = {
	{ 0x33, 0x34, 0x00, 0x00}
};
const struct axradio_address_mask __code localaddr = {
	{ 0x32, 0x34, 0x00, 0x00},
	{ 0xff, 0xff, 0x00, 0x00}
};


const uint8_t __code framing_insert_counter = 0;
const uint8_t __code framing_counter_pos = 0;


const uint8_t __code demo_packet[] =  { 0x00, 0x00, 0x55, 0x66, 0x77, 0x88 };


const uint16_t __code lpxosc_settlingtime = 3000;
