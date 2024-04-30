#include <pthread.h>
#include <atomic>
#include <iostream>
#include <thread>
#include <vector>

#define ERROR_CREATE_THREAD -11
#define ERROR_JOIN_THREAD -12

using namespace std;

// Мьютексы и сигналы
pthread_mutex_t shared_variable_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t shared_variable_changed = PTHREAD_COND_INITIALIZER;

// Глобальные переменные
bool flag_finish = false;    // Флаг завершения
int* shared_variable_buffer; // Разделяемая переменная
int shared_variable_count = 0;  // Текущая длина разделяемой переменной

// Данные, передаваемые в producer
struct producer_data {
  int* numbers;
  size_t numbers_size;
};

// Данные передаваемые в consumer
struct consumer_data {
  bool debug_flag;
  int max_sleep_time;
};

// Данные передаваемые в consumer_interruptor
struct consumer_interruptor_data {
  pthread_t* consumer_threads;
  int n;
};

// Получить tid для потока
int get_tid() {
  static atomic<int> tid_counter{0};
  thread_local static int* tid;
  if (tid == 0) {
    tid_counter++;
    tid = new int(tid_counter);
  }
  return *tid;
}

// Поток producer
void* producer_routine(void* arg) {
  (void)arg;

  // cout << "producer " << endl;

  // Получаем исходные данные
  struct producer_data* producer_data = (struct producer_data*)arg;
  int* numbers = producer_data->numbers;
  size_t numbers_size = producer_data->numbers_size;

  // Записываем данные в разделяемую переменную
  for (size_t i = 0; i < numbers_size; i++) {
    pthread_mutex_lock(&shared_variable_mutex);
    shared_variable_buffer[shared_variable_count] = numbers[i];
    shared_variable_count++;
    pthread_cond_signal(&shared_variable_changed);
    pthread_mutex_unlock(&shared_variable_mutex);
  }

  flag_finish = true;
  pthread_mutex_lock(&shared_variable_mutex);
  pthread_cond_broadcast(&shared_variable_changed);
  pthread_mutex_unlock(&shared_variable_mutex);
  return nullptr;
}

// Поток consumer
void* consumer_routine(void* arg) {
  (void)arg;
  int* thread_sum = new int(0);

  // cout << "consumer " << get_tid() << endl;

  pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, nullptr);

  // Получаем исходные данные
  struct consumer_data* consumer_data = (struct consumer_data*)arg;
  bool debug_flag = consumer_data->debug_flag;
  int max_sleep_time = consumer_data->max_sleep_time;

  // Читаем данные из разделяемой переменной
  while (flag_finish == false || shared_variable_count != 0) {
    pthread_mutex_lock(&shared_variable_mutex);
    while (flag_finish == false && shared_variable_count == 0) {
      pthread_cond_wait(&shared_variable_changed, &shared_variable_mutex);
    }
    if (shared_variable_count == 0) {
      pthread_mutex_unlock(&shared_variable_mutex);
      break;
    }
    int tmp = shared_variable_buffer[shared_variable_count - 1];
    shared_variable_count--;
    *thread_sum += tmp;
    if (debug_flag) cout << get_tid() << ", " << *thread_sum << endl;
    pthread_mutex_unlock(&shared_variable_mutex);

    long sleep_time =
        (max_sleep_time == 0) ? 0 : (rand() % (max_sleep_time + 1));
    this_thread::sleep_for(chrono::milliseconds(sleep_time));
  }

  pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, nullptr);
  return thread_sum;
}

void* consumer_interruptor_routine(void* arg) {
  (void)arg;

  // cout << "consumer_interruptor" << endl;

  // Получаем исходные данные
  struct consumer_interruptor_data* consumer_interruptor_data =
      (struct consumer_interruptor_data*)arg;
  pthread_t* consumer_threads = consumer_interruptor_data->consumer_threads;
  int n = consumer_interruptor_data->n;

  int random_consumer;
  while (flag_finish == false) {
    random_consumer = rand() % n;
    pthread_cancel(consumer_threads[random_consumer]);
  }

  return nullptr;
}

// Запуск потоков
int run_threads(int n, int max_sleep_time, vector<int> numbers,
                bool debug_flag) {

  // Инициализируем разделяемую переменную
  shared_variable_buffer = new int[numbers.size()];

  // Запускаем поток producer
  struct producer_data producer_data;
  producer_data.numbers = &numbers[0];
  producer_data.numbers_size = numbers.size();

  pthread_t producer;
  int producer_create_status =
      pthread_create(&producer, NULL, &producer_routine, &producer_data);
  if (producer_create_status != 0) {
    printf("Can't create thread producer, status = %d\n",
           producer_create_status);
    exit(ERROR_CREATE_THREAD);
  }

  // Запускаем N потоков consumer
  struct consumer_data consumer_data;
  consumer_data.max_sleep_time = max_sleep_time;
  consumer_data.debug_flag = debug_flag;

  pthread_t* consumer_threads = new pthread_t[n];
  for (int i = 0; i < n; i++) {
    int consumer_create_status = pthread_create(
        &consumer_threads[i], NULL, &consumer_routine, &consumer_data);
    if (consumer_create_status != 0) {
      printf("Can't create thread consumer, status = %d\n",
             consumer_create_status);
      exit(ERROR_CREATE_THREAD);
    }
  }

  // Запускаем поток consumer_interruptor
  struct consumer_interruptor_data consumer_interruptor_data;
  consumer_interruptor_data.consumer_threads = consumer_threads;
  consumer_interruptor_data.n = n;

  pthread_t consumer_interruptor;
  int consumer_interruptor_create_status =
      pthread_create(&consumer_interruptor, NULL, &consumer_interruptor_routine,
                     &consumer_interruptor_data);
  if (consumer_interruptor_create_status != 0) {
    printf("Can't create thread consumer_interruptor, status = %d\n",
           consumer_interruptor_create_status);
    exit(ERROR_CREATE_THREAD);
  }

  // Ожидаем завершения потока producer
  int producer_join_status = pthread_join(producer, NULL);
  if (producer_join_status != 0) {
    printf("Can't join thread producer, status = %d\n", producer_join_status);
    exit(ERROR_JOIN_THREAD);
  }

  // Ожидаем завершения всех потоков consumer
  int result = 0;
  for (int i = 0; i < n; i++) {
    int* thread_sum = nullptr;
    int consumer_join_status =
        pthread_join(consumer_threads[i], (void**)&thread_sum);
    if (consumer_join_status != 0) {
      printf("Can't join thread consumer, status = %d\n", consumer_join_status);
      exit(ERROR_JOIN_THREAD);
    }
    if (thread_sum != nullptr) {
      result += *thread_sum;
      delete thread_sum;
    }
  }

  // Ожидаем завершения потока consumer_interruptor
  int consumer_interruptor_join_status =
      pthread_join(consumer_interruptor, NULL);
  if (consumer_interruptor_join_status != 0) {
    printf("Can't join thread consumer_interruptor, status = %d\n",
           consumer_interruptor_join_status);
    exit(ERROR_JOIN_THREAD);
  }

  delete[] shared_variable_buffer;
  delete[] consumer_threads;
  pthread_mutex_destroy(&shared_variable_mutex);
  pthread_cond_destroy(&shared_variable_changed);
  return result;
}
