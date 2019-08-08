
//JCI = JSON Command Interface

/*
Usage

*/





#define JCI_WAITING         0x00
#define JCI_RECEIVING       0x01
#define JCI_PROCESSING      0x02
#define MAX_STR_LENGTH      1000




extern uint8_t brackets; //used by state machine to determine when to stop loading json message string
extern char json_message[MAX_STR_LENGTH]; //string for incoming json message to parse
extern uint8_t msg_len; //used to track length of incoming message, also as index into json_message
extern uint8_t c; //used to read incoming char form UART
extern uint8_t jci_state, jci_done;

extern uint8_t receive_notification_enable = 0;


extern char *request_platform_id_ack = "{\"ack\":\"request_platform_id\",\"payload\":{\"return_value\":true,\"return_string\":\"Command Valid\"}}";
extern char *request_platform_info_ack = "{\"ack\":\"request_platform_info\",\"payload\":{\"return_value\":true,\"return_string\":\"Command Valid\"}}";
extern char *toggle_receive_ack = "{\"ack\":\"toggle_receive\",\"payload\":{\"return_value\":true,\"return_string\":\"Command Valid\"}}";
extern char *toggle_receive_true = "{\"notification\":{\"value\":\"toggle_receive_notification\",\"payload\":{\"enabled\":true}}}";
extern char *toggle_receive_false = "{\"notification\":{\"value\":\"toggle_receive_notification\",\"payload\":{\"enabled\":false}}}";
extern char *platform_id = "{\"notification\":{\"value\":\"platform_id_notification\",\"payload\":{\"name\":\"Sub-GHz Multi-sense Receiver\",\"platform_id\":\"132\",\"class_id\":\"232\",\"count\":1,\"platform_version_id\":\"2.0\"}}}";
extern char *platform_info = "{\"notification\":{\"value\":\"platform_info_notification\",\"payload\":{\"firmware_ver\":\"1.0.0\",\"frequency\":\"868.3\"}}}";
//extern char *receive_notification = "{\"notification\":{\"value\":\"receive_notification\",\"payload\":{\"rssi\":-85,\"packet_error_rate\":00.000,\"data_packet\":\"DEADBEEFFACEFEED\"}}}";
//extern char *spoof_logic = "{\"notification\":{\"value\":\"platform_id\",\"payload\":{\"name\":\"Logic Gates\",\"platform_id\":\"101\",\"class_id\":\"201\",\"count\":11,\"platform_id_version\":\"2.0\",\"verbose_name\":\"Logic Gates\"}}}";


extern void jci_init();
extern void jci_dispatch();
extern void process_uart();
extern int jci_check_notification_enable();
extern void jci_send_receive_notification();
