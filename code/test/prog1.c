//----------------------------------------
// test prog1.c for assignment3
// Author: Haiyang Sun
// Date: 2018/11/25
//----------------------------------------
#include "syscall.h"

int
main()
{
    
	char* message = "Hello from tprog1";
	Register(5);
	Send(message, 17, 6);
        
	Exit(0);
}
