//
// Created by fzy on 23-5-26.
//

#include "Timestamp.h"
#include <stdio.h>


using namespace netflow::base;

Timestamp Timestamp::now() {
    std::timespec ts;
    std::timespec_get(&ts, TIME_UTC);
    return Timestamp(ts);
}

std::string Timestamp::toFormattedString(bool isShowNanosecond){
    char buff[100] = {0};
    /*
    struct tm tm_time = std::gmtime(time_.tv_sec);

    if (isShowNanosecond) {
        snprintf(buff, sizeof buff, "%4d%02d%02d %02d:%02d:%02d.%09ld",
                 tm_time.tm_year + 1900, tm_time.tm_mon + 1, tm_time.tm_mday,
                 tm_time.tm_hour, tm_time.tm_min, tm_time.tm_sec, time_.tv_nsec);
    }
    else {
        snprintf(buff, sizeof buff, "%4d%02d%02d %02d:%02d:%02d",
                 tm_time.tm_year + 1900, tm_time.tm_mon + 1, tm_time.tm_mday,
                 tm_time.tm_hour, tm_time.tm_min, tm_time.tm_sec);
        //strftime(buff, sizeof buff, "%D %T", std::gmtime(&time_.tv_sec))
    }
     */
    return buff;
}
