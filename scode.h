#pragma once

#include <stddef.h>
#include <stdint.h>

#if defined(__cplusplus)
extern "C" {
#endif

#define BYTEORDER_LITTLE_ENDIAN 0 // Little endian machine.
#define BYTEORDER_BIG_ENDIAN 1    // Big endian machine.

// #define BYTEORDER_ENDIAN BYTEORDER_LITTLE_ENDIAN

#ifndef BYTEORDER_ENDIAN
// Detect with GCC 4.6's macro.
#if defined(__BYTE_ORDER__)
#if (__BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__)
#define BYTEORDER_ENDIAN BYTEORDER_LITTLE_ENDIAN
#elif (__BYTE_ORDER__ == __ORDER_BIG_ENDIAN__)
#define BYTEORDER_ENDIAN BYTEORDER_BIG_ENDIAN
#else
#error                                                                         \
    "Unknown machine byteorder endianness detected. User needs to define BYTEORDER_ENDIAN."
#endif
// Detect with GLIBC's endian.h.
#elif defined(__GLIBC__)
#include <endian.h>
#if (__BYTE_ORDER == __LITTLE_ENDIAN)
#define BYTEORDER_ENDIAN BYTEORDER_LITTLE_ENDIAN
#elif (__BYTE_ORDER == __BIG_ENDIAN)
#define BYTEORDER_ENDIAN BYTEORDER_BIG_ENDIAN
#else
#error                                                                         \
    "Unknown machine byteorder endianness detected. User needs to define BYTEORDER_ENDIAN."
#endif
// Detect with _LITTLE_ENDIAN and _BIG_ENDIAN macro.
#elif defined(_LITTLE_ENDIAN) && !defined(_BIG_ENDIAN)
#define BYTEORDER_ENDIAN BYTEORDER_LITTLE_ENDIAN
#elif defined(_BIG_ENDIAN) && !defined(_LITTLE_ENDIAN)
#define BYTEORDER_ENDIAN BYTEORDER_BIG_ENDIAN
// Detect with architecture macros.
#elif defined(__sparc) || defined(__sparc__) || defined(_POWER) ||             \
    defined(__powerpc__) || defined(__ppc__) || defined(__hpux) ||             \
    defined(__hppa) || defined(_MIPSEB) || defined(_POWER) ||                  \
    defined(__s390__)
#define BYTEORDER_ENDIAN BYTEORDER_BIG_ENDIAN
#elif defined(__i386__) || defined(__alpha__) || defined(__ia64) ||            \
    defined(__ia64__) || defined(_M_IX86) || defined(_M_IA64) ||               \
    defined(_M_ALPHA) || defined(__amd64) || defined(__amd64__) ||             \
    defined(_M_AMD64) || defined(__x86_64) || defined(__x86_64__) ||           \
    defined(_M_X64) || defined(__bfin__)
#define BYTEORDER_ENDIAN BYTEORDER_LITTLE_ENDIAN
#elif defined(_MSC_VER) && (defined(_M_ARM) || defined(_M_ARM64))
#define BYTEORDER_ENDIAN BYTEORDER_LITTLE_ENDIAN
#else
#error                                                                         \
    "Unknown machine byteorder endianness detected. User needs to define BYTEORDER_ENDIAN."
#endif
#endif

uint16_t swap_endian_16(uint16_t i);
uint32_t swap_endian_32(uint32_t i);
uint64_t swap_endian_64(uint64_t i);

#if BYTEORDER_ENDIAN == BYTEORDER_BIG_ENDIAN

#define htol16(x) swap_endian_16(x)
#define htol32(x) swap_endian_32(x)
#define htol64(x) swap_endian_64(x)

#define ltoh16(x) swap_endian_16(x)
#define ltoh32(x) swap_endian_32(x)
#define ltoh64(x) swap_endian_64(x)

#else

#define htol16(x) ((uint16_t)(x))
#define htol32(x) ((uint32_t)(x))
#define htol64(x) ((uint64_t)(x))

#define ltoh16(x) ((uint16_t)(x))
#define ltoh32(x) ((uint32_t)(x))
#define ltoh64(x) ((uint64_t)(x))

#endif

/**
 * Run a CRC-8 algorithm on the provided buffer. If you are trying to create a
 * CRC, then set crc to 0. If you are trying to check a CRC, then set crc to the
 * CRC.
 *
 * @param buf buffer
 * @param len length of buffer
 * @param crc crc input
 *
 * @return crc
 */
uint8_t crc_calc(const char *buf, size_t len, uint8_t crc);

typedef struct {
  union {
    uint8_t u8;
    int8_t i8;
    int16_t i16;
    int32_t i32;
    int64_t i64;
    float f32;
    double f64;
    char *str;
  };
  uint8_t param;
} param_t;

#define PARAM_T_STR 0b111
#define PARAM_T_U8 0b110
#define PARAM_T_I8 0b101
#define PARAM_T_I16 0b100
#define PARAM_T_I32 0b011
#define PARAM_T_I64 0b010
#define PARAM_T_F32 0b001
#define PARAM_T_F64 0b000

#define SCODE_ERROR_PARSE -2
#define SCODE_ERROR_DUMP -3
#define SCODE_ERROR_BUFFER -4
#define SCODE_ERROR_CRC -5
#define SCODE_ERROR_EMPTY -6

/**
 * Initialize u8 parameter
 *
 * @param param parameter letter
 * @param val uint8_t value
 *
 * @return initialized parameter
 */
param_t init_param_u8(char param, uint8_t val);
/**
 * Initialize i8 parameter
 *
 * @param param parameter letter
 * @param val int8_t value
 *
 * @return initialized parameter
 */
param_t init_param_i8(char param, int8_t val);
/**
 * Initialize i16 parameter
 *
 * @param param parameter letter
 * @param val int16_t value
 *
 * @return initialized parameter
 */
param_t init_param_i16(char param, int16_t val);
/**
 * Initialize i32 parameter
 *
 * @param param parameter letter
 * @param val uint32_t value
 *
 * @return initialized parameter
 */
param_t init_param_i32(char param, int32_t val);
/**
 * Initialize i64 parameter
 *
 * @param param parameter letter
 * @param val uint64_t value
 *
 * @return initialized parameter
 */
param_t init_param_i64(char param, int64_t val);
/**
 * Initialize f32 parameter
 *
 * @param param parameter letter
 * @param val float value
 *
 * @return initialized parameter
 */
param_t init_param_f32(char param, float val);
/**
 * Initialize f64 parameter
 *
 * @param param parameter letter
 * @param val double value
 *
 * @return initialized parameter
 */
param_t init_param_f64(char param, double val);
/**
 * Initialize string parameter
 *
 * This will create new data on the heap, so free_param() should be called after
 * use
 *
 * @param param parameter letter
 * @param val string value
 *
 * @return initialized parameter
 */
param_t init_param_str(char param, const char *val);

/**
 * Cast the value into a u8
 *
 * @return uint8_t version of value
 */
uint8_t param_cast_u8(const param_t *self);
/**
 * Cast the value into an i8
 *
 * @return int8_t version of value
 */
int8_t param_cast_i8(const param_t *self);
/**
 * Cast the value into an i16
 *
 * @return int16_t version of value
 */
int16_t param_cast_i16(const param_t *self);
/**
 * Cast the value into an i32
 *
 * @return int32_t version of value
 */
int32_t param_cast_i32(const param_t *self);
/**
 * Cast the value into an i64
 *
 * @return int64_t version of value
 */
int64_t param_cast_i64(const param_t *self);
/**
 * Cast the value into an f32
 *
 * @return int32_t version of value
 */
float param_cast_f32(const param_t *self);
/**
 * Cast the value into an f64
 *
 * @return int64_t version of value
 */
double param_cast_f64(const param_t *self);

/**
 * Get the value type of this parameter
 *
 * @return parameter type
 */
uint8_t param_type(const param_t *self);

/**
 * Get the letter that this param represents
 */
char param_letter(const param_t *self);

/**
 * Free any heap memory
 */
void free_param(param_t *self);

/**
 * Parse a binary parameter
 *
 * @param buf buffer
 * @param len length of buffer
 *
 * @return any error codes
 */
int param_parse_binary(param_t *self, const char *buf, size_t len);

/**
 * Parse a human readable parameter
 *
 * @param buf buffer
 * @param len length of buffer
 *
 * @return any error codes
 */
int param_parse_human(param_t *self, const char *buf, size_t len);

/**
 * Dump the parameter as a binary parameter
 *
 * @param buf buffer
 * @param len length of buffer
 *
 * @return number of bytes written
 */
int param_dump_binary(const param_t *self, char *buf, size_t len);
/**
 * Dump the parameter as a human readable parameter
 *
 * @param buf buffer
 * @param len length of buffer
 *
 * @return number of bytes written
 */
int param_dump_human(const param_t *self, char *buf, size_t len);

typedef struct {
  param_t *params;
  uint8_t category;
  uint8_t number;
} code_t;

/**
 * Initialize a new code
 *
 * This will create a new code with the number of parameters allocated. An extra
 * NULL param will be added for you so you don't have to set it.
 *
 * @param letter code letter
 * @param number code number
 * @param num_params The number of params in this code
 *
 * @return initialized code
 */
code_t init_code(char letter, uint8_t number, size_t num_params);

/**
 * Free the code from memory
 *
 * This will recursively free any parameters in this code as well, so you don't
 * have to
 */
void free_code(code_t *self);

/**
 * Parse a code string into a code object
 *
 * @param buf buffer to parse
 * @param len length of buffer
 *
 * @return number of bytes parsed or one of the SCODE_ERROR_X errors
 *
 * If an error occurs, then the code does not need to freed.
 */
int code_parse(code_t *self, const char *buf, size_t len);

/**
 * Dump the code object into a human code string.
 *
 * No null terminator will be added to the buffer
 *
 * @param buf buffer to write to
 * @param len maximum length of the buffer
 *
 * @return number of bytes written or one of the SCODE_ERROR_X errors
 */
int code_dump_binary(const code_t *self, char *buf, size_t len);
/**
 * Dump the code object into a binary code string.
 *
 * This will append a null terminator to the end of the string (not included in
 * the return number)
 *
 * @param buf buffer to write to
 * @param len maximum length of the buffer
 *
 * @return number of bytes written or one of the SCODE_ERROR_X errors
 */
int code_dump_human(const code_t *self, char *buf, size_t len);

/**
 * Get the letter that this code uses
 *
 * @return letter
 */
char code_letter(const code_t *self);
/**
 * Check if this code was in binary form when it was parsed.
 *
 * This will always return true if the code was created with init_code()
 *
 * @return whether the parsed code was binary
 */
int code_is_binary(const code_t *self);

typedef struct {
  size_t end;
  size_t pos;
  size_t cap;
  char *buf;
} code_stream_t;

/**
 * Free code stream
 */
void free_code_stream(code_stream_t *self);
/**
 * Initialize a new code stream
 *
 * @param capacity initial capacity of buffer
 *
 * @return new code stream
 */
code_stream_t init_code_stream(size_t capacity);

/**
 * Add data to the input buffer and parse any ready codes
 *
 * @param buf input buffer
 * @param len length of buffer
 */
void code_stream_update(code_stream_t *self, const char *buf, size_t len);
/**
 * Pop the next available code from the buffer.
 *
 * @param code code to populate
 *
 * @return 0 for success, below zero for an error.
 *
 * If a code could not be parsed, it is still popped.
 */
int code_stream_pop(code_stream_t *self, code_t *code);

#if defined(__cplusplus)
}

