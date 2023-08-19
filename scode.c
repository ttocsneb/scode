#include "scode.h"

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#define MIN(a, b) ((a) < (b) ? (a) : (b))

#define BUF_ASSERT_LEN(len, min)                                               \
  if ((len) < (min))                                                           \
    return SCODE_ERROR_BUFFER;
#define BUF_AT(buf, len, index)                                                \
  ({                                                                           \
    if ((len) <= (index))                                                      \
      return SCODE_ERROR_BUFFER;                                               \
    buf[index];                                                                \
  })
#define BUF_SET(buf, len, index, val)                                          \
  if ((len) <= (index))                                                        \
    return SCODE_ERROR_BUFFER;                                                 \
  buf[index] = val

#define UNWRAP(res)                                                            \
  ({                                                                           \
    int val = res;                                                             \
    if (val < 0)                                                               \
      return val;                                                              \
    val;                                                                       \
  })
#define UNWRAP_OR(res, or)                                                     \
  ({                                                                           \
    int val = res;                                                             \
    if (val < 0)                                                               \
      return or ;                                                              \
    val;                                                                       \
  })

#define CRC_TABLE
#ifdef CRC_TABLE
static const uint8_t crc_lookup[256] = {
    0x00, 0x07, 0x0E, 0x09, 0x1C, 0x1B, 0x12, 0x15, 0x38, 0x3F, 0x36, 0x31,
    0x24, 0x23, 0x2A, 0x2D, 0x70, 0x77, 0x7E, 0x79, 0x6C, 0x6B, 0x62, 0x65,
    0x48, 0x4F, 0x46, 0x41, 0x54, 0x53, 0x5A, 0x5D, 0xE0, 0xE7, 0xEE, 0xE9,
    0xFC, 0xFB, 0xF2, 0xF5, 0xD8, 0xDF, 0xD6, 0xD1, 0xC4, 0xC3, 0xCA, 0xCD,
    0x90, 0x97, 0x9E, 0x99, 0x8C, 0x8B, 0x82, 0x85, 0xA8, 0xAF, 0xA6, 0xA1,
    0xB4, 0xB3, 0xBA, 0xBD, 0xC7, 0xC0, 0xC9, 0xCE, 0xDB, 0xDC, 0xD5, 0xD2,
    0xFF, 0xF8, 0xF1, 0xF6, 0xE3, 0xE4, 0xED, 0xEA, 0xB7, 0xB0, 0xB9, 0xBE,
    0xAB, 0xAC, 0xA5, 0xA2, 0x8F, 0x88, 0x81, 0x86, 0x93, 0x94, 0x9D, 0x9A,
    0x27, 0x20, 0x29, 0x2E, 0x3B, 0x3C, 0x35, 0x32, 0x1F, 0x18, 0x11, 0x16,
    0x03, 0x04, 0x0D, 0x0A, 0x57, 0x50, 0x59, 0x5E, 0x4B, 0x4C, 0x45, 0x42,
    0x6F, 0x68, 0x61, 0x66, 0x73, 0x74, 0x7D, 0x7A, 0x89, 0x8E, 0x87, 0x80,
    0x95, 0x92, 0x9B, 0x9C, 0xB1, 0xB6, 0xBF, 0xB8, 0xAD, 0xAA, 0xA3, 0xA4,
    0xF9, 0xFE, 0xF7, 0xF0, 0xE5, 0xE2, 0xEB, 0xEC, 0xC1, 0xC6, 0xCF, 0xC8,
    0xDD, 0xDA, 0xD3, 0xD4, 0x69, 0x6E, 0x67, 0x60, 0x75, 0x72, 0x7B, 0x7C,
    0x51, 0x56, 0x5F, 0x58, 0x4D, 0x4A, 0x43, 0x44, 0x19, 0x1E, 0x17, 0x10,
    0x05, 0x02, 0x0B, 0x0C, 0x21, 0x26, 0x2F, 0x28, 0x3D, 0x3A, 0x33, 0x34,
    0x4E, 0x49, 0x40, 0x47, 0x52, 0x55, 0x5C, 0x5B, 0x76, 0x71, 0x78, 0x7F,
    0x6A, 0x6D, 0x64, 0x63, 0x3E, 0x39, 0x30, 0x37, 0x22, 0x25, 0x2C, 0x2B,
    0x06, 0x01, 0x08, 0x0F, 0x1A, 0x1D, 0x14, 0x13, 0xAE, 0xA9, 0xA0, 0xA7,
    0xB2, 0xB5, 0xBC, 0xBB, 0x96, 0x91, 0x98, 0x9F, 0x8A, 0x8D, 0x84, 0x83,
    0xDE, 0xD9, 0xD0, 0xD7, 0xC2, 0xC5, 0xCC, 0xCB, 0xE6, 0xE1, 0xE8, 0xEF,
    0xFA, 0xFD, 0xF4, 0xF3,
};

