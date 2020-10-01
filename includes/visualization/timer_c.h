#pragma once
// Project includes
#include "chart_types.h"

// STL includes
#include <iostream>
#include <string>
class Timer_C {

public:
    Timer_C(const std::string& name);
    Timer_C(const Timer_C& other) = delete;
    Timer_C& operator=(const Timer_C& other) = delete;
    ~Timer_C();

public:
    void StopNow();
    
private:
    Timestamp_TP _start_time;
    std::string _name = "";
    double _elapsed_time = 0.0;
    bool _is_stopped = true;
};

inline
Timer_C::Timer_C(const std::string& name)  
    : 
    _name(name)
{
    _start_time.Now();
    _is_stopped = false;
}

// Prints nanoseconds
inline 
Timer_C::~Timer_C()
{
    if ( !_is_stopped ) {
        Timestamp_TP stop_time;
        stop_time.Now();
        _elapsed_time = stop_time.GetNanoseconds() - _start_time.GetNanoseconds();
        std::cout << "Timer: " << _name << "stopped after " << _elapsed_time << " ns" << std::endl;
    }
}

// prints milliseconds
inline 
void 
Timer_C::StopNow()
{
    Timestamp_TP stop_time;
    stop_time.Now();
    _elapsed_time = stop_time.GetMilliseconds() - _start_time.GetMilliseconds();
    std::cout << "Timer: " << _name << "stopped after " << _elapsed_time << " ms"<< std::endl;
    _is_stopped = true;
}
