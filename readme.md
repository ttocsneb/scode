# SCode

SCode (Serial Code) is a language that is inspired by and backwards compatible 
with GCode. It is intended to be a very light weight and efficient to parse language
for communicating between embedded devices. This library is designed to parse and
dump SCode.

## Library Usage

There are a few structs that are made available in `scode.h`.

* param_t
* code_t
* code_stream_t

### param_t

param_t is an object that contains a parameter. It can hold heap data, so after
use: `free_param()` should be called. It is only necessary to free a param if the
contained data is a string, but if you do not know its type, then it is safe to
call `free_param()` even if there is nothing to free. There are several helper 
functions provided for param_t:

The init functions initialize parameters with the given data value.

* init_param_u8(char param, uint8_t val)
* init_param_i8(char param, int8_t val)
* init_param_i16(char param, int16_t val)
* init_param_i32(char param, int32_t val)
* init_param_i64(char param, int64_t val)
* init_param_f32(char param, float val)
* init_param_f64(char param, double val)
* init_param_str(char param, char *val)

You can also cast any numerical parameter into any other numerical data type.

* param_cast_u8(const param_t *self)
* param_cast_i8(const param_t *self)
* param_cast_i16(const param_t *self)
* param_cast_i32(const param_t *self)
* param_cast_i64(const param_t *self)
* param_cast_f32(const param_t *self)
* param_cast_f64(const param_t *self)

You can get the type of a parameter with `param_type` which returns a constant 
for `PARAM_T_TYPENAME`

* param_type(const param_t *self)

You can also get the letter of the parameter.

* param_letter(const param_t *self)

If you know the type of a parameter, then it would be safe to directly access it
through the struct.

### code_t

code_t contains information for an entire code. It does contain heap data,
so a call to `free_code()` is necessary after use.

A code contains some number of params. The number of params is not known, so the
last item in the array will always be a null param.

There are several helper codes for code:

You can initialize a code with a specified number of params (A null terminator 
will automatically be added for you)

* init_code(char letter, uint8_t number, size_t num_params)

You can parse a string into a code object

* code_parse(code_t *self, const char *buf, size_t len)

You can dump the code object into a string either in human or binary form.

* code_dump_binary(const code_t *self, char *buf, size_t len)
* code_dump_human(const code_t *self, char *buf, size_t len)

You can get the code's letter

* code_letter(const code_t *self)

You can also determine if the parsed code was in binary or human form.

* code_is_binary(const code_t *self)

### code_stream_t

The code stream is used as a helper for parsing an input stream into codes. It's
buffer is stored on the heap, so a call to `free_code_stream()` should be called
after use.

You can initialize a new code stream with

* init_code_stream(size_t capacity)

You can add data to the code stream buffer with

* code_stream_update(code_stream_t *self, const char *buf, size_t len)

You can parse the next code from the code stream with

* code_stream_pop(code_stream *self, code_t *code)


## Serial Code Usage

In this language specification, there is no explicit definitions for what each code
type is used for. It is however recommended to follow pre-existing standards (G-Codes
and M-Codes).

Serial Codes can be used to issue commands or request information. A command would
work how one might expect it to where a serial code is sent and the receiver follows
the command. A request for information will look just like a command, but instead
of the receiver doing something, it will instead send a Serial Code back with the
requested information.

Say you have a sensor 3 that is accessible through `S3`. You can request information
by sending the Serial Code `S3` and the receiver will send back something like 
this: `S3 V123.4`.

## Language Specification

There are two modes of communication: human and binary. These two modes both represent
the same data, but binary is designed to be more compact, easier to parse, and 
more reliable. The human mode is fully compatible with GCode.

A Serial Code starts with a code type: This is one of 26 letters (A-Z). Followed
by this type is a code number (0-255). After the code, any number of parameters
can be provided. A parameter is defined by a code and a value (also one of 26 letters).
The value can be a number or a string.

