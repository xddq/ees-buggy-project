//
// Created by leonhard on 19.06.20.
//

#ifndef EES_BUGGY_TIME_UTIL_H
#define EES_BUGGY_TIME_UTIL_H

#include <chrono>

//std::this_thread::sleep_for may sleep longer than passed time.
//To be more accurate in exchange of battery we busy_wait
template<typename chrono_time>
void busy_wait(chrono_time delay){
    auto start = std::chrono::steady_clock::now();
    auto now = std::chrono::steady_clock::now();
    while(now - start < delay){
        now = std::chrono::steady_clock::now();
    }
}

/**
 * busy waits until the given predicate p is true or timeout happend
 * @tparam Rep
 * @tparam Period
 * @tparam predicate
 * @tparam chrono_time
 * @param p
 * @param timeout
 * @return duration representing how long the function ran
 */
template<typename predicate, typename chrono_time = std::chrono::hours>
auto busy_wait_until(predicate p, chrono_time timeout = std::chrono::hours{100'000'000}){
    auto start = std::chrono::steady_clock::now();
    auto now = std::chrono::steady_clock::now();
    while (now - start < timeout && !p()) {
        now = std::chrono::steady_clock::now();
    }
    return start - now;
}

#endif //EES_BUGGY_TIME_UTIL_H
