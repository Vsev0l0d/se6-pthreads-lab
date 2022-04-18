#include <pthread.h>
#include <unistd.h>
#include <atomic>
#include <iostream>

typedef struct {
  const int sleep_millisecond_limit;
  const bool is_debug;
  const int* number;
} consumerData;

typedef struct {
  const pthread_t* consumers;
  const int length;
} interruptorData;

pthread_cond_t cond_produced = PTHREAD_COND_INITIALIZER;
pthread_cond_t cond_processed = PTHREAD_COND_INITIALIZER;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
bool finish = false;
std::atomic_int count_waiting_consumers = 0;
std::atomic_int count_alive_consumers;
bool is_data_produced = false;

int get_tid() {
  thread_local static int tid = 0;
  static std::atomic_int count = 0;
  if (tid == 0) tid = ++count;
  return tid;
}

void* producer_routine(void* arg) {
  int* number = (int*)arg;

  while (scanf("%d", number) == 1) {
    while (count_waiting_consumers < 1) {
    };
    pthread_mutex_lock(&mutex);
    pthread_cond_signal(&cond_produced);
    is_data_produced = true;
    while (is_data_produced) pthread_cond_wait(&cond_processed, &mutex);
    pthread_mutex_unlock(&mutex);
  }

  *number = 0;
  finish = true;
  while (count_alive_consumers) pthread_cond_broadcast(&cond_produced);
  return nullptr;
}

void* consumer_routine(void* arg) {
  pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
  consumerData* data = (consumerData*)arg;
  int* psum = (int*)malloc(sizeof(int));
  *psum = 0;

  while (!finish) {
    pthread_mutex_lock(&mutex);
    count_waiting_consumers++;
    while (!is_data_produced && !finish)
      pthread_cond_wait(&cond_produced, &mutex);
    is_data_produced = false;
    count_waiting_consumers--;
    *psum += *(data->number);
    pthread_cond_signal(&cond_processed);
    pthread_mutex_unlock(&mutex);

    if (finish) break;
    if (data->is_debug) printf("(%d, %d)\n", get_tid(), *psum);
    srand(time(0));
    usleep(1000 * (rand() % (data->sleep_millisecond_limit + 1)));
  }

  count_alive_consumers--;
  return psum;
}

void* consumer_interruptor_routine(void* arg) {
  interruptorData* data = (interruptorData*)arg;
  while (!finish) {
    srand(time(0));
    pthread_cancel(data->consumers[rand() % data->length]);
  }
  return nullptr;
}

int run_threads(int N, int sleep_millisecond_limit, bool is_debug = false) {
  int aggregated_sum = 0, number = 0;
  count_alive_consumers = N;

  pthread_t* consumers = (pthread_t*)malloc(N * sizeof(pthread_t));
  consumerData consumersData = {sleep_millisecond_limit, is_debug, &number};
  for (int i = 0; i < N; i++) {
    pthread_create(&(consumers[i]), NULL, consumer_routine, &consumersData);
  }

  pthread_t producer;
  pthread_create(&producer, NULL, producer_routine, &number);

  pthread_t interruptor;
  interruptorData interruptorData = {consumers, N};
  pthread_create(&interruptor, NULL, consumer_interruptor_routine,
                 &interruptorData);

  pthread_join(producer, NULL);
  pthread_join(interruptor, NULL);
  for (int i = 0; i < N; i++) {
    int* psum;
    pthread_join(consumers[i], (void**)&psum);
    aggregated_sum += *psum;
    free(psum);
  }

  free(consumers);
  pthread_cond_destroy(&cond_processed);
  pthread_cond_destroy(&cond_produced);
  pthread_mutex_destroy(&mutex);
  return aggregated_sum;
}