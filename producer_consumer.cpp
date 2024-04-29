#include <pthread.h>
#include <iostream>
#include <atomic>
#include <thread>
#include <iostream>
#include <vector>

#define ERROR_CREATE_THREAD -11
#define ERROR_JOIN_THREAD -12

using namespace std;

// Мьютекс
pthread_mutex_t shared_variable_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t shared_variable_written = PTHREAD_COND_INITIALIZER; // Данные записаны producer-ом
pthread_cond_t shared_variable_read = PTHREAD_COND_INITIALIZER; // Данные прочитаны consumer-ом

// Флаги завершения
bool flag_finish_write = false;
bool flag_finish_read = false;

// Разделяемая переменная
struct shared_variable {
  // Значение разделяемой переменной
  int value;
  // Флаг наличия значения в разделяемой переменной
  bool flag_presence_value;
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


// Данные, передаваемые в producer
struct producer_data{
  struct shared_variable* shared_variable;
  int* numbers;
  size_t numbers_size;
};

// Поток producer
void* producer_routine(void* arg) {
  (void)arg;
  
  // Получаем исходные данные
  struct producer_data* producer_data = (struct producer_data*) arg;
  struct shared_variable* shared_variable = producer_data->shared_variable;
  int* numbers = producer_data->numbers;
  size_t numbers_size = producer_data->numbers_size;

  // Записываем данные в разделяемую переменную
  for (size_t i = 0; i < numbers_size; i++){
    pthread_mutex_lock(&shared_variable_mutex);

    shared_variable->value = numbers[i];
    shared_variable->flag_presence_value = true;

    if (i == numbers_size - 1){
      flag_finish_write = true;
    }

    pthread_cond_signal(&shared_variable_written);

    while (shared_variable->flag_presence_value != false){
      pthread_cond_wait(&shared_variable_read, &shared_variable_mutex);
    }

    pthread_mutex_unlock(&shared_variable_mutex);
  }

  return nullptr;
}


// Данные передаваемые в consumer
struct consumer_data{
  struct shared_variable* shared_variable;
  bool debug_flag;
  int max_sleep_time;
};

// Поток consumer
void* consumer_routine(void* arg) {
  (void)arg;
  int* thread_sum = new int(0);

  pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, nullptr);
  
  // Получаем исходные данные
  struct consumer_data* consumer_data = (struct consumer_data*) arg;
  struct shared_variable* shared_variable = consumer_data->shared_variable;
  bool debug_flag = consumer_data->debug_flag;
  int max_sleep_time = consumer_data->max_sleep_time;

  // Читаем данные из разделяемой переменной
  while (flag_finish_write == false || flag_finish_read == false){
    pthread_mutex_lock(&shared_variable_mutex);

    while (shared_variable->flag_presence_value != true){
      pthread_cond_wait(&shared_variable_written, &shared_variable_mutex);
    }

    int tmp = shared_variable->value;
    shared_variable->flag_presence_value = false;
    *thread_sum += tmp;
    if (debug_flag) cout << get_tid() << " " << *thread_sum << endl;

    if (flag_finish_write == true && shared_variable->flag_presence_value == false){
      flag_finish_read = true;
    }

    pthread_cond_signal(&shared_variable_read);

    pthread_mutex_unlock(&shared_variable_mutex);

    long sleep_time = (max_sleep_time == 0)
                           ? 0
                           : (rand() % (max_sleep_time + 1));
    this_thread::sleep_for(chrono::milliseconds(sleep_time));
  }

  pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, nullptr);

  return thread_sum;
}


// Данные передаваемые в consumer_interruptor
struct consumer_interruptor_data{
  pthread_t* consumer_threads;
  int n; 
};

void* consumer_interruptor_routine(void* arg) {
  (void)arg;
  
  // Получаем исходные данные
  struct consumer_interruptor_data* consumer_interruptor_data = (struct consumer_interruptor_data*) arg;
  pthread_t* consumer_threads = consumer_interruptor_data->consumer_threads;
  int n = consumer_interruptor_data->n;

  int random_consumer;
  while (flag_finish_write == false || flag_finish_read == false){
    random_consumer = rand() % n;
    pthread_cancel(consumer_threads[random_consumer]);
  }

  return nullptr;
}


// Запуск потоков
int run_threads(int n, int max_sleep_time, vector<int> numbers, bool debug_flag) {
  // Пока для тестов выводим полученные переменные
  cout << "Debug: " << debug_flag << endl;
  cout << "N: " << n << endl;
  cout << "Max_sleep_time: " << max_sleep_time << endl;
  cout << "Numbers: ";
  for (size_t i = 0; i < numbers.size(); i++){
    cout << numbers[i] << " ";
  }
  cout << endl;

  // Инициализируем разделяемую переменную
  // и заполняем её значениями по умолчанию
  struct shared_variable shared_variable;
  shared_variable.value = 0;
  shared_variable.flag_presence_value = false;

  // Запускаем поток producer
  struct producer_data producer_data;
  producer_data.shared_variable = &shared_variable;
  producer_data.numbers = &numbers[0];
  producer_data.numbers_size = numbers.size();
  
  pthread_t producer;
  int producer_create_status = pthread_create(&producer, NULL, &producer_routine, &producer_data);
  if (producer_create_status != 0) {
    printf("Can't create thread producer, status = %d\n", producer_create_status);
    exit(ERROR_CREATE_THREAD);
  }

  // Запускаем N потоков consumer
  struct consumer_data consumer_data;
  consumer_data.shared_variable = &shared_variable;
  consumer_data.max_sleep_time = max_sleep_time;
  consumer_data.debug_flag = debug_flag;
  
  pthread_t* consumer_threads = new pthread_t[n];
  for (int i = 0; i < n; i++){
    int consumer_create_status = pthread_create(&consumer_threads[i], NULL, &consumer_routine, &consumer_data);
    if (consumer_create_status != 0) {
      printf("Can't create thread consumer, status = %d\n", consumer_create_status);
      exit(ERROR_CREATE_THREAD);
    }
  }

  // Ожидаем завершения потока producer
  int producer_join_status = pthread_join(producer, NULL);
  if (producer_join_status != 0) {
    printf("Can't join thread producer, status = %d\n", producer_join_status);
    exit(ERROR_JOIN_THREAD);
  }

  // Ожидаем завершения всех потоков consumer
  int result = 0;
  for (int i = 0; i < n; i++){
    int* thread_sum = nullptr;
    int consumer_join_status = pthread_join(consumer_threads[i], (void**)&thread_sum);
    if (consumer_join_status != 0) {
      printf("Can't join thread consumer, status = %d\n", consumer_join_status);
      exit(ERROR_JOIN_THREAD);
    }
    if (thread_sum != nullptr) {
      result += *thread_sum;
      free(thread_sum);
    }
  }

  cout << "Сумма = " << result << endl;

  return 0;
}
