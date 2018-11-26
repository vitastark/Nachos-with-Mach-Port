// exception.cc 
//	Entry point into the Nachos kernel from user programs.
//	There are two kinds of things that can cause control to
//	transfer back to here from user code:
//
//	syscall -- The user code explicitly requests to call a procedure
//	in the Nachos kernel.  Right now, the only function we support is
//	"Halt".
//
//	exceptions -- The user code does something that the CPU can't handle.
//	For instance, accessing memory that doesn't exist, arithmetic errors,
//	etc.  
//
//	Interrupts (which can also cause control to transfer from user
//	code into the Nachos kernel) are handled elsewhere.
//
// For now, this only handles the Halt() system call.
// Everything else core dumps.
//
// Copyright (c) 1992-1996 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "main.h"
#include "syscall.h"
#include "ksyscall.h"
//----------------------------------------------------------------------
// ExceptionHandler
// 	Entry point into the Nachos kernel.  Called when a user program
//	is executing, and either does a syscall, or generates an addressing
//	or arithmetic exception.
//
// 	For system calls, the following is the calling convention:
//
// 	system call code -- r2
//		arg1 -- r4
//		arg2 -- r5
//		arg3 -- r6
//		arg4 -- r7
//
//	The result of the system call, if any, must be put back into r2.
//
// If you are handling a system call, don't forget to increment the pc
// before returning. (Or else you'll loop making the same system call forever!)
//
//	"which" is the kind of exception.  The list of possible exceptions
//	is in machine.h.
//----------------------------------------------------------------------
void Exit_POS(ThreadId id){
    if(kernel->currentThread->getId()!=id){
        cout<<"thread does not have permission"<<endl;
    }else{
        cout<<"Exit_pos"<<endl;
        Thread *p = kernel->currentThread->getParent();
        if(p != NULL) {
            IntStatus oldLevel = kernel->interrupt->SetLevel(IntOff);
	    if(!kernel->scheduler->getReadyList()->IsInList(p))
	        {	
            		kernel->scheduler->ReadyToRun(p);
        	}
            (void) kernel->interrupt->SetLevel(oldLevel);
                
            p->getchild()->Remove(kernel->currentThread);
            p->getfinish()->Append(kernel->currentThread->getId());
	}

        List<Thread *> *childlist = kernel->currentThread->getchild();
        ListIterator<Thread *> *it = new ListIterator<Thread *>(childlist);
        for (; !it->IsDone(); it->Next()) {
          it->Item()->setParent(NULL);}
        
	
        kernel->currentThread->Finish();
	}
}

void ForkTest1(int id)
{
    printf("ForkTest1 is called, its PID is %d\n", id);
    for (int i = 0; i < 3; i++)
    {
        printf("ForkTest1 is in loop %d\n", i);
        for (int j = 0; j < 100; j++)
            kernel->interrupt->OneTick();
    }
    Exit_POS(id);
}

void ForkTest2(int id)
{
    printf("ForkTest2 is called, its PID is %d\n", id);
    for (int i = 0; i < 3; i++)
    {
        printf("ForkTest2 is in loop %d\n", i);
        for (int j = 0; j < 100; j++)
           kernel->interrupt->OneTick();
    }
    Exit_POS(id);
}

void ForkTest3(int id)
{
    printf("ForkTest3 is called, its PID is %d\n", id);
    for (int i = 0; i < 3; i++)
    {
        printf("ForkTest3 is in loop %d\n", i);
        for (int j = 0; j < 100; j++)
            kernel->interrupt->OneTick();
    }
    Exit_POS(id);
}

