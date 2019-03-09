#include <assert.h>
#include <unistd.h>
#include <stdio.h>
#include "disastrOS.h"
#include "disastrOS_syscalls.h"
#include "disastrOS_semaphore.h"
#include "disastrOS_semdescriptor.h"
#include "disastrOS_globals.h"

void internal_semOpen(){

  //Check to see if the running process has too much semdescriptors on its list
  if(running->sem_descriptors.size >=  MAX_NUM_SEMDESCRIPTORS_PER_PROCESS) {
    running->syscall_retvalue = DSOS_ESEMOPEN_OUT_OF_BOUND_SEMDESCRIPTORS;
    return;
  }


  //It takes the syscall parameter from the pcb of the running process(caller) and checks if the value is admissible
  int id = running->syscall_args[0];
  if(id < 0) {
    running->syscall_retvalue = DSOS_ESEMOPEN_SEMNUM_VALUE;
    return;
  }

  //Check if the sem with that id is already open, if it is not so we allocate it and add it to the semaphores_list(global structure), otherwise we take the sem with the function SemaphoreList_byId
  Semaphore* sem = SemaphoreList_byId((SemaphoreList*)&semaphores_list, id);
  if(!sem) {
    sem = Semaphore_alloc(id, 1);
    if(!sem) {
      running->syscall_retvalue = DSOS_ESEMOPEN_SEM_ALLOC;
      return;
    }
    List_insert(&semaphores_list, semaphores_list.last, (ListItem*)sem);
  }

  //Alloc the SemDescriptor for sem associated with the running process
  SemDescriptor* sem_d = SemDescriptor_alloc(running->last_sem_fd, sem, running);
  if(!sem_d) {
    running->syscall_retvalue = DSOS_ESEMOPEN_SEMDESCRIPTOR_ALLOC;
    return;
  }


  //Update last_sem_fd for the next SemDescriptor for the running process
  running->last_sem_fd++;

  //Alloc SemDescriptorPtr for sem_d
  SemDescriptorPtr* sem_d_ptr = SemDescriptorPtr_alloc(sem_d);
  if(!sem_d_ptr) {
    running->syscall_retvalue = DSOS_ESEMOPEN_SEMDESCRIPTORPTR_ALLOC;
    return;
  }

  //Link sem_d_ptr with sem_d
  sem_d->ptr = sem_d_ptr;

  //Add sem_d to the sem_descriptors list of the running process
  List_insert(&running->sem_descriptors, running->sem_descriptors.last, (ListItem*)sem_d);
  
  //Add sem_d_ptr to the descriptors list of sem
  List_insert(&sem->descriptors, sem->descriptors.last, (ListItem*)sem_d_ptr);
  
  //Set the return value of the syscall(descriptor of sem)
  running->syscall_retvalue = sem_d->fd;
  
  return;

}
