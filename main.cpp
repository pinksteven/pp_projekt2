// Stanisław Latuszek 203248
#include <cctype>
#include <fstream>
#include <iostream>
#include <locale>

// hold all wind data
struct air {
  bool measured;
  bool direct_stable;
  int degree, deg_from, deg_upto;
  bool speed_stable;
  int speed, speed_upto;
  char unit[4];
  bool vrb = false;
};
// weather condition (clouds, visibility, precipitation, temperature)
struct conditions {
  bool cavok = false;
  // for directional it goes [N, NE, E, SE, S, SW, W, NW]
  // else take the index 0
  bool directional = false;
  int visibility[8] = {-1, -1, -1, -1, -1, -1, -1, -1};

  // list of all possible weather types 0=not there
  // else it's a 2 digit number
  // first digit (1-4) is the intensity
  // second digit is the characteristic +1
  // so heavy showers [+SH] would be 18
  // order {BR, DS, DU, DZ, FC, FG, FU, GR, GS, HZ, IC, PE, PO, PY, RA, SA, SG,
  // SN, SQ, SS, UP, VA} for intensity {heavy, light, RE, VC} characteristics
  // {BC, DR, MI, PR, BL, FZ, SH, TS}
  int weather[22] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                     0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

  // out of my ass no-justsu 10 layers
  // [layer][0=cover, 1=height, 2=CB/TCU]
  int clouds[10][3] = {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0},
                       {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}};
  int layers = 0; // holds the number of layers that are there
  // height in ft, cover goes {FEW, SCT, BKN, OVC, VV}, {CB = 1, TCU = 2, ///=
  // 3} I'm gonna assume all the different specials just gove me "no clouds"
  bool cloudless = false;
};

// main struct
struct metar {
  char airport[30];
  int day, hour, minutes;
  air wind;
  conditions weather;
};

// system use different commands :/
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

int length(const char *a) {
  int out = 0;
  for (int i = 0; i < 20; i++) {
    if (a[i] == '\0') {
      out = i;
      break;
    };
  };
  return out;
}

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
  if (charArrCompare(test, "MPH") || charArrCompare(test, "MPS"))
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
  int len = length(input);
  if (len != 7 || input[3] != 'V')
    return false;

  output.wind.direct_stable = false;
  output.wind.deg_from =
      (input[0] - '0') * 100 + (input[1] - '0') * 10 + (input[2] - '0');
  output.wind.deg_upto =
      (input[4] - '0') * 100 + (input[5] - '0') * 10 + (input[6] - '0');
  return true;
};

bool parseVisibility(const char (&input)[20], metar &output) {
  int len = length(input);
  int vis = 0;
  int offset = 0;
  char end[4] = "";
  for (int i = 0; i < len; i++) {
    if (isdigit(input[i])) {
      vis = vis * 10 + (input[i] - '0');
      offset++;
    } else if (isupper(input[i])) {
      end[i - offset] = input[i];
    } else {
      return false;
    };
  };
  if (charArrCompare(end, "") || charArrCompare(end, "NDV")) {
    output.weather.visibility[0] = vis;
    return true;
  } else if (charArrCompare(end, "N")) {
    output.weather.directional = true;
    output.weather.visibility[0] = vis;
    return true;
  } else if (charArrCompare(end, "NE")) {
    output.weather.directional = true;
    output.weather.visibility[1] = vis;
    return true;
  } else if (charArrCompare(end, "E")) {
    output.weather.directional = true;
    output.weather.visibility[2] = vis;
    return true;
  } else if (charArrCompare(end, "SE")) {
    output.weather.directional = true;
    output.weather.visibility[3] = vis;
    return true;
  } else if (charArrCompare(end, "S")) {
    output.weather.directional = true;
    output.weather.visibility[4] = vis;
    return true;
  } else if (charArrCompare(end, "SW")) {
    output.weather.directional = true;
    output.weather.visibility[5] = vis;
    return true;
  } else if (charArrCompare(end, "W")) {
    output.weather.directional = true;
    output.weather.visibility[6] = vis;
    return true;
  } else if (charArrCompare(end, "NW")) {
    output.weather.directional = true;
    output.weather.visibility[7] = vis;
    return true;
  }
  return false;
};

