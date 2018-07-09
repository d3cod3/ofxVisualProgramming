#pragma once


#include "ofMain.h"
#include <atomic>


class ThreadedCommand: public ofThread{
    
public:
    ~ThreadedCommand(){
        stop();
        waitForThread(false);
    }

    void setup(){
        executed = true;
        startThread();
    }

    void execCommand(string cmd){
        command = cmd;
        executed = false;
    }

    void stop(){
        std::unique_lock<std::mutex> lck(mutex);
        stopThread();
        condition.notify_all();
    }

    void threadedFunction(){
        while(isThreadRunning()){
            if(!executed){
                executed = true;
                sys_status = system(command.c_str());
            }
        }
    }

    int getSysStatus() { return sys_status; }

protected:
    std::condition_variable condition;
    string                  command;
    int                     sys_status;
    bool                    executed;
};
