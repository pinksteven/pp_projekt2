// Stanisław Latuszek 203248
#include <fstream>
#include <iostream>
#include <locale>

struct air {
  bool measured;
  bool direct_stable;
  int degree, deg_from, deg_upto;
  bool speed_stable;
  int speed, speed_upto;
  char unit[4];
  bool vrb = false;
};

struct metar {
  char airport[30];
  int day, hour, minutes;
  air wind;
};

void clear() {
#if defined(_WIN32) || defined(_WIN64)
  std::system("cls");
#elif defined(__linux__) || defined(__unix__)
  std::system("clear");
#endif
};

void charArrCpy(char *dst, const char *src) {
  int i = 0;
  do {
    dst[i] = src[i];
    i++;
  } while (src[i - 1] != '\0');
};

bool charArrCompare(const char *a, const char *b) {
  for (int i = 0; a[i] != '\0' || b[i] != '\0'; i++) {
    if (a[i] != b[i]) {
      return false;
    }
  };
  return true;
};

bool parseICAO(const char (&input)[20], metar &output) {
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

  for (int i = 0; i < 20; i++) {
    if (input[i] == '\0') {
      if (i == 4) {
        break;
      } else {
        return false;
      }
    } else if (i >= 4) {
      return false;
    } else if (!std::isupper(input[i])) {
      return false;
    };
  };

  for (int i = 0; i < 15; i++) {
    if (charArrCompare(CODES[i], input)) {
      charArrCpy(output.airport, AIRPORTS[i]);
      return true;
    };
  };

  charArrCpy(output.airport, "Nieznane lotnisko");
  return true;
};

bool parseTime(const char (&input)[20], metar &output) {
  for (int i = 0; i < 20; i++) {
    if (input[i] == '\0') {
      if (i == 7)
        break;
      else
        return false;
    } else if (i < 6 && !std::isdigit(input[i])) {
      return false;
    } else if (i == 6 && input[i] != 'Z') {
      return false;
    } else if (i > 7) {
      return false;
    }
  }

  unsigned int day = (input[0] - '0') * 10 + (input[1] - '0');
  unsigned int hour = (input[2] - '0') * 10 + (input[3] - '0');
  unsigned int minutes = (input[4] - '0') * 10 + (input[5] - '0');

  if (day > 31 || hour > 24 || minutes > 59)
    return false;
  else {
    output.day = day;
    output.hour = hour;
    output.minutes = minutes;
    return true;
  };
  return false;
}

bool parseWindPrim(const char (&input)[20], metar &output) {
  char test[4];
  for (int i = 0; i < 20; i++) {
    if (input[i] == '\0') {
      test[3] = '\0';
      test[2] = input[i - 1];
      test[1] = input[i - 2];
      test[0] = input[i - 3];
      break;
    };
  };
  if (charArrCompare(test, "MPS") || charArrCompare(test, "MPS"))
    charArrCpy(output.wind.unit, test);
  else if (test[1] == 'K' && test[2] == 'T')
    charArrCpy(output.wind.unit, "KT");
  else
    return false;

  if (input[0] == '/') {
    output.wind.measured = false;
    return true;
  } else if (charArrCompare(input, "VRB01")) {
    output.wind.measured = true;
    output.wind.vrb = true;
    return true;
  } else {
    output.wind.measured = true;
    int deg = (input[0] - '0') * 100 + (input[1] - '0') * 10 + (input[2] - '0');
    if (deg < 0 || deg > 360)
      return false;
    int speed = -1, upto = -1;
    int temp = 0;
    for (int i = 3; i < 20; i++) {
      if (input[i] == 'G')
        speed = temp;
      else if (input[i] == 'K' || input[i] == 'M')
        break;
      else
        temp = temp * 10 + (input[i] - '0');
    }
    if (speed == -1)
      speed = temp;
    else
      upto = temp;
    output.wind.direct_stable = true;
    output.wind.degree = deg;
    output.wind.speed_stable = !(upto == -1);
    output.wind.speed = speed;
    output.wind.speed_upto = upto;
  }
  return true;
};

bool parseWindSec(const char (&input)[20], metar &output) {
  int len;
  for (int i = 0; i < 20; i++) {
    if (input[i] == '\0') {
      len = i;
      break;
    };
  };
  if (len != 7 || input[3] != 'V')
    return false;

  output.wind.direct_stable = false;
  output.wind.deg_from =
      (input[0] - '0') * 100 + (input[1] - '0') * 10 + (input[2] - '0');
  output.wind.deg_upto =
      (input[4] - '0') * 100 + (input[5] - '0') * 10 + (input[6] - '0');
  return true;
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

void handleEntry(const char (&entry)[120]) {
  metar result;
  int offset = 0;
  char part[20];
  for (int i = 0; i < 120; i++) {
    if (entry[i] == '\0') {
      /* std::cout << part << std::endl; */
      break;
    } else if (entry[i] == ' ') {
      part[i - offset] = '\0';
      if (parseICAO(part, result))
        std::cout << result.airport << std::endl;
      else if (parseTime(part, result))
        std::cout << result.day << " Dzień miesiąca " << result.hour << ":"
                  << result.minutes << " UTC" << std::endl;
      else if (parseWindPrim(part, result))
        std::cout << "wind " << result.wind.degree << " " << result.wind.speed
                  << std::endl;
      else if (parseWindSec(part, result))
        std::cout << "wind secondary" << std::endl;
      offset = i + 1;
    } else {
      part[i - offset] = entry[i];
    };
  };
  std::cout << std::endl;
};

void handleFile(std::string path) {
  std::fstream in_file;
  in_file.open("./Metar_przykładowe_pliki/Metar_Gdansk.txt");
  while (in_file.peek() != EOF) {
    char entry[120];
    in_file.getline(entry, 120);
    std::cout << entry << std::endl;
    handleEntry(entry);
    std::cout << std::endl;
    entry[0] = '\0';
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
