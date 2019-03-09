#include <assert.h>
#include <unistd.h>
#include <stdio.h>
#include "disastrOS.h"
#include "disastrOS_syscalls.h"
#include "disastrOS_semaphore.h"
#include "disastrOS_semdescriptor.h"
#include "disastrOS_globals.h"

void internal_semWait(){

  //Take the fd from the PCB of the running process and use it to get the sem_d
  int fd = running->syscall_args[0];
  SemDescriptor*  sem_d = SemDescriptorList_byFd(&running->sem_descriptors, fd);
  if(!sem_d) {
    running->syscall_retvalue = DSOS_ESEMWAIT_SEMD_NOT_IN_PROCESS;
    return;
  }

  //Decrease the count of sem and if its value is < 0 insert running in waiting list and change the running process
  Semaphore* sem = sem_d->semaphore;
  assert(sem);
  sem->count--;
  PCB* temp = running;
  if(sem->count < 0) {
    SemDescriptorPtr* sem_d_ptr = SemDescriptorPtr_alloc(sem_d);
    assert(sem_d_ptr);
    List_insert(&sem->waiting_descriptors, sem->waiting_descriptors.last, (ListItem*)sem_d_ptr);
    running->status = Waiting;
    List_insert(&waiting_list, waiting_list.last, (ListItem*)running);
    running = (PCB*)List_detach(&ready_list,ready_list.first);
  }

  //Set the return value of syscall(0 if successful)
  temp->syscall_retvalue = 0;
  return;

}