You can determine whether a serial code is binary or human by the most significant
bit. 

```
0100 1101 - Start of human serial code
1100 1101 - Start of binary serial code
```

### Values

In human form, there is really only two value types: number and string. The binary
values can be represented by one of 8 unit types: u8, i8, i16, i32, i64, f32, and
f64. The type of a value is encoded in the parameter code byte.

The three most significant bits of a parameter code is reserved for the type.

```
       Code M = 0100 1101
Binary Code M = XXX0 1101
```

The coresponding types are as follows:

* 000 = f64
* 001 = f32
* 010 = i64
* 011 = i32
* 100 = i16
* 101 = i8
* 110 = u8
* 111 = str

After the binary parameter code, then the binary representation (in little endian)
follows. Strings are formatted as a null-terminated string.

### Binary Serial Codes

The Binary serial code begins with the code type (the most significant bit is set
to 1). After this is the code number which is represented as a binary u8. After
this any number of parameters can be entered. The end of the parameters is marked
by a NULL character. After the parameters is a CRC-8 (poly: 0xD7) of the serial
code excluding the final NULL character and crc byte.

Here is an example of a human code and it's binary alternative:

```
G34 X-2 Y3 Z4
0xC7 0x22 0xB8 0xFE 0xB9 0x03 0xBA 0x04 0x00 0xB9
G    34   i8X  -2   i8Y  3    i8Z  4    end  crc
```

## Language Definition

There are two language definitions below. When parsing, only one or the other should
match. It is impossible for both languages to accept the same input string. When
parsing, any leading whitespace should be stripped.

The first is the Human language definition. It is defined in terms of 8-bit ascii.

```
CODE ::= LETTER NUMERAL PADDING PARAMS COMMENT [\n] | COMMENT [\n]
PARAMS ::= PARAM PADDING | PARAMS | ε

PADDING ::= SPACE PADDING | ε
SPACE ::= [ ] | [\t] | [\r]

PARAM ::= LETTER STRING | LETTER NUMBER
LETTER ::= [A-Z] | [a-z]

STRING ::= ["] CONTENTS_D ["] | ['] CONTENTS_S [']
CONTENTS_S ::= [^'] CONTENTS_S | ε
CONTENTS_D ::= [^"] CONTENTS_S | ε

COMMENT ::= [;] CONTENTS_C | ε
CONTENTS_C ::= [^\n] CONTENTS_C | ε

NUMBER ::= FLOAT | INTEGER
FLOAT ::= INTEGER [.] NUMERAL | [.] NUMERAL
INTEGER ::= [-] NUMERAL | NUMERAL
NUMERAL ::= DIGIT NUMERAL | DIGIT
DIGIT ::= [0-9]
```

The second is the Binary language definition and this is defined in terms of bits.

```
BCODE ::= [110] LETTER U8 PARAMS NULL U8

PARAMS ::= PARAM_U8 PARAMS |
           PARAM_I8 PARAMS |
           PARAM_I16 PARAMS |
           PARAM_I32 PARAMS |
           PARAM_I64 PARAMS |
           PARAM_F32 PARAMS |
           PARAM_F64 PARAMS |
           PARAM_STR PARAMS | ε

PARAM_U8  ::= [110] LETTER U8
PARAM_I8  ::= [101] LETTER I8
PARAM_I16 ::= [100] LETTER I16
PARAM_I32 ::= [011] LETTER I32
PARAM_I64 ::= [010] LETTER I64
PARAM_F32 ::= [001] LETTER F32
PARAM_F64 ::= [000] LETTER F64
PARAM_STR ::= [111] LETTER STR

U8 ::= 8 * X
I8 ::= 8 * X
I16 ::= 16 * X
I32 ::= 32 * X
I64 ::= 64 * X
F32 ::= 32 * X
F64 ::= 64 * X
STR ::= U8 STR | NULL

LETTER ::= [00001-11010]

X ::= [0] | [1]
NULL ::= [00000000]
```