void
ExceptionHandler(ExceptionType which)
{
    int type = kernel->machine->ReadRegister(2);

    DEBUG(dbgSys, "Received Exception " << which << " type: " << type << "\n");

    switch (which) {
        case PageFaultException:{
                    cout<<"page fault handler"<<endl;
                    // calculate the virtual page number which cause page fault
		    int badVAddr = (int) kernel->machine->ReadRegister(39);
                    int vpn = (int) badVAddr / PageSize;
                    // find a new physical page number
                    int newPhyPage = kernel->FreeMap->FindAndSet();
                    // fetch the page entry of the current Nachos Process
                    TranslationEntry *pageTable = kernel->currentThread->space->getPageTable();
                    TranslationEntry *pageEntry = &pageTable[vpn];
		    TranslationEntry *oldPageEntry = NULL;
                    if(newPhyPage!=-1){
                        //update the page entry
                        pageEntry->physicalPage = newPhyPage;
                        cout<<"the new physical page is "<<newPhyPage<<endl;
                        pageEntry->valid = TRUE;
			// read from swap space and put them into the main memory
                        kernel->swap_space->ReadAt(&(kernel->machine->mainMemory[newPhyPage * PageSize]),PageSize,pageEntry->indexOfSwapSpace * PageSize);
			// append the current pageEntry
                        kernel->FIFO->Append(pageEntry);
                    }

                    else{
			// fetch the first page entry of the FIFO list
                        oldPageEntry = kernel->FIFO->RemoveFront();
                        // update the page into swap file
                        kernel->swap_space->WriteAt(&(kernel->machine->mainMemory[oldPageEntry->physicalPage*PageSize]),PageSize,oldPageEntry->indexOfSwapSpace*PageSize);
                        int physicalpage = oldPageEntry->physicalPage;
                        cout<<"swap the physical page in page #"<<physicalpage<<endl;
                        oldPageEntry->physicalPage = -1;
                        oldPageEntry->valid = FALSE;
			// set the new page 
                        pageEntry->physicalPage = physicalpage;
                        pageEntry->valid = TRUE;
			//read from swap space into main memory
                        kernel->swap_space->ReadAt(&(kernel->machine->mainMemory[physicalpage * PageSize]),PageSize,pageEntry->indexOfSwapSpace * PageSize);
                        kernel->FIFO->Append(pageEntry);
                    }
                    return;

                    ASSERTNOTREACHED();

                }break;

        case SyscallException:
            switch(type) {
                case SC_Halt:
                    DEBUG(dbgSys, "Shutdown, initiated by user program.\n");

                    SysHalt();

                    ASSERTNOTREACHED();
                    break;

                case SC_ForkPOS:
                {
                    DEBUG(dbgSys, "Thread_Fork " << kernel->machine->ReadRegister(4) << "\n");
                    int argument = (int) kernel->machine->ReadRegister(4);
                    ThreadId id;
                    switch(argument){
                        case 1: {
                            id = SysFork_POS((VoidFunctionPtr)ForkTest1);
                        } break;
                        case 2: {
                            id = SysFork_POS((VoidFunctionPtr)ForkTest2);
                        } break;
                        case 3: {
                            id = SysFork_POS((VoidFunctionPtr)ForkTest3);
                        } break;
                        default: cout<<"invalid argument of threadFork"<<endl;
                    }
                    DEBUG(dbgSys, "Thread_Fork returning with " << id << "\n");
                    kernel->machine->WriteRegister(2, (int)id);
                    /* Modify return point */
                    {
                        /* set previous programm counter (debugging only)*/
                        kernel->machine->WriteRegister(PrevPCReg, kernel->machine->ReadRegister(PCReg));

                        /* set programm counter to next instruction (all Instructions are 4 byte wide)*/
                        kernel->machine->WriteRegister(PCReg, kernel->machine->ReadRegister(PCReg) + 4);

                        /* set next programm counter for brach execution */
                        kernel->machine->WriteRegister(NextPCReg, kernel->machine->ReadRegister(PCReg)+4);
                    }
                    return;

                    ASSERTNOTREACHED();
                }break;
                
                case SC_WaitPOS:
                {
                    DEBUG(dbgSys, "parent_wait " << kernel->machine->ReadRegister(4) << "\n");
                    int threadId = (int) kernel->machine->ReadRegister(4);
                    SysWait_POS(threadId);

                    /* Modify return point */
                    {
                        /* set previous programm counter (debugging only)*/
                        kernel->machine->WriteRegister(PrevPCReg, kernel->machine->ReadRegister(PCReg));

                        /* set programm counter to next instruction (all Instructions are 4 byte wide)*/
                        kernel->machine->WriteRegister(PCReg, kernel->machine->ReadRegister(PCReg) + 4);

                        /* set next programm counter for brach execution */
                        kernel->machine->WriteRegister(NextPCReg, kernel->machine->ReadRegister(PCReg)+4);
                    }
                    return;

                    ASSERTNOTREACHED();
                }break;

                case SC_Exit:
                {
                    DEBUG(dbgSys, "SC_Exit" << kernel->machine->ReadRegister(4) << "\n");
                    int argument = (int) kernel->machine->ReadRegister(4);
                    SysExit(argument);

                    /* Modify return point */
                    {
                        /* set previous programm counter (debugging only)*/
                        kernel->machine->WriteRegister(PrevPCReg, kernel->machine->ReadRegister(PCReg));

                        /* set programm counter to next instruction (all Instructions are 4 byte wide)*/
                        kernel->machine->WriteRegister(PCReg, kernel->machine->ReadRegister(PCReg) + 4);

                        /* set next programm counter for brach execution */
                        kernel->machine->WriteRegister(NextPCReg, kernel->machine->ReadRegister(PCReg)+4);
                    }
                    return;

                    ASSERTNOTREACHED();
                }break;

                case SC_Write:
                {
                    DEBUG(dbgSys, "SC_Write" << kernel->machine->ReadRegister(4) << "\n");
                    int buffer = (int) kernel->machine->ReadRegister(4);
                    int size = (int) kernel->machine->ReadRegister(5);
                    int result = SysWrite(buffer,size);
		    
		            DEBUG(dbgSys, "Add returning with " << result << "\n");
                    /* Prepare Result */
                    kernel->machine->WriteRegister(2, (int)result);

                    /* Modify return point */
                    {
                        /* set previous programm counter (debugging only)*/
                        kernel->machine->WriteRegister(PrevPCReg, kernel->machine->ReadRegister(PCReg));

                        /* set programm counter to next instruction (all Instructions are 4 byte wide)*/
                        kernel->machine->WriteRegister(PCReg, kernel->machine->ReadRegister(PCReg) + 4);

                        /* set next programm counter for brach execution */
                        kernel->machine->WriteRegister(NextPCReg, kernel->machine->ReadRegister(PCReg)+4);
                    }
                    return;

                    ASSERTNOTREACHED();
                }break;

                case SC_Register:
                {
                    DEBUG(dbgSys, "SC_Register" << kernel->machine->ReadRegister(4) << "\n");
                    int portNumber = (int) kernel->machine->ReadRegister(4);
                    if(kernel->mach_ports[portNumber].getReceiveThread() == NULL){
                        kernel->mach_ports[portNumber].setReceiveThread(kernel->currentThread);
                    }

                    /* Modify return point */
                    {
                        /* set previous programm counter (debugging only)*/
                        kernel->machine->WriteRegister(PrevPCReg, kernel->machine->ReadRegister(PCReg));

                        /* set programm counter to next instruction (all Instructions are 4 byte wide)*/
                        kernel->machine->WriteRegister(PCReg, kernel->machine->ReadRegister(PCReg) + 4);

                        /* set next programm counter for brach execution */
                        kernel->machine->WriteRegister(NextPCReg, kernel->machine->ReadRegister(PCReg)+4);
                    }
                    return;

                    ASSERTNOTREACHED();
                }break;

                case SC_Send:
                {
                    cout<<"now in SC_Send"<<endl;
                    DEBUG(dbgSys, "SC_Send" << kernel->machine->ReadRegister(4) << "\n");
                    int msg_buf = (int) kernel->machine->ReadRegister(4);
                    int size = (int) kernel->machine->ReadRegister(5);
                    int portNumber = (int) kernel->machine->ReadRegister(6);
                    string message = "";
                    for(int i = 0; i < size; i++){
                        int current;
                        kernel->machine->ReadMem(msg_buf+i, 1, &current);
                        message.push_back((char)current);
                    }
                    bool flag = false;
                    for(int i = 0; i < kernel->mach_ports[portNumber].getSendThreads().size(); i++){
                        if( kernel->mach_ports[portNumber].getSendThreads()[i] == kernel->currentThread){
                            flag = true;
                            break;
                        }
                    }
                    if(flag == false){
                        cout<<"portNumber is: "<<portNumber<<endl;
                        kernel->mach_ports[portNumber].addSendThread(kernel->currentThread);
                    }
                    Message msg;
                    msg.setMsgBody(message);
                    msg.setPort(portNumber);
                    kernel->msg_queue.push(msg);

                    /* Modify return point */
                    {
                        /* set previous programm counter (debugging only)*/
                        kernel->machine->WriteRegister(PrevPCReg, kernel->machine->ReadRegister(PCReg));

                        /* set programm counter to next instruction (all Instructions are 4 byte wide)*/
                        kernel->machine->WriteRegister(PCReg, kernel->machine->ReadRegister(PCReg) + 4);

                        /* set next programm counter for brach execution */
                        kernel->machine->WriteRegister(NextPCReg, kernel->machine->ReadRegister(PCReg)+4);
                    }
                    return;

                    ASSERTNOTREACHED();
                }break;


                case SC_Receive:
                {
                    cout<<"now in SC_Recieve"<<endl;
                    DEBUG(dbgSys, "SC_Receive" << kernel->machine->ReadRegister(4) << "\n");
                    int portNumber = (int) kernel->machine->ReadRegister(4);
                    if(kernel->mach_ports[portNumber].getReceiveThread() != kernel->currentThread){
                        cout<<"no receive permission"<<endl;
                        break;
                    }
                    if(kernel->mach_ports[portNumber].getMsg() == ""){
                        cout<<"this port has no massage inside"<<endl;
                        kernel->currentThread->Yield();
                    }
                    string message = kernel->mach_ports[portNumber].getMsg();
                    kernel->mach_ports[portNumber].setMsg("");
                    cout<<"message is: "<<message<<endl;
                    getchar();


                    /* Modify return point */
                    {
                        /* set previous programm counter (debugging only)*/
                        kernel->machine->WriteRegister(PrevPCReg, kernel->machine->ReadRegister(PCReg));

                        /* set programm counter to next instruction (all Instructions are 4 byte wide)*/
                        kernel->machine->WriteRegister(PCReg, kernel->machine->ReadRegister(PCReg) + 4);

                        /* set next programm counter for brach execution */
                        kernel->machine->WriteRegister(NextPCReg, kernel->machine->ReadRegister(PCReg)+4);
                    }
                    return;

                    ASSERTNOTREACHED();
                }break;


                case SC_Add:{
                    DEBUG(dbgSys, "Add " << kernel->machine->ReadRegister(4) << " + " << kernel->machine->ReadRegister(5) << "\n");

                    /* Process SysAdd Systemcall*/
                    int result;
                    result = SysAdd(/* int op1 */(int)kernel->machine->ReadRegister(4),
                            /* int op2 */(int)kernel->machine->ReadRegister(5));

                    DEBUG(dbgSys, "Add returning with " << result << "\n");
                    /* Prepare Result */
                    kernel->machine->WriteRegister(2, (int)result);

                    /* Modify return point */
                    {
                        /* set previous programm counter (debugging only)*/
                        kernel->machine->WriteRegister(PrevPCReg, kernel->machine->ReadRegister(PCReg));

                        /* set programm counter to next instruction (all Instructions are 4 byte wide)*/
                        kernel->machine->WriteRegister(PCReg, kernel->machine->ReadRegister(PCReg) + 4);

                        /* set next programm counter for brach execution */
                        kernel->machine->WriteRegister(NextPCReg, kernel->machine->ReadRegister(PCReg)+4);
                    }

                    return;

                    ASSERTNOTREACHED();
		}

                    break;

                default:
                    cerr << "Unexpected system call " << type << "\n";
                    break;
            }
            break;
        default:
            cerr << "Unexpected user mode exception" << (int)which << "\n";
            break;
    }
    ASSERTNOTREACHED();
}


