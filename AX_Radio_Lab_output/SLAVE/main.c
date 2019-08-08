/**
*******************************************************************************************************
* @file SLAVE\main.c
* @brief Code skeleton for SLAVE module, illustrating reception of packets.
*        The packet format is determined by AX-RadioLAB_output\config.c, produced by the AX-RadioLab GUI
* @internal
* @author   Thomas Sailer, Janani Chellappan, Srinivasan Tamilarasan, Anand Agrawal
* $Rev: $
* $Date: $
********************************************************************************************************
* Copyright 2016 Semiconductor Components Industries LLC (d/b/a “ON Semiconductor”).
* All rights reserved.  This software and/or documentation is licensed by ON Semiconductor
* under limited terms and conditions.  The terms and conditions pertaining to the software
* and/or documentation are available at http://www.onsemi.com/site/pdf/ONSEMI_T&C.pdf
* (“ON Semiconductor Standard Terms and Conditions of Sale, Section 8 Software”) and
* if applicable the software license agreement.  Do not use this software and/or
* documentation unless you have carefully read and you agree to the limited terms and
* conditions.  By using this software and/or documentation, you agree to the limited
* terms and conditions.
*
* THIS SOFTWARE IS PROVIDED "AS IS".  NO WARRANTIES, WHETHER EXPRESS, IMPLIED
* OR STATUTORY, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF
* MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE.
* ON SEMICONDUCTOR SHALL NOT, IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL,
* INCIDENTAL, OR CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
* @endinternal
*
* @ingroup TEMPLATE_FIRMWARE_5043
*
* @details
*/

#include "../AX_Radio_Lab_output/configslave.h"

#include <axm0f243.h>
#include "axm0_config.h"
#include "axm0f2_pin.h"
#include "axm0f2.h"
#include <libmftypes.h>
#include <libmfradio.h>
#include <libmfflash.h>
#include <libmfwtimer.h>
#include "libmfosc.h"

#ifdef USE_COM0
#include <libmfuart0.h>
#endif // USE_COM0

#ifdef USE_LCD
#include <libaxlcd2.h>
#endif // USE_LCD

#include <libmfdbglink.h>
#include <libmfuart.h>
#include <libmfuart0.h>
#include <libmfuart1.h>

#define ADJUST_FREQREG
#define FREQOFFS_K 3

#if defined(USE_LCD) || defined(USE_COM0)
#define USE_DISPLAY
#endif // defined(USE_LCD) || defined(USE_COM0)

#include "../COMMON/display_com0.h"

#include <libminikitleds.h>

//#include <stdlib.h>

#include "../COMMON/misc.h"
#include "../COMMON/configcommon.h"

// TODO : Verify the function display_writenum16 in libaxdvk2.a
#include <stdbool.h>
const bool txondemand_mode = 0;
const bool AxM0_Mini_Kit = 0;
extern void RADIO_IRQ(void);


uint8_t __data coldstart = 1; // caution: initialization with 1 is necessary! Variables are initialized upon _sdcc_external_startup returning 0 -> the coldstart value returned from _sdcc_external startup does not survive in the coldstart case
uint16_t __data pkts_received = 0, pkts_missing = 0;
uint8_t vBuffer[4] = {0};
char txBuffer[64];
#define BME680
#define BMA400

#ifdef BMA400
float x = 0, y=0, z=0;
#endif // BMA400
#ifdef BME680
float t=0, p=0, h=0;
#endif // BME680
// Debug Link Configuration
    #if defined USE_DBGLINK
        void debuglink_init_axm0(void)
        {
            dbglink_timer0_baud(0, 9600, AXM0XX_HFCLK_CLOCK_FREQ);  // HS OSC, Baud rate, Clock
            dbglink_init(0, 8, 1);                              	// TImer No., Word length, Stop bit
        }
    #endif // USE_DBGLINK


// TODO : If IAR will this condition is valid

