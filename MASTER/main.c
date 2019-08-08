#include "../AX_Radio_Lab_output/configmaster.h"
#include <axm0f243.h>
#include "axm0_config.h"
#include "axm0f2_pin.h"
#include "axm0f2.h"
#include <stdbool.h>
#include <libmftypes.h>
#include <libmfradio.h>
#include <libmfflash.h>
#include <libmfwtimer.h>
#include "libmfosc.h"
#include <libmfuart.h>
#include <libmfuart0.h>
#include <libmfuart1.h>
#include <stdio.h>
#include <string.h>
#include "../COMMON/misc.h"
#include "../COMMON/configcommon.h"
#include "libmfi2cm.h"
#include "i2c.h"
#include <libminikitleds.h>

#define BME680  // COMMENT OR UNCOMMENT THIS THREE LINE FOR ENABLING / DISABLING SENSORS OR UART0 DEBUG MESSAGE
//#define BMA400
//#define UART0_DEBUG

extern void RADIO_IRQ(void);
#ifdef TX_ON_DEMAND
    uint8_t button_pressed = 0;
    const bool txondemand_mode = 1;
#else
    const bool txondemand_mode = 0;
#endif // TX_ON_DEMAND
    const bool AxM0_Mini_Kit = 1;
#define BUTTON_MASK    0x40

#ifdef BMA400
#include "../COMMON/BMA400.h"
#endif// BMA400
#ifdef BME680
#include "../COMMON/bme680.h"
#endif// BME680


uint16_t __data pkt_counter = 0;
char uart_float_buf[64];
uint8_t __data coldstart = 1;
criticalsection_t crit;

uint8_t address[4] = {192,168,0,1};
uint8_t gateway_addrss[4] = {192,168,0,0};

#ifdef BMA400
uint8_t buffer_txBMA[17] = {0};
float x = 0, y= 0, z = 0;
bool intBMA = 0;
#endif// BMA400

#ifdef BME680
uint8_t buffer_txBME[17] = {0};
struct bme680_dev gas_sensor;
int8_t rslt = BME680_OK;
struct bme680_field_data data;
float t=0, p=0, h=0;
uint16_t profil_duration;

uint8_t test_counter = 0;
uint8_t hour_framing[1440] = {0};
#endif // BME680

uint8_t test_packet[6] = { 0x00, 0x00, 0x55, 0x66, 0x77, 0x88 };

#ifdef TX_ON_DEMAND
bool deglitch_busy = 0;
#endif
struct wtimer_desc __xdata wakeup_desc;

// IRQ on PB3 (0.2) or PR5 (2.4)
#ifdef TX_ON_DEMAND
void GPIOPort3_Handler()
{
    NVIC_ClearPendingIRQ(GPIOPort3_IRQn);

    /* Serve INTR at 3.6 (PB3) */
    if(GPIO_PRT3->INTR & 0x40)
    {
        if(!deglitch_busy)
        {
            button_pressed = 1;
        }
    }

    /* clear PC4 3.6 interrupt */
    GPIO_PRT3->INTR |= 1<<6;
}
#endif // TX_ON_DEMAND

#ifdef BMA400
void GPIOPort1_Handler()
{
    NVIC_ClearPendingIRQ(GPIOPort1_IRQn);

    if(GPIO_PRT1->INTR & 0x02)
    {
            intBMA = 1;
    }

    GPIO_PRT1->INTR |= 1<<1;
}
#endif // BMA400
void transmit_packet(uint8_t *array, uint8_t len) // modified version to sent custom packet
{
    uint8_t __xdata demo_packet_[len];

    ++pkt_counter;
        memcpy(demo_packet_, array, len);
    if (framing_insert_counter)
    {
        demo_packet_[framing_counter_pos] = pkt_counter & 0xFF ;
        demo_packet_[framing_counter_pos+1] = (pkt_counter>>8) & 0xFF;
    }
    axradio_transmit(&remoteaddr, demo_packet_, len);
}

