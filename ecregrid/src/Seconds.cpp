#ifndef machine_H
#include "machine.h"
#endif


#ifndef Seconds_H
#include "Seconds.h"
#endif

#include <sys/time.h>

Seconds::Seconds(double seconds):
    seconds_(seconds) {
}

Seconds::Seconds(const timeval& time):
    seconds_(time.tv_sec + time.tv_usec / 1000000.0) {
}

static struct  {
    int length_;
    const char *name_;
} periods[] = {
    {7 * 24 * 60 * 60, "week",},
    {24 * 60 * 60, "day",},
    {60 * 60, "hour",},
    {60, "minute",},
    {1, "second",},
    {0,0,},
};

ostream& operator<<(ostream& s,const Seconds&  sec) {
    double t = sec.seconds_;
    long n  = t;
    int flg = 0;

    for(int i=0; periods[i].length_; i++) {
        long m = n / periods[i].length_;
        if(m) {
            if(flg) s << ' ';
            s << m << ' ' << periods[i].name_;
            if(m>1) s << 's';
            n %= periods[i].length_;
            flg++;
        }
    }

    if(!flg) s << t << " second";

    return s;
}

#if 0
Seconds::operator string() const {
    StrStream s;
    s << *this << StrStream::ends;
    return string(s);
}
#endif
