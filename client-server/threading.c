#include <semaphore.h>
#include <thread>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define MAX_THREADS         50

std::queue<int> task_queue;

/* will help us hold any extra info we need to pass to the threads */
struct thread_info
{
  int thread_id;
};

// container for our thread pool
pthread_t threads[MAX_THREADS];

// controls access to the task queue
static sem_t* task_queue_sem;
const char* task_queue_sem_name = "/TaskQueueInUse";

// controls thread usage
static sem_t* task_avail_sem;
const char* task_avail_sem_name = "/TaskAvailable";

// controls the size of the queue
static sem_t* q_space_sem;
const char* q_space_sem_name = "/SpaceInQueue";

// unlinks semaphores if used and then opens them again
void create_semaphores( ) {

  sem_unlink(task_queue_sem_name);
  task_queue_sem = sem_open( task_queue_sem_name, O_CREAT, 0, 1 );
  if (task_queue_sem == SEM_FAILED)
  {
    fprintf(stderr, "%s %s\n", "ERROR creating semaphore", task_queue_sem_name);
    exit(EXIT_FAILURE);
  }

  sem_unlink(task_avail_sem_name);
  task_avail_sem = sem_open( task_avail_sem_name, O_CREAT, 0, 0 );
  if (task_avail_sem == SEM_FAILED)
  {
    fprintf(stderr, "%s %s\n", "ERROR creating semaphore", task_avail_sem_name);
    exit(EXIT_FAILURE);
  }

  sem_unlink(q_space_sem_name);
  q_space_sem = sem_open( q_space_sem_name, O_CREAT, 0, 100 );
  if (q_space_sem == SEM_FAILED)
  {
    fprintf(stderr, "%s %s\n", "ERROR creating semaphore", q_space_sem_name);
    exit(EXIT_FAILURE);
  }

}

// populates the thread pool
void make_thread_pool(pthread_t* threads, int thread_num) {

  // container for their information
  struct thread_info all_thread_info[MAX_THREADS];

  for( int i = 0; i < thread_num; i++ )
  {
    printf("\ni: %d", i);
    fflush(stdout);

    all_thread_info[ i ].thread_id = i;

    pthread_create( &threads[ i ], NULL, serve, ( void* ) &all_thread_info[ i ] );

  }
}

void kill_all_threads( int thread_num ) {

  int sig = SIGKILL;

  for( int tid = 0; tid < thread_num; tid++)
  {
    printf("pthread_kill returned: %d", pthread_kill(threads[tid], sig));
    fflush(stdout);
  }
}
