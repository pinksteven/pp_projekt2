// Stanisław Latuszek 203248
#include <cctype>
#include <fstream>
#include <iostream>
#include <locale>

void clear() {
#if defined(_WIN32) || defined(_WIN64)
  std::system("cls");
#elif defined(__linux__) || defined(__unix__)
  std::system("clear");
#endif
}

char menu() {
  std::cout << " __  __ _____ _____  _    ____  " << std::endl
            << "|  \\/  | ____|_   _|/ \\  |  _ \\ " << std::endl
            << "| |\\/| |  _|   | | / _ \\ | |_) |" << std::endl
            << "| |  | | |___  | |/ ___ \\|  _ < " << std::endl
            << "|_|  |_|_____| |_/_/   \\_\\_| \\_\\" << std::endl
            << "      Przez: Stanisław Latuszek 203248" << std::endl
            << std::endl;

  char input;
  std::cout << "Wybierz jedną z opcji: " << std::endl
            << "'o' - odczyt z pliku" << std::endl
            << "'n' - stwórz nowy METAR" << std::endl
            << "'q' - wyjście z programu" << std::endl
            << "Twój wybór: ";
  while (!(std::cin >> input) ||
         (input != 'o' && input != 'n' && input != 'q')) {
    std::cout << "Niepoprawny wybór, spróbuj ponownie: ";
    std::cin.clear();
  };
  return input;
}

int main() {
  // Set up utf locale for correct encoding on windows
  std::locale::global(std::locale("pl_PL.UTF-8"));
  menu();
  return 0;
}