void axradio_statuschange(struct axradio_status __xdata *st)
{
    //    static uint16_t rssi_ratelimit;
    #if defined(USE_DBGLINK) && defined(DEBUGMSG)
        if (DBGLNKSTAT & 0x10 && st->status != AXRADIO_STAT_CHANNELSTATE) {
            dbglink_writestr("ST: 0x");
            dbglink_writehex16(st->status, 2, WRNUM_PADZERO);
            dbglink_writestr(" ERR: 0x");
            dbglink_writehex16(st->error, 2, WRNUM_PADZERO);
            dbglink_tx('\n');
        }
    #endif
    switch (st->status)
    {
        case AXRADIO_STAT_RECEIVE:
            #if RADIO_MODE == AXRADIO_MODE_SYNC_SLAVE || RADIO_MODE == AXRADIO_MODE_SYNC_ACK_SLAVE
            switch (st->error)
            {
                case AXRADIO_ERR_TIMEOUT:
                    led2_on();
                    // fall through

                case AXRADIO_ERR_NOERROR:
                case AXRADIO_ERR_RESYNCTIMEOUT:
                        led0_off();
                    break;

                case AXRADIO_ERR_RESYNC:
                    axradio_set_freqoffset(0);
                    // fall through

                case AXRADIO_ERR_RECEIVESTART:
                        led0_on();
                    break;

                default:
                    break;
            }

            if (st->error == AXRADIO_ERR_NOERROR)
            {
                ++pkts_received;
                led2_off();
                #ifdef USE_DISPLAY
                            display_received_packet(st);
                #endif // USE_DISPLAY
            }
            #ifdef USE_DBGLINK
                switch (st->error)
                {
                    case AXRADIO_ERR_RESYNCTIMEOUT:
                        if (DBGLNKSTAT & 0x10)
                            dbglink_writestr("RESYNC Timeout\n");
                        break;

                    case AXRADIO_ERR_RESYNC:
                        if (DBGLNKSTAT & 0x10)
                            dbglink_writestr("RESYNC\n");
                        break;

                    default:
                        break;
                }
            #endif // USE_DBGLINK
            #else  // RADIO_MODE
            {
                uint8_t retran = (st->error != AXRADIO_ERR_NOERROR);
                ++pkts_received;
                led0_toggle();
                if(st->error == AXRADIO_ERR_NOERROR)
                {
                    #ifdef BME680
                    if((st->u.rx.pktdata[0] == (uint8_t)'E'))
                    {
                        memcpy(vBuffer,&(st->u.rx.pktdata[1]),sizeof(vBuffer));
                        t = *(float *)&vBuffer;
                        memcpy(vBuffer,&(st->u.rx.pktdata[5]),sizeof(vBuffer));
                        p = *(float *)&vBuffer;
                        memcpy(vBuffer,&(st->u.rx.pktdata[9]),sizeof(vBuffer));
                        h = *(float *)&vBuffer;
                        sprintf(txBuffer,"T: %.2f degC , P: %.2f hPa , Z : %.2f %rH \r",t,p,h);
                        uart0_writestr(txBuffer);
                    }

                    #endif
                    #ifdef BMA400
                    if((st->u.rx.pktdata[0] == (uint8_t)'A'))
                    {
                        memcpy(vBuffer,&(st->u.rx.pktdata[1]),sizeof(vBuffer));
                        x = *(float *)&vBuffer;
                        memcpy(vBuffer,&(st->u.rx.pktdata[5]),sizeof(vBuffer));
                        y = *(float *)&vBuffer;
                        memcpy(vBuffer,&(st->u.rx.pktdata[9]),sizeof(vBuffer));
                        z = *(float *)&vBuffer;
                        sprintf(txBuffer,"X: %.2f , Y: %.2f , Z : %.2f \r",x,y,z);
                        uart0_writestr(txBuffer);
                    }
                    #endif
                }
                #ifdef USE_DISPLAY
                    if (st->error == AXRADIO_ERR_NOERROR)
                        retran = display_received_packet(st) || retran;
                #endif // USE_DISPLAY

                if (retran)
                    led2_on();
                else
                    led2_off();
            }
            #endif // RADIO_MODE
            #ifdef USE_DBGLINK
                if (st->error == AXRADIO_ERR_NOERROR)
                    dbglink_received_packet(st);
            #endif
            // Frequency Offset Correction
            {
                int32_t foffs = axradio_get_freqoffset();
                    foffs -= (st->u.rx.phy.offset)>>(FREQOFFS_K); //adjust RX frequency by low-pass filtered frequency offset
                // reset if deviation too large
                if (axradio_set_freqoffset(foffs) != AXRADIO_ERR_NOERROR)
                    axradio_set_freqoffset(0);
            }
            break;
#if 0
    case AXRADIO_STAT_CHANNELSTATE:
        if (st->u.cs.busy)
            led3_on();
        else
            led3_off();
        {
            uint16_t t = wtimer0_curtime();
            #if WTIMER0_CLKSRC == CLKSRC_LPXOSC
                #define RSSIRATE (uint16_t)(32768/2)
            #elif WTIMER0_CLKSRC == CLKSRC_LPOSC
                #define RSSIRATE (uint16_t)(640/2)
            #else
                #error "unknown wtimer0 clock source"
            #endif
            if ((uint16_t)(t - rssi_ratelimit) < RSSIRATE)
                break;
            rssi_ratelimit = t;
        }
        display_setpos(0x48);
        display_writestr("R:");
        display_tx(st->u.cs.busy ? '*' : ' ');
        display_writenum16(st->u.cs.rssi, 5, WRNUM_SIGNED);
        break;
#endif

	case AXRADIO_STAT_TRANSMITSTART:
        led3_on();
        break;

    case AXRADIO_STAT_TRANSMITEND:
        led3_off();
        break;

    default:
        break;
    }
}


