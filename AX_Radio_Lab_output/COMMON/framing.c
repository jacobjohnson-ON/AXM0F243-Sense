//#include "framing.h"
//
//uint8_t* create_frame(enum pkt_type pktType, uint8_t *data, uint8_t len,uint8_t counterPkt)
//{
//    uint8_t frame[len+3];
//    memcpy(&frame[0],counterPkt,1);
//    memcpy(&frame[1],pktType,2);
//    switch(pktType)
//    {
//    case BME_GAS:
//        break;
//    case BME_NGAS:
//        break;
//    case BMA400:
//        break;
//    default:
//        break;
//    }
//}