bool parsePhenomena(const char (&input)[20], metar &output) {
  int modifier = 1;
  bool found = false;
  char characteristic[][3] = {"BC", "DR", "MI", "PR", "BL", "FZ", "SH", "TS"};
  char type[][3] = {"BR", "DS", "DU", "DZ", "FC", "FG", "FU", "GR",
                    "GS", "HZ", "IC", "PE", "PO", "PY", "RA", "SA",
                    "SG", "SN", "SQ", "SS", "UP", "VA"};
  char test[3] = "";
  for (int i = 0; i < 20; i += 2) {
    /* std::cout << i << std::endl; */
    test[0] = input[i];
    test[1] = input[i + 1];
    test[2] = '\0';
    if (input[i] == '+') {
      modifier += 10;
      i--;
    } else if (input[i] == '-') {
      modifier += 20;
      i--;
    } else if (input[i] == 'R' && input[i + 1] == 'E') {
      modifier += 30;
    } else if (input[i] == 'V' && input[i + 1] == 'C') {
      modifier += 40;
    };
    /* std::cout << test << std::endl; */
    for (int j = 0; j < 8; j++) {
      if (charArrCompare(characteristic[j], test)) {
        modifier += j + 1;
      };
    };
    for (int j = 0; j < 22; j++) {
      if (charArrCompare(type[j], test)) {
        output.weather.weather[j] = modifier;
        found = true;
        modifier = 1;
      };
    };
  };
  return found;
}

bool parseClouds(const char (&input)[20], metar &output) {
  int len = length(input);
  char pre[][4] = {"FEW", "SCT", "BKN", "OVC", "VV",
                   "NSC", "SKC", "NCD", "CLR"};
  bool found = false;
  char test[4] = {input[0], input[1], input[2], '\0'};
  for (int i = 0; i < 8; i++) {
    if (charArrCompare(test, pre[i])) {
      output.weather.cloudless = (i >= 5 && i <= 8) ? true : false;
      if (i < 5) {
        output.weather.clouds[output.weather.layers][0] = i;
      };
      found = true;
      break;
    }
  }
  if (!found)
    return false;

  int height;
  for (int i = 3; i < len - 2; i++) {
    if (isdigit(input[i])) {
      height = (input[i] - '0') * 100 + (input[i + 1] - '0') * 10 +
               (input[i + 2] - '0');
      i += 3;
    } else {
      test[0] = input[i];
      test[1] = input[i + 1];
      test[2] = input[i + 2];
      test[3] = '\0';

      if (charArrCompare(test, "CB")) {
        output.weather.clouds[output.weather.layers][2] = 1;
        break;
      } else if (charArrCompare(test, "TCU")) {
        output.weather.clouds[output.weather.layers][2] = 2;
        break;
      } else if (charArrCompare(test, "///")) {
        output.weather.clouds[output.weather.layers][2] = 3;
        break;
      }
    }
  }
  output.weather.clouds[output.weather.layers][1] = height * 100;
  output.weather.layers++;

  return true;
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
      else if (parseVisibility(part, result))
        std::cout << "visibility " << result.weather.visibility[0] << "m"
                  << std::endl;
      else if (parsePhenomena(part, result))
        std::cout << "weather" << std::endl;
      else if (parseClouds(part, result))
        std::cout << "Clouds " << result.weather.clouds[0][1] << "m "
                  << result.weather.clouds[0][0] << std::endl;
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
    // TODO: implement
    std::cout << "making a new file, rly rly" << std::endl;
    break;
  case 'q':
    return 0;
  }
  return 0;
};