uint8_t _axm0f2_external_startup(void)
{
    uint8_t     start_cause = get_startcause();


    if((start_cause == STARTCAUSE_COLDSTART) ||
       (start_cause == STARTCAUSE_SWRESET)   ||
       (start_cause == STARTCAUSE_WATCHDOGRESET)) {

		//   __disable_irq();

	    /* Set clock source and prescalar for wakeupTimer0 */
	    wtimer0_setclksrc(WTIMER0_CLKSRC, WTIMER0_PRESCALER);
	    /* Set clock source and prescalar for wakeupTimer1 */
	    wtimer1_setclksrc(CLKSRC_FRCOSC, 7);


	    /** \brief config port0
	     *  0.0 --> Radio-VTCXO --> High Imp Analog
	     *  0.1, 0.3, 0.7 --> High Imp Analog
	     *  0.2, 0.4, 0.5, 0.6  ---> high imp digital
	     *
	     *  PORT0 --> GPIO
	     */

	    HSIOM->PORT_SEL0 = 0x00000000u;
	    GPIO_PRT0->DR = 0x00000000u;
	    GPIO_PRT0->PC = 0x00049040u;
		GPIO_PRT0->PC2 = 0x00000000u;

	    /** \brief config port1
	     *  high imp analog
	     *
	     *  PORT1 --> GPIO
	     *
	     */

	    HSIOM->PORT_SEL1 = 0x00000000u;
	    GPIO_PRT1->DR = 0x00000000u;
	    GPIO_PRT1->PC = 0x00000000u;
		GPIO_PRT1->PC2 = 0x00000000u;

		/** \brief config port3
	     *  3_6 --> push button
	     *  3_7 --> LED
	     *
	     */

	    HSIOM->PORT_SEL3 &= ~0xFF000000u;
	    GPIO_PRT3->DR |= 0x00000040u;
	    GPIO_PRT3->PC |= 0x00C40000u;

	    /** \brief config port4
	     *  high impedance digital
	     *  PORT4 --> GPIO
	     *
	     */
	    HSIOM->PORT_SEL4 = 0x00000000u;
	    GPIO_PRT4->DR = 0x00000000u;
	    GPIO_PRT4->PC = 0x00000249u;
		GPIO_PRT4->PC2 = 0x00000000;

        coldstart = 1; 	/* Cold start */
        return 0; 		/* Variables init required */
    }
	coldstart = 0; 	/* Warm start */
	return 1; 		/* Variables init not required */

}

