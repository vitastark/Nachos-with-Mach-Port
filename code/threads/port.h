//------------------------------------------------------
// This is the Port class definition for Assignment3
// Author: Haiyang Sun
// Date: 2018/11/25
//------------------------------------------------------
#include "copyright.h"

#include "thread.h"
#include <string>
#include <vector>

class Port{

    private:
        string message;
        vector<Thread*> sendThreads;
        Thread* receiveThread;

    public:
        // constructor and destructor
        Port(){};
        ~Port(){};

        // get and set functions for private member
        void setMsg(string msg){
            message = msg;
        }
        string getMsg(){
            return message;
        }
        vector<Thread*> getSendThreads(){
            return sendThreads;
        }
        void addSendThread(Thread* t){
            sendThreads.push_back(t);
        }
        Thread* getReceiveThread(){
            return receiveThread;
        }
        void setReceiveThread(Thread* t){
            receiveThread = t;
        }
};