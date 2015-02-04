#include <semaphore.h>
#include <thread>
#include <vector>

#define QUEUE_SIZE          100

void push_task( int task_id )
{
  printf( "\nAdding a new task to the task_queue: %d\n", task_id );
  fflush(stdout);

  sem_wait( q_space_sem );
  sem_wait( task_queue_sem );

  task_queue.push( task_id );

  sem_post( task_queue_sem );
  sem_post( task_avail_sem );
}

int pop_task()
{
  sem_wait( task_avail_sem );
  sem_wait( task_queue_sem );

  int task_id = task_queue.front( );
  task_queue.pop( );

  sem_post( task_queue_sem );
  sem_post( q_space_sem );

  return task_id;


}
