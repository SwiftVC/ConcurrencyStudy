#include <chrono>

class Timer {
    std::chrono::high_resolution_clock::time_point begin;
    std::chrono::high_resolution_clock::time_point end;
public:
    Timer() {
        begin = std::chrono::high_resolution_clock::now();
    }
    void stop() { end = std::chrono::high_resolution_clock::now(); }
    std::chrono::nanoseconds getTime() {
        return std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin);
    }
};