uint8_t crc_calc(const char *buf, size_t len, uint8_t crc) {
  uint8_t val = 0;
  for (size_t i = 0; i < len; ++i) {
    val = crc_lookup[val ^ ((uint8_t)buf[i])];
  }
  return val ^ crc;
}

#else

uint8_t crc_calc(const char *buf, size_t len, uint8_t crc) {
#define DIVISOR(pos) (0x107 << ((pos)-8))

  uint16_t data = buf[0] & 0x00FF;
  size_t pos = 1;
  size_t i = 15;

  while (1) {
    // Shift the data by one byte or return the final crc
    if ((data & 0xFF00) == 0) {
      if (pos == len) {
        data = (data << 8) | crc;
      } else if (pos > len) {
        return data & 0x00FF;
      } else {
        data = (data << 8) | (buf[pos] & 0x00FF);
      }
      pos++;
      i = 15;
    }
    // Perform the next divide
    if ((data & (1 << i)) != 0) {
      data = data ^ DIVISOR(i);
    }
    i--;
  }
}

#endif

uint16_t swap_endian_16(uint16_t i) {
  return ((i & 0xFF00) >> 8) | ((i & 0x00FF) << 8);
}

uint32_t swap_endian_32(uint32_t i) {
  uint32_t a = ((uint32_t)swap_endian_16((i & 0xFFFF0000) >> 16));
  uint32_t b = (((uint32_t)swap_endian_16(i & 0x0000FFFF)) << 16);
  return a | b;
}

uint64_t swap_endian_64(uint64_t i) {
  uint64_t a = ((uint64_t)swap_endian_32((i & 0xFFFFFFFF00000000) >> 32));
  uint64_t b = (((uint64_t)swap_endian_32(i & 0x00000000FFFFFFFF)) << 32);
  return a | b;
}

////////////////////////////////////////////////////////////////////////////////

param_t init_param_u8(char param, uint8_t val) {
  param_t p;
  p.param = (param & 0b00011111) | (PARAM_T_U8 << 5);
  p.u8 = val;
  return p;
}

param_t init_param_i8(char param, int8_t val) {
  param_t p;
  p.param = (param & 0b00011111) | (PARAM_T_I8 << 5);
  p.i8 = val;
  return p;
}

param_t init_param_i16(char param, int16_t val) {
  param_t p;
  p.param = (param & 0b00011111) | (PARAM_T_I16 << 5);
  p.i16 = val;
  return p;
}

param_t init_param_i32(char param, int32_t val) {
  param_t p;
  p.param = (param & 0b00011111) | (PARAM_T_I32 << 5);
  p.i32 = val;
  return p;
}

param_t init_param_i64(char param, int64_t val) {
  param_t p;
  p.param = (param & 0b00011111) | (PARAM_T_I64 << 5);
  p.i64 = val;
  return p;
}

param_t init_param_f32(char param, float val) {
  param_t p;
  p.param = (param & 0b00011111) | (PARAM_T_F32 << 5);
  p.f32 = val;
  return p;
}

param_t init_param_f64(char param, double val) {
  param_t p;
  p.param = (param & 0b00011111) | (PARAM_T_F64 << 5);
  p.f64 = val;
  return p;
}

param_t init_param_str(char param, const char *val) {
  param_t p;
  p.param = (param & 0b00011111) | (PARAM_T_STR << 5);
  p.str = malloc(strlen(val) + 1);
  strcpy(p.str, val);
  return p;
}

