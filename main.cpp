// Stanisław Latuszek 203248
#include <fstream>
#include <iostream>
#include <locale>

struct metar {
  char airport[30];
};

void clear() {
#if defined(_WIN32) || defined(_WIN64)
  std::system("cls");
#elif defined(__linux__) || defined(__unix__)
  std::system("clear");
#endif
};

void charArrayCopy(char (&dst)[], const char (&src)[], int len) {
  for (int i = 0; i < len; i++) {
    dst[i] = src[i];
  };
};

int icaoToName(const char (&code)[4], metar &output) {
  const char CODES[][5] = {"EPWA", "EPKK", "EPGD", "EPKT", "EPWR",
                           "EPMO", "EPPO", "EPRZ", "EPSC", "EPLB",
                           "EPBY", "EPLL", "EPSY", "EPRA", "EPZG"};
  const char AIRPORTS[][30] = {"Warszawa Okęcie",
                               "Kraków-Balice",
                               "Gdańsk-Rębiechowo",
                               "Katowice w Pyrzowicach",
                               "Wrocław-Strachowice",
                               "Warszawa-Modlin",
                               "Poznań-Ławica",
                               "Rzeszów-Jasionka",
                               "Szczecin-Goleniów",
                               "Lublin",
                               "Bydgoszcz",
                               "Łódź",
                               "Olsztyn-Mazury",
                               "Warszawa-Radom",
                               "Zielona Góra-Babimost"};
  const int len = sizeof(CODES) / sizeof(CODES[0]);
  for (int i = 0; i <= len; i++) {
    if (code[0] == CODES[i][0] && code[1] == CODES[i][1] &&
        code[2] == CODES[i][2] && code[3] == CODES[i][3]) {
      charArrayCopy(output.airport, AIRPORTS[i],
                    sizeof(AIRPORTS[i]) / sizeof(AIRPORTS[i][0]));
      return 1;
    }
  }

  charArrayCopy(output.airport, "Nieznane lotnisko", 20);
  return 0;
};

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
         (std::tolower(input) != 'o' && std::tolower(input) != 'n' &&
          std::tolower(input) != 'q')) {
    std::cout << "Niepoprawny wybór, spróbuj ponownie: ";
    std::cin.clear();
  };
  return tolower(input);
};

void handleEntry(char (&entry)[120]) {
  metar result;
  char icao[4];
  // Grab first 4 characters
  for (int i = 0; i < 4; i++)
    icao[i] = entry[i];
  icaoToName(icao, result);
  std::cout << result.airport << std::endl;
};

void handleFile(std::string path) {
  std::fstream in_file;
  in_file.open("./Metar_przykładowe_pliki/Metar_Gdansk.txt");
  while (in_file.peek() != EOF) {
    char entry[120];
    in_file.getline(entry, 120);
    handleEntry(entry);
  };
};

int main() {
  // Set up utf locale for correct encoding on windows
  std::locale::global(std::locale("pl_PL.UTF-8"));
  char mode = menu();

  switch (mode) {
  case 'o':
    handleFile("./Metar_przykładowe_pliki/Metar_Gdansk.txt");
    break;
  case 'n':
    std::cout << "making a new file, rly rly" << std::endl;
    break;
  case 'q':
    return 0;
  }
  return 0;
};