class Param {
public:
  param_t param;

  Param() : param({0}) {}
  Param(param_t &&other) : param(other) {
    other.str = nullptr;
    other.param = 0;
  }

  Param(char param, uint8_t val) : param(init_param_u8(param, val)) {}
  Param(char param, int8_t val) : param(init_param_i8(param, val)) {}
  Param(char param, int16_t val) : param(init_param_i16(param, val)) {}
  Param(char param, int32_t val) : param(init_param_i32(param, val)) {}
  Param(char param, int64_t val) : param(init_param_i64(param, val)) {}
  Param(char param, float val) : param(init_param_f32(param, val)) {}
  Param(char param, double val) : param(init_param_f64(param, val)) {}
  Param(char param, const char *val) : param(init_param_str(param, val)) {}

  Param(Param &&other) : param(other.param) {
    other.param.str = nullptr;
    other.param.param = 0;
  }
  Param(Param &other) = delete;

  ~Param() { free_param(&this->param); }

  uint8_t cast_u8() const { return param_cast_u8(&this->param); }
  int8_t cast_i8() const { return param_cast_i8(&this->param); }
  int16_t cast_i16() const { return param_cast_i16(&this->param); }
  int32_t cast_i32() const { return param_cast_i32(&this->param); }
  int64_t cast_i64() const { return param_cast_i64(&this->param); }
  float cast_f32() const { return param_cast_f32(&this->param); }
  double cast_f64() const { return param_cast_f64(&this->param); }