void axradio_statuschange(struct axradio_status __xdata *st)
{
    #if defined(USE_DBGLINK) && defined(DEBUGMSG)
        if (DBGLNKSTAT & 0x10)
        {
            dbglink_writestr("ST: 0x");
            dbglink_writehex16(st->status, 2, WRNUM_PADZERO);
            dbglink_writestr(" ERR: 0x");
            dbglink_writehex16(st->error, 2, WRNUM_PADZERO);
            dbglink_tx('\n');
        }
    #endif
    switch (st->status)
    {
        case AXRADIO_STAT_TRANSMITSTART:
            led0_on();
            if (st->error == AXRADIO_ERR_RETRANSMISSION)
                led2_on();
            #ifdef TX_ON_DEMAND
                if( st->error = AXRADIO_ERR_TIMEOUT )
                    deglitch_busy = 0;
            #endif
            #if RADIO_MODE == AXRADIO_MODE_SYNC_MASTER || RADIO_MODE == AXRADIO_MODE_SYNC_ACK_MASTER
                display_transmit_packet();
            #endif
            break;

        case AXRADIO_STAT_TRANSMITEND:
            led0_off();
            if (st->error == AXRADIO_ERR_NOERROR)
            {
                led2_off();
            #ifdef TX_ON_DEMAND
                deglitch_busy = 0;
            #endif
            #if RADIO_MODE == AXRADIO_MODE_ACK_TRANSMIT || RADIO_MODE == AXRADIO_MODE_WOR_ACK_TRANSMIT || RADIO_MODE == AXRADIO_MODE_SYNC_ACK_MASTER
                display_setpos(0x0d);
                display_writestr(":-)");
                #ifdef USE_DBGLINK
                    if (DBGLNKSTAT & 0x10)
                        dbglink_writestr(":-)\n");
                #endif // USE_DBGLINK
            #endif // RADIO_MODE
            }
            else if (st->error == AXRADIO_ERR_TIMEOUT)
            {
                led2_on();
                #if RADIO_MODE == AXRADIO_MODE_ACK_TRANSMIT || RADIO_MODE == AXRADIO_MODE_WOR_ACK_TRANSMIT || RADIO_MODE == AXRADIO_MODE_SYNC_ACK_MASTER
                    display_setpos(0x0d);
                    display_writestr(":-(");
                    #ifdef USE_DBGLINK
                        if (DBGLNKSTAT & 0x10)
                            dbglink_writestr(":-(\n");
                    #endif // USE_DBGLINK
                #endif // RADIO_MODE
            }
            if (st->error == AXRADIO_ERR_BUSY)
                led3_on();
            else
                led3_off();
            break;

    #if RADIO_MODE == AXRADIO_MODE_SYNC_MASTER || RADIO_MODE == AXRADIO_MODE_SYNC_ACK_MASTER
        case AXRADIO_STAT_TRANSMITDATA:
            // in SYNC_MASTER mode, transmit data may be prepared between the call to TRANSMITEND until the call to TRANSMITSTART
            // TRANSMITDATA is called when the crystal oscillator is enabled, approximately 1ms before transmission
            transmit_packet();
            break;
    #endif

        case AXRADIO_STAT_CHANNELSTATE:
            if (st->u.cs.busy)
                led3_on();
            else
                led3_off();
            break;

        default:
            break;
    }
}

