#include <AP_Timesync/AP_Timesync.h>

AP_Timesync::AP_Timesync()
{
    if(_singleton != nullptr){
        return;
    }
    _singleton = this;
}

void AP_Timesync::handle_sync(mavlink_ptp_timesync_t &packet)
{

}

void AP_Timesync::handle_follow_up(mavlink_ptp_timesync_t &packet)
{

}

void AP_Timesync::handle_delay_response(mavlink_ptp_timesync_t &packet)
{

}




AP_Timesync *AP_Timesync::_singleton;

namespace AP {
AP_Timesync &ptp()
{
    return *AP_Timesync::get_singleton();
}
};