#include <AP_Timesync/AP_Timesync.h>
extern const AP_HAL::HAL& hal;

timespec AP_Timesync::t1;
timespec AP_Timesync::t2;
timespec AP_Timesync::t3;
timespec AP_Timesync::t4;

AP_Timesync *AP_Timesync::_singleton;

AP_Timesync::AP_Timesync()
{
    if(_singleton != nullptr){
        return;
    }
    _singleton = this;
}

void AP_Timesync::handle_ptp_timesync(GCS_MAVLINK &link, const mavlink_message_t &msg)
{
    mavlink_ptp_timesync_t packet;
    mavlink_msg_ptp_timesync_decode(&msg, &packet);

    static uint8_t msg_type = PTP_DEFAULT_STATE;
    
    msg_type = packet.msg_type;

    switch(msg_type){
        case PTP_SYNC: {
            handle_sync(packet);
            break;
        }
        case PTP_FOLLOW_UP: {
            handle_follow_up(link, packet);
            break;
        }
        case PTP_DELAY_REQUEST: {
            handle_delay_response(packet);
            break;
        }
        default:
            break;
    }
}

void AP_Timesync::handle_sync(mavlink_ptp_timesync_t &packet)
{
   // t2.tv_sec = //sync 받은 sec
    //t2.tv_nsec = //sync 받은 nsec
   
}

void AP_Timesync::handle_follow_up(GCS_MAVLINK &link, mavlink_ptp_timesync_t &packet)
{
    _request_sending_link = &link;
 
    t1.tv_sec = packet.time_sec;
    t1.tv_nsec = packet.time_nsec;
    //delay_request.time_sec = //get time sec
    //delay_request.time_nsec = //get time nsec

    //t3.tv_sec = get time sec;
    //t3.tv_nsec = get time nsec
    
    mavlink_ptp_timesync_t delay_request;
    
    delay_request.msg_type = PTP_DELAY_REQUEST;
    delay_request.seq = 2;
    //delay_request.time_sec = t3.tv_sec;
    //delay_request.time_nsec = t3.tv_nsec;

    delay_request.time_sec = t1.tv_sec;
    delay_request.time_nsec = t1.tv_nsec;
    mavlink_msg_ptp_timesync_send(
        _request_sending_link->get_chan(),
        delay_request.msg_type,
        delay_request.seq,
        delay_request.time_sec,
        delay_request.time_nsec
        );
    hal.uartA->printf("t1.sec: %d, t1.usec: %d\r\n", t1.tv_sec, t1.tv_nsec);
    hal.uartA->printf("recieved follow up\r\n");
}

void AP_Timesync::handle_delay_response(mavlink_ptp_timesync_t &packet)
{
    hal.uartA->printf("recieved delay response\r\n");
}

namespace AP {
AP_Timesync &ptp()
{
    return *AP_Timesync::get_singleton();
}
};