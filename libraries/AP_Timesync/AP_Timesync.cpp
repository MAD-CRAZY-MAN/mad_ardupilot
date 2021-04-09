#include <AP_Timesync/AP_Timesync.h>
extern const AP_HAL::HAL& hal;

_timespec AP_Timesync::t1;
_timespec AP_Timesync::t2;
_timespec AP_Timesync::t3;
_timespec AP_Timesync::t4;

_timespec AP_Timesync::base_time = {1617202800, 0};
_timespec AP_Timesync::sync_time;

_timespec AP_Timesync::takeoff_time = {0, 0};

uint64_t AP_Timesync::base_boot = 0;

AP_Timesync *AP_Timesync::_singleton;

AP_Timesync::AP_Timesync()
{
    if(_singleton != nullptr){
        return;
    }
    _singleton = this;
}

void AP_Timesync::handle_ptp_timesync(GCS_MAVLINK &link, mavlink_ptp_timesync_t &packet)
{
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
        case PTP_DELAY_RESPONSE: {
            handle_delay_response(packet);
            break;
        }
        case TAKEOFF_TIME: {
                takeoff_time.time_sec = packet.time_sec;
                hal.uartA->printf("takeoff time: %d", takeoff_time.time_sec);
        }
        default:
            break;
    }
}

void AP_Timesync::handle_sync(mavlink_ptp_timesync_t &packet)
{
    get_time(&t2);
    hal.uartA->printf("recieved sync\r\n");
}

void AP_Timesync::handle_follow_up(GCS_MAVLINK &link, mavlink_ptp_timesync_t &packet)
{
    _request_sending_link = &link;
    hal.uartA->printf("link: %d\r\n", (mavlink_channel_t)_request_sending_link->get_chan());
    t1.time_sec = packet.time_sec;
    t1.time_nsec = packet.time_nsec;
    
    mavlink_ptp_timesync_t delay_request;
    
    delay_request.msg_type = PTP_DELAY_REQUEST;
    delay_request.target_system = 255;
    get_time(&t3);
    delay_request.time_sec = t3.time_sec;
    delay_request.time_nsec = t3.time_nsec;
    delay_request.takeoff_t = 9999; //ignore packet

    mavlink_msg_ptp_timesync_send(
        _request_sending_link->get_chan(),
        delay_request.msg_type,
        delay_request.target_system,
        delay_request.time_sec,
        delay_request.time_nsec,
        delay_request.takeoff_t
    );

     

    hal.uartA->printf("recieved follow up\r\n");
}

void AP_Timesync::handle_delay_response(mavlink_ptp_timesync_t &packet)
{
    _timespec tmp;
    _timespec tmp_l;
    _timespec tmp_r;
    //_timespec delay;
    _timespec time_offset;

    t4.time_sec = packet.time_sec;
    t4.time_nsec = packet.time_nsec;

    //delay = ((t2-t1) + (t4 - t3))/2
    // time_sub(&tmp_l, &t2, &t1);
    // time_sub(&tmp_r, &t4, &t3);
    // time_add(&tmp, &tmp_l, &tmp_r);

    // delay.time_sec = tmp.time_sec/2;
    // delay.time_nsec = tmp.time_nsec/2;

    //offset = ((t2-t1) - (t4 - t3))/2
    time_sub(&tmp_l, &t2, &t1);
    time_sub(&tmp_r, &t4, &t3);
    time_sub(&tmp, &tmp_l, &tmp_r);

    time_offset.time_sec = tmp.time_sec/2;
    time_offset.time_nsec = tmp.time_nsec/2;

    if(tmp.time_sec % 2 == 1)
    {
        if((long)tmp.time_sec > 0){
            time_offset.time_nsec = (long)tmp.time_nsec/2 + 500000000;
        }
        else{
            time_offset.time_nsec = (long)tmp.time_nsec/2 - 500000000;
        }
    }
    hal.uartA->printf("time offset: %ld.%ld\r\n", time_offset.time_sec, time_offset.time_nsec);

    //sync time = current time - offset
    get_time(&tmp_l);
    time_sub(&tmp, &tmp_l, &time_offset);
    set_time(tmp); 
    
    hal.uartA->printf("recieved delay response\r\n");
}

void AP_Timesync::get_time(struct _timespec *get)
{
    _timespec flow;
    uint64_t usec;

    usec = AP_HAL::micros64() - base_boot;
    flow.time_sec = usec/1000000;
    flow.time_nsec = (usec%1000000) * 1000;

    time_add(&sync_time,&base_time, &flow);

    get->time_sec = sync_time.time_sec;
    get->time_nsec = sync_time.time_nsec;
}

void AP_Timesync::set_time(struct _timespec &set)
{
    base_time.time_sec = set.time_sec;
    base_time.time_nsec = set.time_nsec;

    base_boot = AP_HAL::micros64();    

    sync_time.time_sec = set.time_sec;
    sync_time.time_nsec = set.time_nsec; 
    hal.uartA->printf("t1: %ld, t2: %ld, t3: %ld, t4: %ld\r\n", t1.time_sec, t2.time_sec, t3.time_sec, t4.time_sec);
    hal.uartA->printf("sync_time: %ld.%ld\r\n", sync_time.time_sec, sync_time.time_nsec);
}

void AP_Timesync::time_add(struct _timespec *output, const struct _timespec *left, const struct _timespec *right)
{
    long sec = left->time_sec + right->time_sec;
    long nsec = left->time_nsec + right->time_nsec;

    if(sec>0 && nsec<0){
        nsec += AP_NSEC_PER_SEC;
        sec--;
    }
    if(sec<=0 && nsec < -(long)AP_NSEC_PER_SEC)
    {
        nsec += AP_NSEC_PER_SEC;
        sec--;
    }

    if(nsec >= (long)AP_NSEC_PER_SEC)
    {
        nsec -= AP_NSEC_PER_SEC;
        sec++;
    }

    output->time_sec = sec;
    output->time_nsec = nsec;
}

void AP_Timesync::time_sub(struct _timespec *output, const struct _timespec *left, const struct _timespec *right)
{
    long sec = left->time_sec - right->time_sec;
    long nsec = left->time_nsec - right->time_nsec;

    if(left->time_sec >= 0 && left->time_nsec>=0)
    {
        if((sec < 0 && nsec > 0) || (sec > 0 && nsec >= (long)AP_NSEC_PER_SEC))
        {
            nsec -= AP_NSEC_PER_SEC;
            sec++;
        }
        if(sec>0 && nsec<0){
            nsec += AP_NSEC_PER_SEC;
            sec--;
        }

    }
    else{
        if(nsec <= -(long)AP_NSEC_PER_SEC || nsec >= (long)AP_NSEC_PER_SEC)
        {
            nsec += (long)AP_NSEC_PER_SEC;
            sec--;
        }
        if((sec < 0 && nsec > 0))
        {
            nsec -= (long)AP_NSEC_PER_SEC;
            sec++;
        }
    }
    output->time_sec = sec;
    output->time_nsec = nsec;
}

namespace AP {
AP_Timesync &ptp()
{
    return *AP_Timesync::get_singleton();
}
};
