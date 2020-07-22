#include <iostream>
#include <random>
#include <thread>
#include <chrono>
#include <future>
#include "TrafficLight.h"

/* Implementation of class "MessageQueue" */
template <typename T>
T MessageQueue<T>::receive() {
    // FP.5a : The method receive should use std::unique_lock<std::mutex> and _condition.wait() 
    // to wait for and receive new messages and pull them from the queue using move semantics. 
    // The received object should then be returned by the receive function. 
    std::unique_lock<std::mutex> unqLock(_mutex);
    //working under the lock
    _condition.wait(unqLock, [this]{ return  !_queue.empty();});

    T message = std::move(_queue.front());
    _queue.pop_front();
    return message;
}

template <typename T>
void MessageQueue<T>::send(T &&msg) {
    // FP.4a : The method send should use the mechanisms std::lock_guard<std::mutex> 
    // as well as _condition.notify_one() to add a new message to the queue and afterwards send a notification.
    std::lock_guard<std::mutex> unqLock(_mutex);
    //working under the lock
    _queue.emplace_back(msg);
    _condition.notify_one();
}


/* Implementation of class "TrafficLight" */


TrafficLight::TrafficLight() {
    _trafficPhaseNow = TrafficLightPhase::red;
}

void TrafficLight::waitForGreen() {
    // FP.5b : add the implementation of the method waitForGreen, in which an infinite while-loop 
    // runs and repeatedly calls the receive function on the message queue. 
    // Once it receives TrafficLightPhase::green, the method returns.
    while (true) {
         std::this_thread::sleep_for(std::chrono::milliseconds(1)); //do not being soooo fast
        if(_messageQueue.receive() == TrafficLightPhase::green){
            return;
        }
    }
}

TrafficLightPhase TrafficLight::getCurrentPhase() {
    return _trafficPhaseNow;
}

void TrafficLight::simulate() {
    // FP.2b : Finally, the private method „cycleThroughPhases“ should be started in a thread when the public method „simulate“ is called. To do this, use the thread queue in the base class. 
    threads.emplace_back(std::thread(&TrafficLight::cycleThroughPhases, this));
}

// virtual function which is executed in a thread
void TrafficLight::cycleThroughPhases()
{
    // FP.2a : Implement the function with an infinite loop that measures the time between two loop cycles 
    // and toggles the current phase of the traffic light between red and green and sends an update method 
    // to the message queue using move semantics. The cycle duration should be a random value between 4 and 6 seconds. 
    // Also, the while-loop should use std::this_thread::sleep_for to wait 1ms between two cycles. 
    std::random_device pseudoRandomDevice;
    std::mt19937 pseudoRandomGenerator(pseudoRandomDevice());
    std::uniform_int_distribution<int> distribution(4000, 6000);
    auto entryTime = std::chrono::system_clock::now();
    int lightDuration = distribution(pseudoRandomGenerator);
    while (true) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1)); //reducing CPU bruteforce
        long timeFromEntryTime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - entryTime).count();
        if(timeFromEntryTime >= lightDuration){
            _trafficPhaseNow = _trafficPhaseNow == TrafficLightPhase::red ? TrafficLightPhase::green : TrafficLightPhase::red;
            _messageQueue.send(std::move(_trafficPhaseNow));
            entryTime = std::chrono::system_clock::now();
            lightDuration = distribution(pseudoRandomGenerator);
        }
    }
}

