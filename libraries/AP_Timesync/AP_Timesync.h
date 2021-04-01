#pragma once

#include <AP_HAL/AP_HAL.h>
#include <GCS_MAVLink/GCS.h>

#define PTP_SYNC 0x00
#define PTP_FOLLOW_UP 0x08
#define PTP_DELAY_REQUEST 0x01
#define PTP_DELAY_RESPONSE 0x09
#define TAKEOFF_TIME 0x03
#define PTP_DEFAULT_STATE 255

struct timespec {
	        long	time_sec;		/* seconds */
        	long	time_nsec;	/* and nanoseconds */
        };

class AP_Timesync
{
    public:
        AP_Timesync();
        static AP_Timesync *get_singleton() {
            return _singleton;
        }
        void handle_ptp_timesync(GCS_MAVLINK &link, const mavlink_message_t &msg);
        void handle_sync(mavlink_ptp_timesync_t &packet);
        void handle_follow_up(class GCS_MAVLINK &, mavlink_ptp_timesync_t &packet);
        void handle_delay_response(mavlink_ptp_timesync_t &packet);        
        void time_add(struct timespec *output, const struct timespec *left, const struct timespec *right);
        void time_sub(struct timespec *output, const struct timespec *left, const struct timespec *right);
        void get_time(struct timespec *get);
        void set_time(struct timespec &set);

    private:
        GCS_MAVLINK *_request_sending_link;
        static struct timespec t1, t2, t3, t4;
        static AP_Timesync *_singleton;
        static struct timespec base_time;
        static struct timespec sync_time;

        static uint64_t base_boot;

};

namespace AP {
    AP_Timesync &ptp();
};

