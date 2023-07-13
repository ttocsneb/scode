#include <stdio.h>

#include <scode.h>
#include <string.h>

void print_error(int error) {
  switch (error) {
  case SCODE_ERROR_EMPTY:
    return;
  case SCODE_ERROR_CRC:
    printf("CRC Error\n");
    return;
  case SCODE_ERROR_BUFFER:
    return;
  case SCODE_ERROR_PARSE:
    printf("Parse Error\n");
    return;
  case SCODE_ERROR_DUMP:
    printf("Dump Error\n");
    return;
  default:
    printf("Unknown Error\n");
    return;
  }
}

void run_code(code_stream_t *cs) {
  code_t cmd;
  int res;
  char b[1024];

  while (1) {
    res = code_stream_pop(cs, &cmd);
    if (res == SCODE_ERROR_BUFFER) {
      return;
    }
    if (res != 0) {
      print_error(res);
      continue;
    }

    res = code_dump_human(&cmd, b, sizeof(b));
    if (res < 0) {
      print_error(res);
      continue;
    }
    b[res] = '\0';
    printf("%s", b);

    res = code_dump_binary(&cmd, b, sizeof(b));
    if (res < 0) {
      print_error(res);
      continue;
    }
    for (size_t i = 0; i < res; ++i) {
      if (b[i] >= 0x20 && b[i] < 0x7F) {
        putchar(b[i]);
      } else {
        printf("\\x%02X", (uint8_t)b[i]);
      }
    }
    printf("\n");
    for (size_t i = 0; i < res; ++i) {
      if (b[i] >= 0x20 && b[i] < 0x7F) {
        putchar(b[i]);
      } else {
        putchar('.');
      }
    }
    printf("\n");
    free_code(&cmd);
  }
}

void run_stdin(code_stream_t *cs) {
  size_t len = 0;
  int res = 0;
  while (res != EOF) {
    res = getchar();
    if (res != EOF) {
      char c = res;
      code_stream_update(cs, &c, 1);
    }
    if (res == '\n' || res == EOF) {
      run_code(cs);
    }
  }
}

int main(int argc, char *argv[]) {
  code_stream_t cs = init_code_stream(16);
  if (argc > 1) {
    for (int i = 1; i < argc; ++i) {
      code_stream_update(&cs, argv[i], strlen(argv[i]));
      char c = ' ';
      code_stream_update(&cs, &c, 1);
    }
    char c = '\n';
    code_stream_update(&cs, &c, 1);
    run_code(&cs);
  } else {
    run_stdin(&cs);
  }
  free_code_stream(&cs);
  return 0;
}