#define CAST_PARAM                                                             \
  switch (param_type(self)) {                                                  \
  case PARAM_T_I8:                                                             \
    return self->i8;                                                           \
  case PARAM_T_U8:                                                             \
    return self->u8;                                                           \
  case PARAM_T_I16:                                                            \
    return self->i16;                                                          \
  case PARAM_T_I32:                                                            \
    return self->i32;                                                          \
  case PARAM_T_I64:                                                            \
    return self->i64;                                                          \
  case PARAM_T_F32:                                                            \
    return self->f32;                                                          \
  case PARAM_T_F64:                                                            \
    return self->f64;                                                          \
  }                                                                            \
  return 0

uint8_t param_cast_u8(const param_t *self) { CAST_PARAM; }
int8_t param_cast_i8(const param_t *self) { CAST_PARAM; }
int16_t param_cast_i16(const param_t *self) { CAST_PARAM; }
int32_t param_cast_i32(const param_t *self) { CAST_PARAM; }
int64_t param_cast_i64(const param_t *self) { CAST_PARAM; }
float param_cast_f32(const param_t *self) { CAST_PARAM; }
double param_cast_f64(const param_t *self) { CAST_PARAM; }

uint8_t param_type(const param_t *self) { return (self->param >> 5) & 0b111; }

char param_letter(const param_t *self) {
  return (self->param & 0b00011111) | 0b01000000;
}

void free_param(param_t *self) {
  if (param_type(self) == PARAM_T_STR && self->str != NULL) {
    free(self->str);
    self->str = NULL;
  }
  self->param = 0;
}

int param_parse_binary(param_t *self, const char *buf, size_t len) {
  int read = 0;
  self->param = BUF_AT(buf, len, 0);
  char l = param_letter(self);

  if (l < 'A' || l > 'Z') {
    return SCODE_ERROR_PARSE;
  }

  switch (param_type(self)) {
  case PARAM_T_U8:
  case PARAM_T_I8:
    self->u8 = BUF_AT(buf, len, 1);
    read = 2;
    break;
  case PARAM_T_I16:
    BUF_ASSERT_LEN(len, 3);
    read = 3;
    self->i16 = ltoh16(*(uint16_t *)&buf[1]);
    break;
  case PARAM_T_I32:
  case PARAM_T_F32:
    BUF_ASSERT_LEN(len, 5);
    read = 5;
    self->i32 = ltoh32(*(uint32_t *)&buf[1]);
    break;
  case PARAM_T_I64:
  case PARAM_T_F64:
    BUF_ASSERT_LEN(len, 9);
    read = 9;
    self->i64 = ltoh64(*(uint64_t *)&buf[1]);
    break;
  case PARAM_T_STR: {
    size_t length = 0;
    for (size_t i = 1; i < len; ++i) {
      if (buf[i] == '\0') {
        break;
      }
      length++;
    }
    self->str = malloc(length + 1);
    memcpy(self->str, &buf[1], length);
    self->str[length] = '\0';
    read = 2 + length;
    break;
  }
  }
  return read;
}

void set_type(param_t *self, uint8_t type) {
  self->param = (self->param & 0b00011111) | (type << 5);
}

int parse_string(param_t *self, const char *buf, size_t len) {
  char quote = BUF_AT(buf, len, 0);
  if (quote != '"' && quote != '\'') {
    return 0;
  }
  size_t length = 0;
  while (BUF_AT(buf, len, length + 1) != quote) {
    length++;
  }

  self->str = malloc(length + 1);
  memcpy(self->str, &buf[1], length);
  self->str[length] = '\0';
  set_type(self, PARAM_T_STR);
  return length + 2;
}