static void wakeup_callback(struct wtimer_desc __xdata *desc) // read and send BME680 data
{
    desc;
    #ifdef BME680
        wakeup_desc.time += wtimer0_correctinterval(WTIMER0_PERIOD);
        wtimer0_addabsolute(&wakeup_desc);

        gas_sensor.power_mode = BME680_FORCED_MODE;
        rslt = bme680_set_sensor_mode(&gas_sensor);
        delay_ms(profil_duration);
        rslt = bme680_get_sensor_data(&data, &gas_sensor);
        p = data.pressure / 100.0;
        memcpy(&buffer_txBME[5],&data.temperature,sizeof(data.temperature)); // 32 bits float
        memcpy(&buffer_txBME[9],&p,sizeof(data.pressure)); // 32 bits float
        memcpy(&buffer_txBME[13],&data.humidity,sizeof(data.humidity)); // 32 bits float
        transmit_packet(buffer_txBME, sizeof(buffer_txBME));

    #endif
    #ifdef BMA400

    #endif // BMA400
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
        #ifdef BMA400
            GPIO_PRT1->PC = 0x00000008u;
        #else
            GPIO_PRT1->PC = 0x00000000u;
        #endif // BMA400

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
#ifdef BME680
void setup_BME680(void)
{
    gas_sensor.dev_id = BME680_I2C_ADDR_PRIMARY;
    gas_sensor.intf = BME680_I2C_INTF;
    gas_sensor.read = user_i2c_read;
    gas_sensor.write = user_i2c_write;
    gas_sensor.delay_ms = delay_ms;
    gas_sensor.amb_temp = 25;


    rslt = bme680_init(&gas_sensor);

    uint8_t set_required_settings;
    /* Set the temperature, pressure and humidity settings */
    gas_sensor.tph_sett.os_hum = BME680_OS_16X;
    gas_sensor.tph_sett.os_pres = BME680_OS_4X;
    gas_sensor.tph_sett.os_temp = BME680_OS_8X;
    gas_sensor.tph_sett.filter = BME680_FILTER_SIZE_3;
    /* Set the remaining gas sensor settings and link the heating profile */
    gas_sensor.gas_sett.run_gas = BME680_DISABLE_GAS_MEAS;
    /* Create a ramp heat waveform in 3 steps */
    gas_sensor.gas_sett.heatr_temp = 320; /* degree Celsius */
    gas_sensor.gas_sett.heatr_dur = 150; /* milliseconds */
    /* Select the power mode */
    /* Must be set before writing the sensor configuration */
    gas_sensor.power_mode = BME680_FORCED_MODE;
    /* Set the required sensor settings needed */
    set_required_settings = BME680_OSH_SEL;
    /* Set the desired sensor configuration */
    rslt = bme680_set_sensor_settings(set_required_settings,&gas_sensor);
    set_required_settings = BME680_FILTER_SEL;
    /* Set the desired sensor configuration */
    rslt = bme680_set_sensor_settings(set_required_settings,&gas_sensor);
    set_required_settings = BME680_GAS_SENSOR_SEL;
    /* Set the desired sensor configuration */
    rslt = bme680_set_sensor_settings(set_required_settings,&gas_sensor);
    /* Set the required sensor settings needed */
    set_required_settings =  BME680_OST_SEL | BME680_OSP_SEL;
    /* Set the desired sensor configuration */
    rslt = bme680_set_sensor_settings(set_required_settings,&gas_sensor);
    /* Set the power mode */
    rslt = bme680_set_sensor_mode(&gas_sensor);
    bme680_get_profile_dur(&profil_duration,&gas_sensor);
}
#endif

int main(void)
{
    uint8_t i;
    __enable_irq();
    wtimer_init();

    if (coldstart)
    {
        axradio_setup_pincfg3();
        #ifdef BME680
            wakeup_desc.handler = wakeup_callback;
        #endif // BME680

        #ifdef TX_ON_DEMAND
            if(txondemand_mode)
            {
                GPIO_PRT3->PC |= 2 << 18;               // Configure push button to high pull up
                GPIO_PRT3->DR |= 1<<6;                  // Configure push button to high pull up
                GPIO_PRT3->INTR_CFG |= 2<<12;           // Configure Push button to falling edge triggered
                NVIC_EnableIRQ(GPIOPort3_IRQn);         // Enable GPIOPort3 IRQ
                __enable_irq();
            }
        #endif // TX_ON_DEMAND

        #ifdef BMA400
            GPIO_PRT1->PC |= 1 << 3; // 1.1 INPUT
            GPIO_PRT1->INTR_CFG |= 1<<2; // 1.1 to rising edge
            NVIC_EnableIRQ(GPIOPort1_IRQn);
            __enable_irq();
        #endif // BMA400

        i = axradio_init();
        if (i != AXRADIO_ERR_NOERROR)
        {
            if (i == AXRADIO_ERR_NOCHIP)
            {
                goto terminate_error;
            }
            goto terminate_radio_error;
        }
        axradio_set_local_address(&localaddr);
        axradio_set_default_remote_address(&remoteaddr);
        /* IMO calibration initialization and setup code start */
        /* Set Pin 0.0 (VTCXO pin) */
        GPIO_PRT0->DR_SET |= (1 << AXM0F2_VTCXO_PIN);

        /* IMO and ILO calibration setup */
        setup_osc_calibration(AXM0XX_HFCLK_CLOCK_FREQ, CLKSRC_RSYSCLK);
        /* IMO calibration initialization and setup code end */

        i = axradio_set_mode(RADIO_MODE);
        if (i != AXRADIO_ERR_NOERROR)
            goto terminate_radio_error;
        #ifdef BME680
        #if defined(WTIMER0_PERIOD)
            wakeup_desc.time = wtimer0_correctinterval(WTIMER0_PERIOD);
            wtimer0_addrelative(&wakeup_desc);
        #endif
        #endif // BME680
        #ifdef UART0_DEBUG
        uart_timer1_baud(CLKSRC_FRCOSC, 115200, 48000000UL);
        uart0_init(1, 8, 1);
        delay_ms(5000); // just a wait to have time to connect HTERM on PC.
        uart0_writestr("\rUART OK\r");
        #endif // UART0_DEBUG

        uint8_t i2c_status = i2c1_init(AXM0_I2C_MASTER,AXM0_I2C_FAST_MODE,AXM0_I2C_INT_DIV_0, AXM0_I2C_INTERNAL_CLK);
        #ifdef UART0_DEBUG
            uart0_writestr("I2C OK \r");
        #endif // UART0_DEBUG
        #ifdef BME680
            buffer_txBME[0] = address[0];
            buffer_txBME[1] = address[1];
            buffer_txBME[2] = address[2];
            buffer_txBME[3] = address[3];
            buffer_txBME[4] = (uint8_t)'E';
            setup_BME680();
            #ifdef UART0_DEBUG
                uart0_writestr("BME680 OK\r");
            #endif // UART0_DEBUG
        #endif // BME680
        #ifdef BMA400
            if(BMA400_isReady())
            {
                buffer_txBMA[0] = address[0];
                buffer_txBMA[1] = address[1];
                buffer_txBMA[2] = address[2];
                buffer_txBMA[3] = address[3];
                buffer_txBMA[4] = (uint8_t)'A';
                BMA400_init();
                #ifdef UART0_DEBUG
                    uart0_writestr("BMA400 READY\r");
                #endif // UART0_DEBUG
            }else
            {
                #ifdef UART0_DEBUG
                    uart0_writestr("BMA400 NOT CONNECTED");
                #endif // UART0_DEBUG
            }
            BMA400_setPowerMode(LOW_POWER);
        #endif // BMA400
        axradio_setup_pincfg2();

    }
    else
    {
//        //  Warm Start
//        ax5043_commsleepexit();
//
//        NVIC_EnableIRQ(GPIOPort2_IRQn);
//        /* ENABLE RIRQ PR5 2.4 INTERRUPT */
//        GPIO_PRT2->INTR_CFG |= 1 << 8;
//
        /* IMO and ILO calibration setup */
        setup_osc_calibration(AXM0XX_HFCLK_CLOCK_FREQ, CLKSRC_RSYSCLK); // NEED THIS !! IF COMMENTED, TIMER FOR DEEP SLEEP WILL HAVE A BIAS.

//        #ifdef MINI_KIT
//        if(txondemand_mode)
//        {
//            /* ENABLE PC4 3.6 INTERRUPT */
//            GPIO_PRT3->INTR_CFG |= 2<<12;
//            NVIC_EnableIRQ(GPIOPort3_IRQn);
//        }
//        #ifdef BMA400
//            GPIO_PRT1->INTR_CFG |= 1<<2;
//            NVIC_EnableIRQ(GPIOPort1_IRQn);
//        #endif // BMA400
//        #else
//        if(txondemand_mode)
//        {
//            /* ENABLE PB3 0.2 INTERRUPT */
//            GPIO_PRT0->INTR_CFG |= 2<<4;
//            NVIC_EnableIRQ(GPIOPort0_IRQn);
//        }
//        #endif // MIN_KIT


    }
        for(;;)
        {
            wtimer_runcallbacks();
            crit = enter_critical();
            #ifdef TX_ON_DEMAND
                if(button_pressed)
                {
                    button_pressed = 0;
                    exit_critical(crit);
                    if( !deglitch_busy )
                    {
                        deglitch_busy = 1;
                        transmit_packet();
                        display_transmit_packet();
                    }
                    continue;
                }
            #endif // TX_ON_DEMAND
            #ifdef BMA400
                if(intBMA)
                {
                    intBMA = 0;
                    exit_critical(crit);
                    BMA400_getAcc(&x,&y,&z);
                    memcpy(&buffer_txBMA[5],&x,sizeof(x)); // 32 bits float
                    memcpy(&buffer_txBMA[9],&y,sizeof(y)); // 32 bits float
                    memcpy(&buffer_txBMA[13],&z,sizeof(z)); // 32 bits float
                    transmit_packet(buffer_txBMA,sizeof(buffer_txBMA));
                    BMA400_setPowerMode(LOW_POWER);
                }
            #endif // BMA400
            uint8_t flg = WTFLAG_CANSTANDBY;
            #ifdef MCU_SLEEP
                if (axradio_cansleep())
                {
                    flg |= WTFLAG_CANSLEEP;
                    //flg |= WTFLAG_CANSLEEPCONT; // sleep cont does not seems to work. Sleep with almost empty warm start is ok.
                }
            #endif // MCU_SLEEP
            wtimer_idle(flg);
            __enable_irq();
        } // for(;;)
    terminate_radio_error:

    terminate_error:
        for (;;)
        {
            wtimer_runcallbacks();
            {
                uint8_t flg = WTFLAG_CANSTANDBY;
                #ifdef MCU_SLEEP
                    if (axradio_cansleep())
                    {
                        flg |= WTFLAG_CANSLEEP;
                    }
                #endif
                wtimer_idle(flg);
            }
        }
}
