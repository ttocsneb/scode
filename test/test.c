#include <munit.h>

#include <scode.h>

#define TEST(name)                                                             \
  static MunitResult name(const MunitParameter params[], void *data)
#define TEST_ITEM(test)                                                        \
  { #test, test, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL }
#define TEST_NULL                                                              \
  { NULL, NULL, NULL, NULL, 0, NULL }

TEST(test_param_init) {
  param_t param;

  param = init_param_i8('T', 4);
  munit_assert_uint8(param_type(&param), ==, PARAM_T_I8);
  munit_assert_int8(param.i8, ==, 4);

  return MUNIT_OK;
}
TEST(test_param_dump_binary) {
  char buf[1024];
  int result;
  param_t param;

  param = init_param_i8('T', 4);
  result = param_dump_binary(&param, buf, sizeof(buf));
  munit_assert_uint8(result, ==, 2);
  munit_assert_memory_equal(result, buf, "\xB4\x04");

  param = init_param_i8('T', -4);
  result = param_dump_binary(&param, buf, sizeof(buf));
  munit_assert_uint8(result, ==, 2);
  munit_assert_memory_equal(result, buf, "\xB4\xFC");

  param = init_param_i16('T', 12345);
  result = param_dump_binary(&param, buf, sizeof(buf));
  munit_assert_uint8(result, ==, 3);
  munit_assert_memory_equal(result, buf, "\x94\x39\x30");

  param = init_param_i32('T', -123450000);
  result = param_dump_binary(&param, buf, sizeof(buf));
  munit_assert_uint8(result, ==, 5);
  munit_assert_memory_equal(result, buf, "\x74\x70\x4d\xa4\xf8");

  char *str = malloc(1024);
  strcpy(str, "Hello World");
  param = init_param_str('T', str);
  result = param_dump_binary(&param, buf, sizeof(buf));
  munit_assert_uint8(result, >, 0);
  munit_assert_memory_equal(result, buf, "\xF4Hello World\0");

  strcpy(str, "won't you join us?");
  param = init_param_str('T', str);
  result = param_dump_binary(&param, buf, sizeof(buf));
  munit_assert_uint8(result, >, 0);
  munit_assert_memory_equal(result, buf, "\xF4won't you join us?\0");

  strcpy(str, "\"oops'");
  param = init_param_str('T', str);
  result = param_dump_binary(&param, buf, sizeof(buf));
  munit_assert_uint8(result, >, 0);
  munit_assert_memory_equal(result, buf, "\xF4\"oops'\0");

  free_param(&param);

  return MUNIT_OK;
}

TEST(test_param_dump_human) {
  char buf[1024];
  int result;
  param_t param;

  param = init_param_i8('T', 4);
  result = param_dump_human(&param, buf, sizeof(buf));
  munit_assert_uint8(result, ==, 2);
  buf[result] = '\0';
  munit_assert_string_equal(buf, "T4");

  param = init_param_i8('T', -4);
  result = param_dump_human(&param, buf, sizeof(buf));
  munit_assert_uint8(result, ==, 3);
  buf[result] = '\0';
  munit_assert_string_equal(buf, "T-4");

  param = init_param_i16('T', 12345);
  result = param_dump_human(&param, buf, sizeof(buf));
  munit_assert_uint8(result, >, 0);
  buf[result] = '\0';
  munit_assert_string_equal(buf, "T12345");

  param = init_param_i32('T', -123450000);
  result = param_dump_human(&param, buf, sizeof(buf));
  munit_assert_uint8(result, >, 0);
  buf[result] = '\0';
  munit_assert_string_equal(buf, "T-123450000");

  param = init_param_f32('T', 0.5);
  result = param_dump_human(&param, buf, sizeof(buf));
  munit_assert_uint8(result, >, 0);
  buf[result] = '\0';
  munit_assert_string_equal(buf, "T0.5");

  param = init_param_f32('T', -0.5);
  result = param_dump_human(&param, buf, sizeof(buf));
  munit_assert_uint8(result, >, 0);
  buf[result] = '\0';
  munit_assert_string_equal(buf, "T-0.5");

  param = init_param_f32('T', 123.0);
  result = param_dump_human(&param, buf, sizeof(buf));
  munit_assert_uint8(result, >, 0);
  buf[result] = '\0';
  munit_assert_string_equal(buf, "T123.0");

  param = init_param_f32('T', 123.125);
  result = param_dump_human(&param, buf, sizeof(buf));
  munit_assert_uint8(result, >, 0);
  buf[result] = '\0';
  munit_assert_string_equal(buf, "T123.125");

  char *str = malloc(1024);
  strcpy(str, "Hello World");
  param = init_param_str('T', str);
  result = param_dump_human(&param, buf, sizeof(buf));
  munit_assert_uint8(result, >, 0);
  buf[result] = '\0';
  munit_assert_string_equal(buf, "T'Hello World'");

  strcpy(str, "won't you join us?");
  param = init_param_str('T', str);
  result = param_dump_human(&param, buf, sizeof(buf));
  munit_assert_uint8(result, >, 0);
  buf[result] = '\0';
  munit_assert_string_equal(buf, "T\"won't you join us?\"");

  strcpy(str, "\"oops'");
  param = init_param_str('T', str);
  result = param_dump_human(&param, buf, sizeof(buf));
  munit_assert_uint8(result, ==, SCODE_ERROR_DUMP);

  free_param(&param);
  return MUNIT_OK;
}

TEST(test_param_parse_human) {
  char *buffer;
  param_t param;
  int result;

  buffer = "D2";
  result = param_parse_human(&param, buffer, strlen(buffer));
  munit_assert_int(result, ==, 2);
  munit_assert_uint8(param_type(&param), ==, PARAM_T_U8);
  munit_assert_uint8(param.u8, ==, 2);
  free_param(&param);

  buffer = "D-2";
  result = param_parse_human(&param, buffer, strlen(buffer));
  munit_assert_int(result, ==, 3);
  munit_assert_uint8(param_type(&param), ==, PARAM_T_I8);
  munit_assert_int8(param.i8, ==, -2);
  free_param(&param);

  buffer = "D500";
  result = param_parse_human(&param, buffer, strlen(buffer));
  munit_assert_int(result, ==, 4);
  munit_assert_uint8(param_type(&param), ==, PARAM_T_I16);
  munit_assert_int16(param.i16, ==, 500);
  free_param(&param);

  buffer = "D500000";
  result = param_parse_human(&param, buffer, strlen(buffer));
  munit_assert_int(result, ==, 7);
  munit_assert_uint8(param_type(&param), ==, PARAM_T_I32);
  munit_assert_int32(param.i32, ==, 500000);
  free_param(&param);

  buffer = "D5000000000000000000";
  result = param_parse_human(&param, buffer, strlen(buffer));
  munit_assert_int(result, ==, 20);
  munit_assert_uint8(param_type(&param), ==, PARAM_T_I64);
  munit_assert_int64(param.i64, ==, 5000000000000000000);
  free_param(&param);

  buffer = "D000000000000";
  result = param_parse_human(&param, buffer, strlen(buffer));
  munit_assert_int(result, ==, 13);
  munit_assert_uint8(param_type(&param), ==, PARAM_T_U8);
  munit_assert_uint8(param.u8, ==, 0);
  free_param(&param);

  buffer = "D10.2";
  result = param_parse_human(&param, buffer, strlen(buffer));
  munit_assert_int(result, ==, 5);
  munit_assert_uint8(param_type(&param), ==, PARAM_T_F32);
  munit_assert_float(param.f32, ==, 10.2);
  free_param(&param);

  buffer = "D00000000000000010.2";
  result = param_parse_human(&param, buffer, strlen(buffer));
  munit_assert_int(result, ==, 20);
  munit_assert_uint8(param_type(&param), ==, PARAM_T_F32);
  munit_assert_float(param.f32, ==, 10.2);
  free_param(&param);

  buffer = "D.2";
  result = param_parse_human(&param, buffer, strlen(buffer));
  munit_assert_int(result, ==, 3);
  munit_assert_uint8(param_type(&param), ==, PARAM_T_F32);
  munit_assert_float(param.f32, ==, .2);
  free_param(&param);

  buffer = "D0.0000102";
  result = param_parse_human(&param, buffer, strlen(buffer));
  munit_assert_int(result, ==, 10);
  munit_assert_uint8(param_type(&param), ==, PARAM_T_F32);
  munit_assert_float(param.f32, ==, 0.0000102);
  free_param(&param);

  buffer = "D0.00000102";
  result = param_parse_human(&param, buffer, strlen(buffer));
  munit_assert_int(result, ==, 11);
  munit_assert_uint8(param_type(&param), ==, PARAM_T_F64);
  munit_assert_double(param.f64, ==, 0.00000102);
  free_param(&param);

  buffer = "D5.";
  result = param_parse_human(&param, buffer, strlen(buffer));
  munit_assert_int(result, ==, 3);
  munit_assert_uint8(param_type(&param), ==, PARAM_T_F32);
  munit_assert_float(param.f32, ==, 5);
  free_param(&param);

  buffer = "D.";
  result = param_parse_human(&param, buffer, strlen(buffer));
  munit_assert_int(result, ==, 2);
  munit_assert_uint8(param_type(&param), ==, PARAM_T_F32);
  munit_assert_float(param.f32, ==, 0);
  free_param(&param);

  buffer = "D'hello world'";
  result = param_parse_human(&param, buffer, strlen(buffer));
  munit_assert_int(result, ==, 14);
  munit_assert_uint8(param_type(&param), ==, PARAM_T_STR);
  munit_assert_string_equal(param.str, "hello world");
  free_param(&param);

  buffer = "D'hello world";
  result = param_parse_human(&param, buffer, strlen(buffer));
  munit_assert_int(result, ==, SCODE_ERROR_BUFFER);
  free_param(&param);

  buffer = "D'hello world' dooasdf";
  free_param(&param);
  result = param_parse_human(&param, buffer, strlen(buffer));
  munit_assert_int(result, ==, 14);
  munit_assert_uint8(param_type(&param), ==, PARAM_T_STR);
  munit_assert_string_equal(param.str, "hello world");
  free_param(&param);

  buffer = "D";
  result = param_parse_human(&param, buffer, strlen(buffer));
  munit_assert_int(result, ==, SCODE_ERROR_BUFFER);
  free_param(&param);

  return MUNIT_OK;
}

TEST(test_param_parse_binary) {
  char *buffer;
  param_t param;
  int result;

  buffer = "\xB4\x02";
  result = param_parse_binary(&param, buffer, strlen(buffer));
  munit_assert_int(result, ==, 2);
  munit_assert_uint8(param_type(&param), ==, PARAM_T_I8);
  munit_assert_int8(param.i8, ==, 2);
  free_param(&param);

  buffer = "\xB4\xFC";
  result = param_parse_binary(&param, buffer, strlen(buffer));
  munit_assert_int(result, ==, 2);
  munit_assert_uint8(param_type(&param), ==, PARAM_T_I8);
  munit_assert_int8(param.i8, ==, -4);
  free_param(&param);

  buffer = "\x94\x39\x30";
  result = param_parse_binary(&param, buffer, strlen(buffer));
  munit_assert_int(result, ==, 3);
  munit_assert_uint8(param_type(&param), ==, PARAM_T_I16);
  munit_assert_int16(param.i16, ==, 12345);
  free_param(&param);

  buffer = "\x74\x70\x4d\xa4\xf8";
  result = param_parse_binary(&param, buffer, strlen(buffer));
  munit_assert_int(result, ==, 5);
  munit_assert_uint8(param_type(&param), ==, PARAM_T_I32);
  munit_assert_int32(param.i32, ==, -123450000);
  free_param(&param);

  buffer = "\xF4Hello World\0";
  result = param_parse_binary(&param, buffer, 13);
  munit_assert_int(result, ==, 13);
  munit_assert_uint8(param_type(&param), ==, PARAM_T_STR);
  munit_assert_string_equal(param.str, "Hello World");
  free_param(&param);

  buffer = "\xF4won't you join us?\0";
  result = param_parse_binary(&param, buffer, 20);
  munit_assert_int(result, ==, 20);
  munit_assert_uint8(param_type(&param), ==, PARAM_T_STR);
  munit_assert_string_equal(param.str, "won't you join us?");
  free_param(&param);

  buffer = "\xF4\"oops'\0";
  result = param_parse_binary(&param, buffer, 8);
  munit_assert_int(result, ==, 8);
  munit_assert_uint8(param_type(&param), ==, PARAM_T_STR);
  munit_assert_string_equal(param.str, "\"oops'");
  free_param(&param);

  return MUNIT_OK;
}

TEST(test_code_dump_human) {
  char buf[1024];
  code_t code;
  int result;

  code = init_code('S', 2, 0);
  result = code_dump_human(&code, buf, sizeof(buf));
  munit_assert_int(result, ==, 3);
  buf[result] = '\0';
  munit_assert_string_equal(buf, "S2\n");
  free_code(&code);

  code = init_code('S', 2, 1);
  code.params[0] = init_param_i16('N', 1234);
  result = code_dump_human(&code, buf, sizeof(buf));
  munit_assert_int(result, ==, 9);
  buf[result] = '\0';
  munit_assert_string_equal(buf, "S2 N1234\n");
  free_code(&code);

  return MUNIT_OK;
}

TEST(test_code_dump_binary) {
  char buf[1024];
  code_t code;
  int result;

  code = init_code('S', 2, 0);
  result = code_dump_binary(&code, buf, sizeof(buf));
  munit_assert_int(result, ==, 4);
  munit_assert_memory_equal(4, buf, "\xD3\x02\x00\x8B");
  free_code(&code);

  code = init_code('S', 2, 1);
  code.params[0] = init_param_i16('N', 1235);
  result = code_dump_binary(&code, buf, sizeof(buf));
  munit_assert_int(result, ==, 7);
  munit_assert_memory_equal(7, buf, "\xD3\x02\x8E\xD3\x04\x00\x59");
  free_code(&code);

  return MUNIT_OK;
}

TEST(test_code_parse_human) {
  char *buf;
  code_t code;
  int result;

  buf = "S2\n";
  result = code_parse(&code, buf, strlen(buf));
  munit_assert_int(result, ==, 3);
  munit_assert_uint8(code_letter(&code), ==, 'S');
  munit_assert_int(code_is_binary(&code), ==, 0);
  munit_assert_uint8(code.number, ==, 2);
  munit_assert_ptr_null(code.params);
  free_code(&code);

  buf = "S2   N1234\n";
  result = code_parse(&code, buf, strlen(buf));
  munit_assert_int(result, ==, 11);
  munit_assert_uint8(code_letter(&code), ==, 'S');
  munit_assert_int(code_is_binary(&code), ==, 0);
  munit_assert_uint8(code.number, ==, 2);
  munit_assert_ptr_not_null(code.params);
  munit_assert_uint8(param_type(&code.params[0]), ==, PARAM_T_I16);
  munit_assert_uint8(param_letter(&code.params[0]), ==, 'N');
  munit_assert_int16(code.params[0].i16, ==, 1234);
  munit_assert_uint8(code.params[1].param, ==, 0);
  free_code(&code);

  return MUNIT_OK;
}

TEST(test_code_parse_binary) {
  char *buf;
  code_t code = {0};
  int result;

  buf = "\xD3\x02\x00\x8B";
  result = code_parse(&code, buf, 4);
  munit_assert_int(result, ==, 4);
  munit_assert_uint8(code_letter(&code), ==, 'S');
  munit_assert_int(code_is_binary(&code), !=, 0);
  munit_assert_uint8(code.number, ==, 2);
  munit_assert_ptr_null(code.params);
  free_code(&code);

  buf = "\xD3\x02\x8E\xD3\x04\x00\x59";
  result = code_parse(&code, buf, 7);
  munit_assert_int(result, ==, 7);
  munit_assert_uint8(code_letter(&code), ==, 'S');
  munit_assert_int(code_is_binary(&code), !=, 0);
  munit_assert_uint8(code.number, ==, 2);
  munit_assert_ptr_not_null(code.params);
  munit_assert_uint8(param_type(&code.params[0]), ==, PARAM_T_I16);
  munit_assert_uint8(param_letter(&code.params[0]), ==, 'N');
  munit_assert_int16(code.params[0].i16, ==, 1235);
  munit_assert_uint8(code.params[1].param, ==, 0);
  free_code(&code);

  buf = "\xD3\x06\x8E\xD3\x04\x00\x10";
  result = code_parse(&code, buf, 7);
  munit_assert_int(result, ==, SCODE_ERROR_CRC);
  free_code(&code);

  return MUNIT_OK;
}

TEST(test_crc) {
  char *buf;
  uint8_t result;

  buf = "Hello";
  result = crc_calc(buf, strlen(buf), 0);

  munit_assert_uint8(crc_calc(buf, strlen(buf), result), ==, 0);
  munit_assert_uint8(result, ==, 0xF6);

  buf = "Hello World!";
  result = crc_calc(buf, strlen(buf), 0);
  munit_assert_uint8(result, ==, 0x1C);

  buf = "Hello WOrld!";
  munit_assert_uint8(crc_calc(buf, strlen(buf), result), !=, 0);

  return MUNIT_OK;
}

TEST(test_comments) {
  char *buf;
  code_t code;
  int result;

  buf = "  D6\r\n";
  result = code_parse(&code, buf, strlen(buf));
  munit_assert_int(result, ==, 6);
  free_code(&code);

  buf = "\t\xD3\x02\x8E\xD3\x04\x00\x59 \nG2\n";
  result = code_parse(&code, buf, 13);
  munit_assert_int(result, ==, 10);
  free_code(&code);

  buf = "    \t;D6\n";
  result = code_parse(&code, buf, strlen(buf));
  munit_assert_int(result, ==, SCODE_ERROR_EMPTY);
  free_code(&code);

  buf = "    \n;D6\n";
  result = code_parse(&code, buf, strlen(buf));
  munit_assert_int(result, ==, SCODE_ERROR_EMPTY);
  free_code(&code);

  buf = "D6;\nHello World!";
  result = code_parse(&code, buf, strlen(buf));
  munit_assert_int(result, ==, 4);
  free_code(&code);

  buf = "D6 ;N2\nHello World!";
  result = code_parse(&code, buf, strlen(buf));
  munit_assert_int(result, ==, 7);
  munit_assert_null(code.params);
  free_code(&code);

  buf = "D6 ;N2 ;\nHello World!";
  result = code_parse(&code, buf, strlen(buf));
  munit_assert_int(result, ==, 9);
  munit_assert_null(code.params);
  free_code(&code);

  buf = "\t;\xD3\x02\x8E\xD3\x04\x00\x10\n";
  result = code_parse(&code, buf, 10);
  munit_assert_int(result, ==, SCODE_ERROR_EMPTY);
  free_code(&code);

  return MUNIT_OK;
}

TEST(test_code_stream) {
  code_stream_t cs = init_code_stream(0);
  code_t cmd;
  char *buf;

  munit_assert_int(code_stream_pop(&cs, &cmd), ==, SCODE_ERROR_BUFFER);

  buf = "G4 G2\n";
  code_stream_update(&cs, buf, strlen(buf));
  munit_assert_int(code_stream_pop(&cs, &cmd), ==, 0);
  munit_assert_char(code_letter(&cmd), ==, 'G');
  munit_assert_uint8(cmd.number, ==, 4);
  free_code(&cmd);

  buf = "\n      \n";
  code_stream_update(&cs, buf, strlen(buf));
  munit_assert_int(code_stream_pop(&cs, &cmd), ==, SCODE_ERROR_BUFFER);

  buf = "\n\nP3\n";
  code_stream_update(&cs, buf, strlen(buf));
  munit_assert_int(code_stream_pop(&cs, &cmd), ==, 0);
  munit_assert_char(code_letter(&cmd), ==, 'P');
  munit_assert_uint8(cmd.number, ==, 3);
  free_code(&cmd);

  buf = "Gg\nG4\n T12";
  code_stream_update(&cs, buf, strlen(buf));

  munit_assert_int(code_stream_pop(&cs, &cmd), ==, SCODE_ERROR_PARSE);
  munit_assert_int(code_stream_pop(&cs, &cmd), ==, 0);
  munit_assert_char(code_letter(&cmd), ==, 'G');
  munit_assert_uint8(cmd.number, ==, 4);
  free_code(&cmd);

  munit_assert_int(code_stream_pop(&cs, &cmd), ==, SCODE_ERROR_BUFFER);

  buf = " G3\n";
  code_stream_update(&cs, buf, strlen(buf));
  munit_assert_int(code_stream_pop(&cs, &cmd), ==, 0);
  munit_assert_char(code_letter(&cmd), ==, 'T');
  munit_assert_uint8(cmd.number, ==, 12);
  free_code(&cmd);

  free_code_stream(&cs);
  return MUNIT_OK;
}

TEST(test_swap_endian) {

  munit_assert_uint16(swap_endian_16(*(uint16_t *)"AB"), ==, *(uint16_t *)"BA");
  munit_assert_uint32(swap_endian_32(*(uint32_t *)"ABCD"), ==,
                      *(uint32_t *)"DCBA");
  munit_assert_uint64(swap_endian_64(*(uint64_t *)"ABCDEFGH"), ==,
                      *(uint64_t *)"HGFEDCBA");

  return MUNIT_OK;
}

static MunitTest test_suite_tests[] = {TEST_ITEM(test_crc),
                                       TEST_ITEM(test_param_init),
                                       TEST_ITEM(test_param_dump_binary),
                                       TEST_ITEM(test_param_dump_human),
                                       TEST_ITEM(test_param_parse_human),
                                       TEST_ITEM(test_param_parse_binary),
                                       TEST_ITEM(test_code_dump_human),
                                       TEST_ITEM(test_code_dump_binary),
                                       TEST_ITEM(test_code_parse_human),
                                       TEST_ITEM(test_code_parse_binary),
                                       TEST_ITEM(test_comments),
                                       TEST_ITEM(test_code_stream),
                                       TEST_ITEM(test_swap_endian),
                                       TEST_NULL};

static const MunitSuite test_suite = {"", test_suite_tests, NULL, 1,
                                      MUNIT_SUITE_OPTION_NONE};
int main(int argc, char **argv) {
  return munit_suite_main(&test_suite, NULL, argc, argv);
}
