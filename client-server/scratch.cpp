#include <queue>

sem_t we_wait;
sem_t work_to_do;
sem_t space_on_q;

std::queue<int> work_tasks;

struct thread_info //made so that we can pass in more stuff to that serve function
{
  int thread_id;
}


pthread_t threads[THREADS];

struct thread_info all_thread_info[THREADS];

int main(int argc, char[] argv)
{
  //do same for all
  sem_init(&we_wait, 0, 1);
  sem_init(&work_to_do, 0, 0);
  sem_init(&space_on_q, 0, 100);

  for(THREADS) {

    sem_wait(&work_mutex);

    all_thread_info[i].thread_id = i;

    pthread_create(&threads[i], NULL, serve, (void*) &all_thread_info[i]);

    sem_post(&work_mutex);
  }


  // create socket sockfd

  // bind socket sockfd

  // tell socket to listen


  int counter = 0; //this should be a filedescriptor from the accept
  while(1)
  {
    // spin your wheels
    // web server will add requests to work_tasks here

    // accept connections on sockfd and returns a new_sock_fd

    sem_wait(&space_on_q);
    sem_wait(&work_mutex);

    work_tasks.push( counter );

    sem_post( &work_mutex );
    sem_post( &work_to_do );

    counter++;
  }

}

void* serve(void* in_data)
{

  struct thread_info = ( struct thread_info*) in_data;
  int tid = t_info->thread_id;

  while( 1 )
  {
    sem_wait(&work_to_do)
    sem_wait(&work_mutex)

    // get the
    int work_thing_i_grabbed = work_taks.front();
    work_tasks.pop();

    sem_wait(&work_mutex)
    sem_wait(&space_on_q)


    // read from file descriptor
    // parse the headers
    // find resource
    //  check if folder or file
    //  get file contents
    //  get the lenghts
    //  get file extensions for http response
    //
    // format the response
    // write response to requester
    // close()


  }
}

"-std=c++11"
