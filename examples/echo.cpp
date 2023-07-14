#include <scode.h>

#include <cstring>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>

void print_error(int error) {
  switch (error) {
  case SCODE_ERROR_EMPTY:
    return;
  case SCODE_ERROR_CRC:
    std::cout << "CRC Error" << std::endl;
    return;
  case SCODE_ERROR_BUFFER:
    return;
  case SCODE_ERROR_PARSE:
    std::cout << "Parse Error" << std::endl;
    return;
  case SCODE_ERROR_DUMP:
    std::cout << "Dump Error" << std::endl;
    return;
  default:
    std::cout << "Unknown Error" << std::endl;
    return;
  }
}

void run_code(CodeStream &cs) {
  code_t cmd;
  int res;
  char buf[1024];

  while (true) {
    res = cs.pop(&cmd);
    Code code(std::move(cmd));

    if (res == SCODE_ERROR_BUFFER) {
      return;
    }
    if (res != 0) {
      print_error(res);
      continue;
    }

    res = code.dump_human(buf, sizeof buf);
    if (res < 0) {
      print_error(res);
      continue;
    }
    std::cout << buf << std::flush;

    res = code.dump_binary(buf, sizeof buf);
    if (res < 0) {
      print_error(res);
      continue;
    }
    std::cout << std::setfill('0') << std::hex;
    for (size_t i = 0; i < res; ++i) {
      if (buf[i] >= 0x20 && buf[i] < 0x7F) {
        std::cout << buf[i];
      } else {
        std::cout << "\\x" << std::setw(2)
                  << (static_cast<unsigned>(buf[i]) & 0xFF);
      }
    }
    std::cout << std::endl;
    for (size_t i = 0; i < res; ++i) {
      if (buf[i] >= 0x20 && buf[i] < 0x7F) {
        std::cout << buf[i];
      } else {
        std::cout << '.';
      }
    }
    std::cout << std::endl;
  }
}

void run_stdin(CodeStream &cs) {
  for (std::string line; std::getline(std::cin, line);) {
    line += "\n";
    cs.update(line.c_str(), line.length());
    run_code(cs);
  }
}

int main(int argc, char *argv[]) {
  CodeStream cs(16);

  if (argc > 1) {
    for (int i = 1; i < argc; ++i) {
      cs.update(argv[i], strlen(argv[i]));
      char c = ' ';
      cs.update(&c, 1);
    }
    char c = '\n';
    cs.update(&c, 1);
    run_code(cs);
  } else {
    run_stdin(cs);
  }
  return 0;
}
