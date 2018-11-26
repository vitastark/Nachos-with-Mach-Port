/**************************************************************
 *
 * userprog/ksyscall.h
 *
 * Kernel interface for systemcalls 
 *
 * by Marcus Voelp  (c) Universitaet Karlsruhe
 *
 **************************************************************/

#ifndef __USERPROG_KSYSCALL_H__ 
#define __USERPROG_KSYSCALL_H__

#include "kernel.h"
#include "list.h"

void SysHalt()
{
  kernel->interrupt->Halt();
}


int SysAdd(int op1, int op2)
{
  return op1 + op2;
}

int SysFork_POS (VoidFunctionPtr myFunction)
{
  cout<<"system call Fork_POS"<<endl;
  Thread *child = new Thread("child");
  child->Fork(myFunction, (void *) child->getId());
  child->setParent(kernel->currentThread);
  kernel->currentThread->getchild()->Append(child);
  return child->getId();
}

void SysWait_POS (int threadId)
{
  cout<<"system call Wait_POS"<<endl;
  List<Thread *> *childlist = kernel->currentThread->getchild();
  ListIterator<Thread *> *it = new ListIterator<Thread *>(childlist);
  for (; !it->IsDone(); it->Next()) {
    if(it->Item()->getId()==threadId){
      cout<<kernel->currentThread->getId()<<" will sleep"<<endl;
      IntStatus oldLevel = kernel->interrupt->SetLevel(IntOff);
      kernel->currentThread->Sleep(false);
      (void) kernel->interrupt->SetLevel(oldLevel);
      return;
    }
  }
  List<int> *finish = kernel->currentThread->getfinish();
  ListIterator<int> *it2 = new ListIterator<int>(finish);
  for (; !it2->IsDone(); it2->Next()) {
    if(it2->Item()==threadId) {cout<<"this child thread is finished!"<<endl; return;}
  }
  cout<<"invalid child thread id"<<endl;
}

void SysExit(int argument)
{
   if(argument == 0){
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

int SysWrite(int buffer,int size){
    int num = 0;
    for(int i=0;i<size;i++){
       int current;
       kernel->machine->ReadMem(buffer+i,1,&current);
       printf("%c",current);}
    printf("\n");
    return num;
}



#endif /* ! __USERPROG_KSYSCALL_H__ */
