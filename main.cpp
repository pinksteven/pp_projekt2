#include <iostream>
#include <locale>

int main() {
  std::locale::global(std::locale("pl_PL.UTF-8"));
  std::cout << "Hello World!";
  return 0;
}
