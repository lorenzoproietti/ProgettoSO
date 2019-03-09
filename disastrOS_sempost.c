#include <assert.h>
#include <unistd.h>
#include <stdio.h>
#include "disastrOS.h"
#include "disastrOS_syscalls.h"
#include "disastrOS_semaphore.h"
#include "disastrOS_semdescriptor.h"
#include "disastrOS_globals.h"

void internal_semPost(){

  //Take the fd from the PCB of the running process and use it to get the sem_d
  int fd = running->syscall_args[0];
  SemDescriptor* sem_d = SemDescriptorList_byFd(&running->sem_descriptors, fd);
  if(!sem_d) {
    running->syscall_retvalue = DSOS_ESEMPOST_SEMD_NOT_IN_PROCESS;
    return;
  }

  //Increase the count of sem and if its value is <= 0, it means that a process in the sem waiting list can go to the Ready state
  Semaphore* sem = sem_d->semaphore;
  assert(sem);
  sem->count++;
  if(sem->count <= 0) {
    SemDescriptorPtr* sem_d_ptr = (SemDescriptorPtr*)List_detach(&sem->waiting_descriptors, sem->waiting_descriptors.first);
    PCB* pcb = sem_d_ptr->descriptor->pcb;
    SemDescriptorPtr_free(sem_d_ptr);
    pcb->status = Ready;
    List_detach(&waiting_list, (ListItem*)pcb);
    List_insert(&ready_list, ready_list.last, (ListItem*)pcb);
  }

  //Set the return value of syscall(0 if successful)
  running->syscall_retvalue = 0;

}
