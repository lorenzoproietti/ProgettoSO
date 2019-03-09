#include <assert.h>
#include <unistd.h>
#include <stdio.h>
#include "disastrOS.h"
#include "disastrOS_syscalls.h"
#include "disastrOS_semaphore.h"
#include "disastrOS_semdescriptor.h"
#include "disastrOS_globals.h"

void internal_semClose(){
  
  //Get the fd of the SemDescriptor that the running process wants to close
  int fd = running->syscall_args[0];
  
  //Get sem_d from the list of sem_descriptors opened by the running process
  SemDescriptor* sem_d =  SemDescriptorList_byFd(&running->sem_descriptors, fd);
  if(!sem_d) {
    running->syscall_retvalue = DSOS_ESEMCLOSE_SEMD_NOT_IN_PROCESS;
    return;
  }
  
  //Remove sem_d from the list of sem_descriptors of the running process
  List_detach(&running->sem_descriptors, (ListItem*)sem_d);

  //Get sem from sem_d
  Semaphore* sem = sem_d->semaphore;
  assert(sem);

  //Remove sem_d_ptr from the list of sem descriptors
  SemDescriptorPtr* sem_d_ptr = (SemDescriptorPtr*)List_detach(&sem->descriptors, (ListItem*)(sem_d->ptr));
  assert(sem_d_ptr);

  //Free the memory
  SemDescriptorPtr_free(sem_d_ptr);
  SemDescriptor_free(sem_d);

  //Check if there are no other processes on sem, if this is so, remove it from the global structure and free the memory
  if(sem->descriptors.size == 0) {
    sem = (Semaphore*)List_detach(&semaphores_list, (ListItem*)sem);
    assert(sem);
    Semaphore_free(sem);
  }

  //Set the return value of syscall(0 if successful)
  running->syscall_retvalue = 0;

}
