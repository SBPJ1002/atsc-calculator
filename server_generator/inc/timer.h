#ifndef INC_TIMER_H_
#define INC_TIMER_H_

#include <ctime>
#include <iostream>

#define YES 1
#define NO 0

#define COUNTING YES
#define NOT_COUNTING NO

class cl_timer {
private:
    struct timespec startTime, stopTime;
public:
    cl_timer();
    ~cl_timer();
    void reset();
    void start();
    void _continue();
    void stop();
    void update();
    void print();
    int get_elapsed_time_ms();
    int get_counter_status();
    int seconds;
    int miliSeconds;
    int microseconds;
    long int nanoseconds;
    int counting;
};

#endif