int parse_number(param_t *self, const char *buf, size_t len) {
  size_t pos = 0;
  int is_negative = BUF_AT(buf, len, 0) == '-';
  if (is_negative) {
    pos++;
  }

  int digits = 0;
  int precision = 0;

  /*
   * 0001 => 1
   * 100000 => 1
   * 100001 => 6
   */

  int64_t val = 0;
  while (1) {
    if (len <= pos) {
      break;
    }
    char c = buf[pos];
    if (!isdigit(c)) {
      break;
    }
    uint8_t v = c - '0';
    val = val * 10 + v;
    pos++;
    if (digits > 0 || v != 0) {
      digits++;
    }
    if (v != 0) {
      precision = digits;
    }
  }

  if (buf[pos] == '.') {
    double f64 = (double)val;
    pos++;

    uint32_t divisor = 10;
    while (1) {
      if (len <= pos) {
        break;
      }
      char c = buf[pos];
      if (!isdigit(c)) {
        break;
      }
      double v = c - '0';
      f64 += v / divisor;
      divisor *= 10;
      pos++;
      digits++;
      if (v != 0) {
        precision = digits;
      }
    }
    // Store as a float if less than 7 digits of precision are used.
    if (precision <= 7) {
      set_type(self, PARAM_T_F32);
      self->f32 = (float)f64;
    } else {
      set_type(self, PARAM_T_F64);
      self->f64 = f64;
    }
  } else {
    if (is_negative) {
      val = -val;
    }
    if (val >= 0 && val <= UINT8_MAX) {
      set_type(self, PARAM_T_U8);
      self->u8 = (uint8_t)val;
    } else if (val >= INT8_MIN && val <= INT8_MAX) {
      set_type(self, PARAM_T_I8);
      self->i8 = (int8_t)val;
    } else if (val >= INT16_MIN && val <= INT16_MAX) {
      set_type(self, PARAM_T_I16);
      self->i16 = (int16_t)val;
    } else if (val >= INT32_MIN && val <= INT32_MAX) {
      set_type(self, PARAM_T_I32);
      self->i32 = (int32_t)val;
    } else {
      set_type(self, PARAM_T_I64);
      self->i64 = (int64_t)val;
    }
  }

  return pos;
}

int param_parse_human(param_t *self, const char *buf, size_t len) {
  self->param = toupper(BUF_AT(buf, len, 0));
  if (self->param > 'Z' || self->param < 'A') {
    return SCODE_ERROR_PARSE;
  }
  int res = UNWRAP(parse_string(self, buf + 1, len - 1));
  if (res > 0) {
    return res + 1;
  }
  res = UNWRAP(parse_number(self, buf + 1, len - 1));
  if (res == 0) {
    return SCODE_ERROR_PARSE;
  }
  return res + 1;
}

int param_dump_binary(const param_t *self, char *buf, size_t len) {
  BUF_ASSERT_LEN(len, 1);
  buf[0] = self->param;
  switch (param_type(self)) {
  case PARAM_T_U8:
  case PARAM_T_I8:
    BUF_ASSERT_LEN(len, 2);
    memcpy(&buf[1], &self->u8, 1);
    return 2;
  case PARAM_T_I16:
    BUF_ASSERT_LEN(len, 3);
    uint16_t u16 = htol16(self->i16);
    memcpy(&buf[1], &u16, 2);
    return 3;
  case PARAM_T_I32:
  case PARAM_T_F32:
    BUF_ASSERT_LEN(len, 5);
    uint32_t u32 = htol32(self->i32);
    memcpy(&buf[1], &u32, 4);
    return 5;
  case PARAM_T_I64:
  case PARAM_T_F64:
    BUF_ASSERT_LEN(len, 9);
    uint64_t u64 = htol64(self->i64);
    memcpy(&buf[1], &u64, 8);
    return 9;
  case PARAM_T_STR: {
    uint8_t length = (uint8_t)strlen(self->str);
    BUF_ASSERT_LEN(len, length + 2);
    memcpy(&buf[1], self->str, length);
    buf[1 + length] = '\0';
    return length + 2;
  }
  }
  return 0;
}

int format_int(int64_t value, char *buf, size_t len) {
  size_t pos = 0;
  int is_negative = value < 0;
  if (is_negative) {
    value = -value;
    BUF_ASSERT_LEN(len, pos + 1);
    buf[pos++] = '-';
  }
  char temp[20];
  int i = 0;
  while (value > 0) {
    temp[i] = (value % 10) + '0';
    value /= 10;
    i++;
  }
  if (i == 0) {
    temp[i++] = '0';
  }
  BUF_ASSERT_LEN(len, i + pos);

  for (--i; i >= 0; --i) {
    buf[pos++] = temp[i];
  }
  return pos;
}