  char letter() const { return param_letter(&this->param); }

  param_t *replace() {
    free_param(&this->param);
    return &this->param;
  }
};

class Code {
public:
  code_t code;

  Code() : code({0}) {}
  Code(code_t &&code) : code(code) {
    code.params = nullptr;
    code.category = 0;
    code.number = 0;
  }
  Code(char letter, uint8_t number, size_t num_params)
      : code(init_code(letter, number, num_params)) {}
  Code(Code &&other) : code(other.code) {
    other.code.params = nullptr;
    other.code.category = 0;
    other.code.number = 0;
  }
  Code(Code &other) = delete;

  ~Code() { free_code(&this->code); }

  int dump_binary(char *buf, size_t len) const {
    return code_dump_binary(&this->code, buf, len);
  }

  int dump_human(char *buf, size_t len) const {
    return code_dump_human(&this->code, buf, len);
  }

  char letter() const { return code_letter(&this->code); }
  bool is_binary() const { return code_is_binary(&this->code); }

  void set_param(size_t i, Param &&param) {
    code.params[i] = param.param;
    param.param.str = nullptr;
    param.param.param = 0;
  }

  code_t *replace() {
    free_code(&this->code);
    return &this->code;
  }
};

class CodeStream {
public:
  code_stream_t code_stream;

  CodeStream() : code_stream({0}) {}
  CodeStream(code_stream_t &&code_stream) : code_stream(code_stream) {
    code_stream.end = 0;
    code_stream.pos = 0;
    code_stream.cap = 0;
    code_stream.buf = nullptr;
  }
  CodeStream(size_t capacity) : code_stream(init_code_stream(capacity)) {}
  CodeStream(CodeStream &&other) : code_stream(other.code_stream) {
    other.code_stream.buf = nullptr;
    other.code_stream.cap = 0;
    other.code_stream.pos = 0;
    other.code_stream.end = 0;
  }
  CodeStream(CodeStream &other) = delete;

  ~CodeStream() { free_code_stream(&this->code_stream); }

  void update(const char *buf, size_t len) {
    code_stream_update(&this->code_stream, buf, len);
  }

  int pop(code_t *code) { return code_stream_pop(&this->code_stream, code); }
};

#endif