int main(void)
{
    uint8_t i;

        __enable_irq();

        #if defined USE_DBGLINK
            debuglink_init_axm0();
        #endif // USE_DBGLINK

    wtimer_init();

    if (coldstart)
    {
                    axradio_setup_pincfg3();
        led0_off();
        led1_off();
        led2_off();
        led3_off();
        display_init();
        display_setpos(0);

            NVIC_EnableIRQ(GPIOPort2_IRQn);
            NVIC_EnableIRQ(GPIOPort0_IRQn);
            /* ENABLE RIRQ PR5 2.4 INTERRUPT */
            GPIO_PRT2->INTR_CFG |= 1<<8;

        i = axradio_init();
        if (i != AXRADIO_ERR_NOERROR)
        {
            if (i == AXRADIO_ERR_NOCHIP)
            {
                display_writestr("No AX5043 RF\nchip found");
                #ifdef USE_DBGLINK
                    if(DBGLNKSTAT & 0x10)
                        dbglink_writestr("No AX5043 RF chip found \n");
                #endif // USE_DBGLINK
                goto terminate_error;
            }
            goto terminate_radio_error;
        }
        display_writestr("found AX5043\n");

        #ifdef USE_DBGLINK
            if (DBGLNKSTAT & 0x10)
                dbglink_writestr("found AX5043\n");
        #endif // USE_DBGLINK

        axradio_set_local_address(&localaddr);
        axradio_set_default_remote_address(&remoteaddr);

        #if RADIO_MODE == AXRADIO_MODE_WOR_RECEIVE || RADIO_MODE == AXRADIO_MODE_WOR_ACK_RECEIVE
            // LPOSC Calibrations needs full control of the radio, so it must run while the radio is idle otherwise
            calibrate_lposc();
        #endif

        #if RADIO_MODE == MODE_SYNC_MASTER || RADIO_MODE == AXRADIO_MODE_SYNC_ACK_MASTER || RADIO_MODE == AXRADIO_MODE_SYNC_SLAVE || RADIO_MODE == AXRADIO_MODE_SYNC_ACK_SLAVE
            display_writestr("settle LPXOSC");
            #ifdef USE_DBGLINK
                if (DBGLNKSTAT & 0x10)
                    dbglink_writestr("settle LPXOSC\n");
            #endif // USE_DBGLINK
            delay_ms(lpxosc_settlingtime);
            display_clear(0x40, 16);
            display_setpos(0x40);
        #endif  // RADIO_MODE

        #ifdef USE_DISPLAY
            display_writestr("RNG=");
            display_writenum16(axradio_get_pllrange(), 2, 0);
            {
                uint8_t x = axradio_get_pllvcoi();
                if (x & 0x80) {
                    display_writestr(" VCOI=");
                    display_writehex16(x, 2, 0);
                }
            }
            delay_ms(1000); // just to show PLL RNG
            display_clear(0, 16);
            display_clear(0x40, 16);
            display_setpos(0);
        #endif // USE_DISPLAY

        #ifdef USE_DBGLINK
            if (DBGLNKSTAT & 0x10) {
                dbglink_writestr("RNG = ");
                dbglink_writenum16(axradio_get_pllrange(), 2, 0);
                {
                    uint8_t x = axradio_get_pllvcoi();
                    if (x & 0x80) {
                        dbglink_writestr("\nVCOI = ");
                        dbglink_writehex16(x, 2, 0);
                    }
                }
                dbglink_writestr("\nSLAVE\n");
            }
        #endif // USE_DBGLINK

        #if RADIO_MODE == AXRADIO_MODE_WOR_RECEIVE || RADIO_MODE == AXRADIO_MODE_WOR_ACK_RECEIVE
            display_writestr("SLAVE  RX WOR\n               ");
            #ifdef USE_DBGLINK
                if (DBGLNKSTAT & 0x10)
                    dbglink_writestr("SLAVE  RX WOR\n");
            #endif // USE_DBGLINK
        #elif RADIO_MODE == AXRADIO_MODE_SYNC_SLAVE || RADIO_MODE == AXRADIO_MODE_SYNC_ACK_SLAVE
            display_writestr("SLAVE  RX SYNC\n               ");
            #ifdef USE_DBGLINK
                if (DBGLNKSTAT & 0x10)
                    dbglink_writestr("SLAVE  RX SYNC\n");
            #endif // USE_DBGLINK
        #else  // RADIO_MODE
            display_writestr("SLAVE  RX CONT\n");
            #ifdef USE_DBGLINK
                if (DBGLNKSTAT & 0x10)
                    dbglink_writestr("SLAVE  RX CONT\n");
            #endif // USE_DBGLINK
        #endif // else RADIO_MODE

        /* IMO calibration initialization and setup code start */
        /* Set Pin 0.0 (VTCXO pin) */
        GPIO_PRT0->DR_SET |= (1 << AXM0F2_VTCXO_PIN);

        /* IMO and ILO calibration setup */
        setup_osc_calibration(AXM0XX_HFCLK_CLOCK_FREQ, CLKSRC_RSYSCLK);
        /* IMO calibration initialization and setup code end */

        i = axradio_set_mode(RADIO_MODE);
        if (i != AXRADIO_ERR_NOERROR)
            goto terminate_radio_error;

        #ifdef DBGPKT
            AX5043_IRQMASK0 = 0x41;
            AX5043_RADIOEVENTMASK0 =0x0C; // radio state changed | RXPS changed
        #endif

    }
    else
    {
        //  Warm Start
        ax5043_commsleepexit();
            NVIC_EnableIRQ(GPIOPort2_IRQn);
            NVIC_EnableIRQ(GPIOPort0_IRQn);
            /* ENABLE RIRQ PR5 2.4 INTERRUPT */
            GPIO_PRT2->INTR_CFG |= 1<<8;

            /* IMO and ILO calibration setup */
            setup_osc_calibration(AXM0XX_HFCLK_CLOCK_FREQ, CLKSRC_RSYSCLK);


    }
    axradio_setup_pincfg2();
    uart_timer1_baud(CLKSRC_FRCOSC, 115200, 48000000UL);
    uart0_init(1, 8, 1);
    delay_ms(2000);
    uart0_writestr("\rUART0 OK\r");

    for(;;)
    {
        wtimer_runcallbacks();
        __disable_irq();
        {
            uint8_t flg = WTFLAG_CANSTANDBY;
            #ifdef MCU_SLEEP
                if (axradio_cansleep())
                {
                  flg |= WTFLAG_CANSLEEP;
                  flg |= WTFLAG_CANSLEEPCONT;
                }
            #endif // MCU_SLEEP

            wtimer_idle(flg);
        }
        __enable_irq();

    }
    terminate_radio_error:
        display_radio_error(i);
        #ifdef USE_DBGLINK
            dbglink_display_radio_error(i);
        #endif // USE_DBGLINK
    terminate_error:
        for (;;)
        {
            wtimer_runcallbacks();
            {
                uint8_t flg = WTFLAG_CANSTANDBY;
                #ifdef MCU_SLEEP
                    if (axradio_cansleep()
                        #ifdef USE_DBGLINK
                            && dbglink_txidle()
                        #endif // USE_DBGLINK
                            && display_txidle())

                                flg |= WTFLAG_CANSLEEP;

                #endif // MCU_SLEEP

                wtimer_idle(flg);
            }
        }
}



