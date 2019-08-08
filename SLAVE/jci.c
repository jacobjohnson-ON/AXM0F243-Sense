#include <libmftypes.h>
#include <axm0f2.h>
#include <libminikitleds.h>
#include "jci.h"
#include "printf.h"
#include "../COMMON/axradio.h"





uint8_t brackets;
char json_message[MAX_STR_LENGTH];
uint8_t msg_len;
uint8_t c;
uint8_t jci_state;
uint8_t jci_done;
uint8_t receive_notification_enable;
char *receive_notification;

uint8_t timeout = 0;
uint8_t timeout_value = 1000;

uint16_t __data jci_per_test_counter = 0, jci_per_test_counter_previous = 0;
extern uint16_t __data pkts_received, pkts_missing;

uint8_t per_buffer[10];
uint8_t per_reset = 0;

char req_plat_id[] = "request_platform_id";
char req_plat_info[] = "request_platform_info";
char req_tog_recv[] = "toggle_receive";
char req_tog_true[] = "true";
char nak[] = "{\"ack\":\"unknown_command\",\"payload\":{\"return_value\":false,\"return_string\":\"No Comprendo\"}}\n";


extern void jci_init(){ //this needs to happen on transition from JCI_PROCESSING to JCI_WAITING
    //need to allocate memory for initial json message buffer or clear message buffer - how to do?
    if (msg_len > 0){
        uint8_t i;
        for (i = 0; i <= msg_len; i++){
            json_message[i] = '0';
        }
    }
    msg_len = 0;
    brackets = 0;
    jci_done = 0;
    jci_state = JCI_WAITING;
    timeout = 0;
    //uart0_writestr("Intializing JCI State Machine \n");

}

extern int jci_check_notification_enable(){
    if (receive_notification_enable){
        return 1;
    }
    else{
        return 0;
    }
}

extern void jci_send_receive_notification(struct axradio_status __xdata *st){
    int i;
    if (receive_notification_enable){
        int rx_rssi = st->u.rx.phy.rssi;
        uint8_t* addr = st->u.rx.pktdata;

        float t=0, p=0, h=0;
        t = *(float *)&(st->u.rx.pktdata[5]);
        p = *(float *)&(st->u.rx.pktdata[9]);
        h = *(float *)&(st->u.rx.pktdata[13]);


        printf("{\"notification\":{\"value\":\"receive_notification\",\"payload\":{\"sensor_id\":\"0x00%d\",\"sensor_type\":\"multi\",\"rssi\":%d,\"data\":{\"temperature\":%f,\"pressure\":%f,\"humidity\":%f,\"soil\":0}}}}\n",addr[3],rx_rssi,t,p,h);

    }
}

void jci_dispatch(){


    if(strstr(json_message,req_plat_id)){
        uart0_writestr(request_platform_id_ack);
        uart0_writestr("\n");
        uart0_writestr(platform_id);
        uart0_writestr("\n");

    }
    else if (strstr(json_message, req_plat_info)){
        uart0_writestr(request_platform_info_ack);
        uart0_writestr("\n");
        uart0_writestr(platform_info);
        uart0_writestr("\n");
    }
    else if (strstr(json_message,req_tog_recv)){
        uart0_writestr(toggle_receive_ack);
        uart0_writestr("\n");

        if (strstr(json_message, req_tog_true)){
            receive_notification_enable = 1;
            uart0_writestr(toggle_receive_true);
            uart0_writestr("\n");
            per_reset = 1;
            led0_on();
        }
        else{
            receive_notification_enable = 0;
            uart0_writestr(toggle_receive_false);
            uart0_writestr("\n");
            led0_off();
        }
    }
    else{
        printf(nak);
    }

    jci_done = 1;
}

extern void process_uart()
{
    switch (jci_state){

        case JCI_WAITING:
            if (uart0_rxcount()){
                c = uart0_rx();
                if (c == '{'){
                    timeout ++;
                    brackets++;
                    json_message[msg_len] = c;
                    msg_len++;
                    jci_state = JCI_RECEIVING;
                    //uart0_writestr("Received {, loading input string \n");
                    break;
                }
            }
            break;

        case JCI_RECEIVING:
            timeout++;
            if (uart0_rxcount()){
                c = uart0_rx();  //grab next char in UART buffer. What happens if there is more than one? Current assumption is that processor loops and handles faster than buffer stacks up.
                if (c == '{'){
                   // uart0_writestr("New Bracket, brackets: ");
                    //uart0_tx(brackets);
                   // uart0_writestr("\n");
                    brackets++;
                }
                else if (c =='}'){
                    //uart0_writestr("New Bracket, brackets: ");
                   // uart0_tx(brackets);
                   // uart0_writestr("\n");
                    brackets--;
                }
                if (c == '\n' || c == '\r'){
                    break;
                }
                else{
                    json_message[msg_len] = c;
                    msg_len++;
                }

            }
            if (brackets == 0){ // brackets will only be zero after last bracket is closed. What if a bracket is missed? Need to handle exception there.
                jci_state = JCI_PROCESSING;
                //uart0_writestr("brackets 0, initiating parsing\n");
                jci_dispatch();
            }
            if ((msg_len > 100) || (timeout > timeout_value)) {
                //uart0_writestr("Message too long, resetting");
                printf(nak);
                jci_init();
            }
            break; //what if the message is too long? Send error then restart JSON state machine

        case JCI_PROCESSING:
            //what happens here? if we get a command while running other - do nothing, let UART buffer fill up
            if (jci_done){
                jci_init();
            }
            else
                break;
    }
}



