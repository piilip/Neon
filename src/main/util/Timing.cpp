#include "Timing.h"

Timer::~Timer() {
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = end - start;
    timeKeeper.add(name, duration);
}

void TimeKeeper::add(const std::string &name, std::chrono::nanoseconds duration) {
    // TODO what should we do if there are multiple durations for the same name?
    times[name] = duration;
}

std::chrono::nanoseconds TimeKeeper::get(const std::string &name) const { return times.at(name); }