int param_dump_human(const param_t *self, char *buf, size_t len) {
  char letter = param_letter(self);
  BUF_ASSERT_LEN(len, 1);
  buf[0] = letter;
  size_t pos = 1;

  uint8_t type = param_type(self);
  if (type == PARAM_T_STR) {
    // dump string
    size_t length = strlen(self->str);
    int contains_single_quote = 0;
    int contains_double_quote = 0;
    for (size_t i = 0; i < length; ++i) {
      if (self->str[i] == '\'') {
        contains_single_quote = 1;
      }
      if (self->str[i] == '"') {
        contains_double_quote = 1;
      }
      if (contains_single_quote && contains_double_quote) {
        return SCODE_ERROR_DUMP;
      }
    }
    char quote = contains_single_quote ? '"' : '\'';
    BUF_ASSERT_LEN(len, length + 2 + pos);
    buf[pos++] = quote;
    memcpy(&buf[pos], self->str, length);
    pos += length;
    buf[pos++] = quote;
  } else if (type == PARAM_T_F32 || type == PARAM_T_F64) {
    // dump float
    double value = param_cast_f64(self);
    if (value < 0) {
      BUF_ASSERT_LEN(len, pos + 1);
      buf[pos++] = '-';
      value = -value;
    }
    int64_t base = (int64_t)value;
    value = value - base;
    if (value > 0.9999) {
      base += 1;
      value -= 0.9999;
    }
    pos += UNWRAP(format_int(base, buf + pos, len - pos));

    BUF_ASSERT_LEN(len, pos + 1);
    buf[pos++] = '.';
    if (value < 0.0001) {
      BUF_ASSERT_LEN(len, pos + 1);
      buf[pos++] = '0';
    }

    while (value > 0.0001) {
      value *= 10;
      uint8_t digit = (uint8_t)value;
      value -= digit;
      if (value > 0.9999 && digit < 9) {
        digit += 1;
        value -= 0.9999;
      }
      BUF_ASSERT_LEN(len, pos + 1);
      buf[pos++] = digit + '0';
    }
  } else {
    // dump number
    int64_t value = param_cast_i64(self);
    pos += UNWRAP(format_int(value, buf + pos, len - pos));
  }
  return pos;
}

////////////////////////////////////////////////////////////////////////////////

code_t init_code(char letter, uint8_t number, size_t num_params) {
  code_t self;
  self.category = (letter & 0b00011111) | 0b11000000;
  self.number = number;
  if (num_params == 0) {
    self.params = NULL;
  } else {
    self.params = malloc(sizeof(param_t) * (num_params + 1));
    self.params[num_params].param = 0;
    self.params[num_params].str = NULL;
  }
  return self;
}

void free_code(code_t *self) {
  if (self->params != NULL) {
    for (int i = 0; self->params[i].param != 0; ++i) {
      free_param(&self->params[i]);
    }
    free(self->params);
    self->params = NULL;
  }
}

struct param_list {
  param_t param;
  struct param_list *next;
};

void free_param_list(struct param_list *self) {
  while (self != NULL) {
    struct param_list *temp = self;
    free_param(&self->param);
    self = self->next;
    free(temp);
  }
}

int code_parse_args_human(struct param_list **params, size_t *param_len,
                          const char *buf, size_t len) {
  struct param_list *last_param = *params;
  while (last_param != NULL && last_param->next != NULL) {
    last_param = last_param->next;
  }
  size_t pos = 0;

  while (1) {
    if (len <= pos) {
      break;
    }
    char next = buf[pos];
    if (isspace(next)) {
      pos++;
      if (next == '\n') {
        break;
      }
      if (next == '\r') {
        break;
      }
      continue;
    }
    param_t param;
    pos += UNWRAP(param_parse_human(&param, buf + pos, len - pos));
    struct param_list *new_param = malloc(sizeof(struct param_list));
    memcpy(&new_param->param, &param, sizeof(param_t));
    new_param->next = NULL;
    if (last_param == NULL) {
      *params = new_param;
      last_param = new_param;
    } else {
      last_param->next = new_param;
      last_param = new_param;
    }
    (*param_len)++;
  }

  return pos;
}

