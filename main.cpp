#include <iostream>
#include "producer_consumer.h"
#include <vector>

using namespace std;

int main(int argc, char* argv[]) {

  // Определяем наличие флага -debug
  bool debug_flag = false;
  for (int i = 1; i < argc; i++){
    if (string(argv[i]) == "-debug") debug_flag = true;
  }

  // Считываем числа
  string rawInput;
  vector<string> numbers;
  while (getline(cin, rawInput, ' ')){
    if (rawInput == "\n") {
        break;
    }
    numbers.push_back(rawInput);
  }


  for (size_t i = 0; i < numbers.size(); i++){
    cout << numbers[i] << endl;
  }

  cout << "Debug_flag " << debug_flag << endl;

  cout << run_threads() << endl;
  return 0;
}
