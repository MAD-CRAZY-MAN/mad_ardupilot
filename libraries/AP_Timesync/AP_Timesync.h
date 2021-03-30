#pragma once

#include <GCS_MAVLink/GCS.h>

#define PTP_SYNC 0x00
#define PTP_FOLLOW_UP 0x08
#define PTP_DELAY_REQUSET 0x01
#define PTP_DELAY_RESPONSE 0x09
#define PTP_DEFAULT_STATE 255

class AP_Timesync
{
    public:
        AP_Timesync();
        static AP_Timesync *get_singleton() {
            return _singleton;
        }
        void handle_sync(mavlink_ptp_timesync_t &packet);
        void handle_follow_up(mavlink_ptp_timesync_t &packet);
        void handle_delay_response(mavlink_ptp_timesync_t &packet);        

    private:
        struct timestamp{

        };

        static struct timestamp t1, t2, t3, t4;
        static AP_Timesync *_singleton;
};

namespace AP {
    AP_Timesync &ptp();
};