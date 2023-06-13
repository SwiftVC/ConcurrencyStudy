/* Parallel std::accumulate workthrough, improvements, and performance test

/*
Lessons:
Don't pass references to vector index objects or any dataobjects whose locations may change before a thread accesses the data, this caused
inconsistent crashes and gibberish answers.
Further to this, a vectors size must be predefined if references to its elements are passed as arguments.

Printing to std::cout with multiple threads may combine the text shown, getting terminal output from threads must done with care.
It's better to debug multiple threads by switching between and freezing them (via Visual Studio 2022, Debug/Windows/Threads, rmb->freeze/switch to;
leaving breakpoints within the passed multithreaded functions and [Continue] is particularly useful).

The size of the <array> is a part of the type, iterators must include this.

For a thread to edit a variable, enclose the argument in std::ref(), the function signature's parameter will have a '&' to match.

Reminder: syntax messages when using templates are arse, draft the problem without templates.
*/

/*
Improvements:
Solution was adapted to C++ Concurrency in Action 2019 pg.32 to use a minimum elements per thread as it is expected to be more efficient
at lower elements-to-threads to process more elements with fewer threads.
*/

#include <iostream>
#include <array>
#include <thread>
#include <vector>
#include "Timer.h"

template<typename iteratorToT, typename T>
T csAccumulate(iteratorToT first, iteratorToT last, T initial){
    for (; first != last; first++) {
        initial += *first;
    }
    return initial;
}

template<typename iteratorToT, typename T>
void accumulateSection(iteratorToT first, iteratorToT last, T& sectionSum) {
    sectionSum = csAccumulate(first, last, sectionSum);
}

template<typename iteratorToT, typename T>
T csThreadedAccumulate(iteratorToT first, iteratorToT last, T initial) {

    int maxThreads = std::thread::hardware_concurrency();
    if (maxThreads == 0) { maxThreads = 2; }

    int totalElements = int(std::distance(first, last));

    int minElementsPerThread = 10;
    int threadCount{ 0 };
    if(totalElements < maxThreads){threadCount = 1;}
    else {threadCount = std::min(totalElements / minElementsPerThread, maxThreads);}
    
    
    // fire off subsection sums
    std::vector<std::thread> threads;
    std::vector<T> sectionSums{ 0 };
    sectionSums.resize(threadCount, 0); // Important that vector be predefined size before threads are spawned.
    int index{ 0 };
    iteratorToT blockStart{ first };
    iteratorToT blockEnd{ first };

    int blockSize = totalElements / threadCount;
    for (int i = 0; i < threadCount; i++) {     
        std::advance(blockEnd, blockSize);
        threads.push_back(std::thread(accumulateSection<iteratorToT, T>, blockStart, blockEnd, std::ref(sectionSums[i])));
        std::advance(blockStart, blockSize);
    }

    for (std::thread& t : threads) { t.join(); }

    return csAccumulate<std::vector<int>::iterator, T>(sectionSums.begin(), sectionSums.end(), T());
}

int main(){
    std::array<int, 100> arr;
    for (int i = 0; i < arr.size(); i++) {
        arr[i] = i + 1;
    }

    auto timer1 = Timer();
    int ans1 = csAccumulate(arr.begin(), arr.end(), 0);
    timer1.stop();
    auto time1 = timer1.getTime();

    auto timer2 = Timer();
    int ans2 = csThreadedAccumulate(arr.begin(), arr.end(), 0);
    timer2.stop();
    auto time2 = timer2.getTime();

    const int FIELDWIDTH{ 10 };
    std::cout << "Answer is 5050" << std::endl;
    std::cout << "Standard approach:\t" << std::setw(FIELDWIDTH) << time1 << "\t" << "Answer:" << std::setw(FIELDWIDTH) << ans1 << std::endl;
    std::cout << "Multithreaded approach:\t" << std::setw(FIELDWIDTH) << time2 << "\t" << "Answer:" << std::setw(FIELDWIDTH) << ans2 << std::endl;

    if(time1 == time2){ std::cout << "Equal time for each." << std::endl; }
    else if (time1 < time2) { std::cout << "Standard approach is faster." << std::endl; }
    else { std::cout << "Multithreaded approach is faster." << std::endl; }

    return 0;
}

