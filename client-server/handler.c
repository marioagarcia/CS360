#include <signal.h>

void sig_handler(int signo)
{
  if (signo == SIGINT )
  {
    printf("received SIGINT signal\n");
    fflush(stdout);

    sem_close(task_queue_sem);
    sem_close(task_avail_sem);
    sem_close(q_space_sem);

    kill_all_threads( thread_num );

    exit(1);
  }

  if (signo == SIGSEGV )
  {

    printf("received SIGSEGV signal\n");
    fflush(stdout);

    sem_close(task_queue_sem);
    sem_close(task_avail_sem);
    sem_close(q_space_sem);

    kill_all_threads( thread_num );

    exit(1);
  }
}

void read_request( char* request, char* request_type, char* file_path, char* HTTP_ver )
{
  if(!strlen(request))
  {
    printf("No request received");
    exit(1);
  }
  else
  {
    printf("\nfirst header: %s", request );
    fflush(stdout);
    sscanf( request, "%s %s %s", request_type, file_path, HTTP_ver );
  }
}
