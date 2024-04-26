#include <iostream>
#include "producer_consumer.h"

using namespace std;

int main(int argc, char* argv[]) {

  // Определяем наличие флага -debug
  bool debug_flag = false;
  for (int i = 1; i < argc; i++){
    if (argv[i] == "-debug") debug_flag = true;
  }

  // Считываем числа
  string rawInput;
  getline(cin, rawInput, ' ');

  cout << rawInput;

  cout << run_threads() << endl;
  return 0;
}