int code_parse_args_binary(struct param_list **params, size_t *param_len,
                           uint8_t *crc, const char *buf, size_t len) {
  struct param_list *last_param = *params;
  while (last_param != NULL && last_param->next != NULL) {
    last_param = last_param->next;
  }
  size_t pos = 0;

  while (1) {
    uint8_t next = BUF_AT(buf, len, pos);
    if (next == '\0') {
      pos++;
      break;
    }
    param_t param;
    pos += UNWRAP(param_parse_binary(&param, buf + pos, len - pos));
    struct param_list *new_param = malloc(sizeof(struct param_list));
    memcpy(&new_param->param, &param, sizeof(param_t));
    new_param->next = NULL;
    if (last_param == NULL) {
      *params = new_param;
      last_param = new_param;
    } else {
      last_param->next = new_param;
      last_param = new_param;
    }
    (*param_len)++;
  }
  *crc = BUF_AT(buf, len, pos);
  pos++;

  return pos;
}

int code_parse(code_t *self, const char *buf, size_t len) {
  param_t code = {0};
  size_t pos = 0;
  struct param_list *params = NULL;
  size_t params_len = 0;

  while (pos < len) {
    if (!isspace(buf[pos]) || buf[pos] == '\n' || buf[pos] == '\r') {
      break;
    }
    pos++;
  }
  uint8_t first = BUF_AT(buf, len, pos);

  if (first & 0x80) {
    size_t start = pos;
    uint8_t crc;
    pos += UNWRAP(param_parse_binary(&code, buf + pos, len - pos));
    int res = code_parse_args_binary(&params, &params_len, &crc, buf + pos,
                                     len - pos);

    if (res < 0) {
      free_param(&code);
      free_param_list(params);
      return res;
    }
    pos += res;

    if (crc_calc(buf + start, pos - 2 - start, crc) != 0) {
      free_param(&code);
      free_param_list(params);
      return SCODE_ERROR_CRC;
    }

    self->category = first;

    while (pos < len) {
      if (!isspace(buf[pos])) {
        break;
      }
      pos++;
    }

  } else {
    // Find a comment if it exists
    size_t comment = len;
    size_t eol = SIZE_MAX;
    size_t eoc = SIZE_MAX;
    for (size_t i = pos; i < len; ++i) {
      if (buf[i] == ';' && comment > i) {
        comment = i;
      } else if (buf[i] == '\n' || buf[i] == '\r') {
        eol = i + 1;
        eoc = i;
        break;
      }
    }

    if (eoc >= len) {
      return SCODE_ERROR_BUFFER;
    }

    len = MIN(comment, eoc);

    if (pos - len == 0) {
      return SCODE_ERROR_EMPTY;
    }

    pos += UNWRAP(param_parse_human(&code, buf + pos, len - pos));
    int res = code_parse_args_human(&params, &params_len, buf + pos, len - pos);

    if (res < 0) {
      free_param(&code);
      free_param_list(params);
      return res;
    }
    pos += res;

    self->category = param_letter(&code);
    pos = eol;
  }

  self->number = param_cast_u8(&code);
  free_param(&code);

  param_t *param_arr = NULL;
  if (params_len > 0) {
    param_arr = malloc(sizeof(param_t) * (params_len + 1));
    struct param_list *item = params;
    int i = 0;
    while (item != NULL) {
      memcpy(&param_arr[i], &item->param, sizeof(param_t));
      struct param_list *temp = item;
      item = item->next;
      free(temp);
      i++;
    }
    param_arr[params_len].param = 0;
    param_arr[params_len].str = NULL;
  }
  self->params = param_arr;

  return pos;
}

