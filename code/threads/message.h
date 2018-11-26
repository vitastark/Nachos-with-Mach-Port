//------------------------------------------------------
// This is the Port class definition for Assignment3
// Author: Haiyang Sun
// Date: 2018/11/25
//------------------------------------------------------
#include "copyright.h"
#include "thread.h"
#include <string>

class Message{
    private:
    string msg_body;
    int toPort;

    public:
    Message(){};
    ~Message(){};
    string getMsgBody(){
        return msg_body;
    }
    void setMsgBody(string body){
        msg_body = body;
    }
    int getPort(){
        return toPort;
    }
    void setPort(int portNumber){
        toPort = portNumber;
    }
};