int code_dump_binary(const code_t *self, char *buf, size_t len) {
  size_t pos = 0;
  param_t start = init_param_u8(self->category, self->number);
  pos += UNWRAP(param_dump_binary(&start, buf, len));
  for (int i = 0; self->params != NULL && self->params[i].param != 0; ++i) {
    pos += UNWRAP(param_dump_binary(&self->params[i], buf + pos, len - pos));
  }
  uint8_t crc = crc_calc(buf, pos, 0);
  BUF_SET(buf, len, pos, '\0');
  pos++;
  BUF_SET(buf, len, pos, crc);
  pos++;

  return pos;
}

int code_dump_human(const code_t *self, char *buf, size_t len) {
  size_t pos = 0;
  param_t start = init_param_u8(self->category, self->number);
  pos += UNWRAP(param_dump_human(&start, buf, len));
  BUF_SET(buf, len, pos, ' ');
  pos++;
  for (int i = 0; self->params != NULL && self->params[i].param != 0; ++i) {
    pos += UNWRAP(param_dump_human(&self->params[i], buf + pos, len - pos));
    BUF_SET(buf, len, pos, ' ');
    pos++;
  }
  buf[pos - 1] = '\r';
  BUF_SET(buf, len, pos, '\n');
  pos++;
  buf[pos] = '\0';
  return pos;
}

char code_letter(const code_t *self) {
  return (self->category & 0b00011111) | 0b01000000;
}

int code_is_binary(const code_t *self) { return (self->category & 0x80) != 0; }

////////////////////////////////////////////////////////////////////////////////

void free_code_stream(code_stream_t *self) {
  if (self->buf != NULL) {
    free(self->buf);
    self->buf = NULL;
  }
}

code_stream_t init_code_stream(size_t capacity) {
  code_stream_t stream;
  stream.end = 0;
  stream.pos = 0;
  stream.cap = capacity;
  if (capacity > 0) {
    stream.buf = malloc(capacity);
  } else {
    stream.buf = NULL;
  }
  return stream;
}

void code_stream_update(code_stream_t *self, const char *buf, size_t len) {
  size_t new_end = self->end + len;
  if (new_end > self->cap) {
    // Not enough space
    if (new_end - self->pos <= self->cap) {
      // Move around data
      memmove(self->buf, &self->buf[self->pos], self->end - self->pos);
      self->end = self->end - self->pos;
      self->pos = 0;
    } else {
      // Allocate some new data
      char *new_buf = malloc(new_end - self->pos);
      if (self->buf != NULL) {
        memcpy(new_buf, &self->buf[self->pos], self->end - self->pos);
        free(self->buf);
      }
      self->buf = new_buf;
      self->cap = new_end - self->pos;
      self->end = self->end - self->pos;
      self->pos = 0;
    }
  }

  memcpy(&self->buf[self->end], buf, len);
  self->end += len;
}

int code_stream_pop(code_stream_t *self, code_t *code) {
  if (self->buf == NULL) {
    return SCODE_ERROR_BUFFER;
  }
  int result = code_parse(code, &self->buf[self->pos], self->end - self->pos);
  if (result > 0) {
    self->pos += result;
    return 0;
  }
  // Skip the code if an error occurred (except for buffer errors)
  // This will not guarentee that the code is skipped, but that should be
  // okay since if it is not, then the "next" code should be invalid.
  switch (result) {
  case SCODE_ERROR_PARSE:
  case SCODE_ERROR_DUMP:
    // Skip until after null termination
    while (self->pos < self->end) {
      char c = self->buf[self->pos++];
      if (c == '\0') {
        if (self->pos < self->end) {
          self->pos++;
        }
        break;
      }
      if (c == '\r') {
        break;
      }
      if (c == '\n') {
        break;
      }
    }
    break;
  case SCODE_ERROR_CRC:
    // Skip until after null termination
    while (self->pos < self->end) {
      char c = self->buf[self->pos++];
      if (c == '\0') {
        if (self->pos < self->end) {
          self->pos++;
        }
        break;
      }
    }
    break;
  case SCODE_ERROR_EMPTY:
    // Skip until newline and try again
    while (self->pos < self->end) {
      char c = self->buf[self->pos++];
      if (c == '\n') {
        break;
      }
      if (c == '\r') {
        break;
      }
    }
    return code_stream_pop(self, code);
  case SCODE_ERROR_BUFFER:
    break;
  }

  return result;
}
