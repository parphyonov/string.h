#include "s21_string.h"

#define SPECIFIERS "cdieEfgGosuxXpn"
#define MAX_BUFFER_SIZE 64
#define DEBUGGING_MODE 0

// struct to parse format into meaningful chunks
struct Specifier {
  int width, outer_width;
  char type, length;
};

// macros for dealing with ints
#define READ_INT_VALUE(TYPE, BASE)   \
  TYPE* ptr = va_arg(*argus, TYPE*); \
  *ptr = (TYPE)strtol(buffer, NULL, BASE);

// macros for dealing with floats
#define READ_FLOAT_VALUE(TYPE)       \
  TYPE* ptr = va_arg(*argus, TYPE*); \
  *ptr = (TYPE)strtod(buffer, NULL);

/*
    int s21_sscanf(const char *str, const char *format, ...)
        считывает форматированный ввод из строки
*/
int s21_sscanf(const char* str, const char* format, ...);

int deal_vargs(va_list*, char*, char, char, int, int);

void fill_buffer(char*, const char*, char, int, char, int);

int get_num_specs(const char*);
int get_base(const char*, const char);
int get_b4fmt(const char*, const char*, const char, const int, int);

int get_float_len(const char*, const char*);
int get_int_len(const char*, const char*);
int get_str_len(const char*, int);
int get_x_len(const char*, const char*);
int get_8_len(const char*, const char*);

int guess_base(const char*);

int parse_specifier(const char*, struct Specifier*);

int s21_sscanf(const char* str, const char* format, ...) {
  int matches = 0;

  va_list argus;
  va_start(argus, format);

  int spec_num = get_num_specs(format);

  // for handling %n
  int str_tracker = 0;

  // used in DEBUGGING_MODE
  static int casectr = 0;
  if (DEBUGGING_MODE) casectr++;

  if (DEBUGGING_MODE) printf("<<<\t<<<\t<<<\tBEGIN_CASE #%d\n", casectr);

  // empty string from the start
  int is_empty_str = 0;
  if (str[0] == '\0') is_empty_str = 1;

  while (*format) {
    if (DEBUGGING_MODE) printf("\n\n\t--- FORMAT: %s", format);

    // service vars for precision cases parsing
    static int escape_newline = 0;
    static int pullchar = 0;

    // $str != $fmt
    if (*str != *format) {
      if (*str == ' ' && *format == '%' && !pullchar) {
        str++;
        str_tracker++;
        continue;
      }

      if ((*str == '\n') && escape_newline) {
        str++;
        str_tracker++;
        continue;
      }
    }

    // driver
    if (*str == *format) {
      str++;
      str_tracker++;
      format++;
    }

    // $fmt == '%'
    if (*format == '%' && *(format + 1) != '%' && *(format + 1) != '\0') {
      // init struct for current specifier
      struct Specifier current_specifier = {
          .width = 0, .outer_width = 0, .type = '\0', .length = 0};

      // encounter speciefier, parse, step over in format
      format += parse_specifier(format, &current_specifier);

      // shorter refs
      char type = current_specifier.type;
      char len = current_specifier.length;
      int w = current_specifier.width;
      int ow = current_specifier.outer_width;

      // visually controlling output
      if (DEBUGGING_MODE) {
        printf("\n\t--- STRING: %s", str);
        printf("\n\t--- LEN=%c, T=%c, OW=%d, W=%d\n", len, type, ow, w);
      }

      // safeguards for newlines when type != 'c'
      // pullchar is when type = 'c' and we need to read space as char
      if (type != 'c') {
        pullchar = 0;
        escape_newline = 1;
      } else {
        pullchar = 1;
        escape_newline = 0;
      }

      // base for ints
      int base = get_base(str, type);
      // for how deep into str should we venture
      int b4fmt = get_b4fmt(format, str, type, base, w);

      // skip \t in the beginning
      if (type == 's' && *str == '\t')
        while (*str == '\t') {
          str++;
          str_tracker++;
        }

      // safeguards for scan failure
      if (*str != ' ' && b4fmt == 0) continue;

      // buffer for ints, floats, pointers and strings
      char buffer[MAX_BUFFER_SIZE];
      s21_memset(buffer, 0, sizeof(buffer));

      // buffer time
      if (w > -1) {
        fill_buffer(buffer, str, type, w, len, b4fmt);

        // skip unplanned \t
        if (type == 's') {
          for (s21_size_t i = 0; i < (s21_size_t)b4fmt; i++)
            if (buffer[i] == '\t') buffer[i] = '\0';
        }

        // use va_arg
        matches += deal_vargs(&argus, buffer, type, len, base, str_tracker);
        if (type == 's' && buffer[0] == '\0') matches -= 1;
      }

      if (DEBUGGING_MODE)
        printf("\t--- b4fmt=%d, buff=%s (size %lu)", b4fmt, buffer,
               s21_strlen(buffer));

      // string stepover calculation
      int str_inc = 0;
      if (type == 'p')
        str_inc = b4fmt;
      else {
        if (w <= 0)
          str_inc = b4fmt;
        else {
          if (w > b4fmt)
            str_inc = b4fmt;
          else
            str_inc = w;
        }
      }

      // step over in str
      str += str_inc;
      str_tracker += str_inc;

      if (*str != ' ' && (*format == ' ' || *format == '\t')) format++;

    } else if (*format == '%' && *(format + 1) == '%')
      // %% scenario
      format += 1;

    // further driving
    if (*str != *format) {
      if (*format == '%')
        continue;
      else
        break;
    }

    // break if matches somehow overexceed
    if (matches >= spec_num) break;
    // break if there are no more '%'
    if (s21_strchr(format, '%') == NULL) spec_num = matches;
  }

  if (DEBUGGING_MODE) printf("\n<<<\t<<<\t<<<\tFINISH_CASE #%d\n\n", casectr);

  va_end(argus);

  if (is_empty_str && matches == 0) matches = -1;

  return matches;
}

int deal_vargs(va_list* argus, char* buffer, char type, char len, int base,
               int str_tracker) {
  int success = 1;
  if (type == 'c' || type == 's') {
    char* ptr = va_arg(*argus, char*);
    s21_strncpy(ptr, buffer, s21_strlen(buffer));
  } else if (type == 'd' || type == 'i' || type == 'o' || type == 'x' ||
             type == 'X' || type == 'u') {
    if (len == 'h') {
      if (type == 'x' || type == 'X' || type == 'o' || type == 'u') {
        READ_INT_VALUE(unsigned short, base);
      } else {
        READ_INT_VALUE(short, base);
      }
    } else if (len == 'l') {
      if (type == 'x' || type == 'X' || type == 'o' || type == 'u') {
        READ_INT_VALUE(unsigned long, base);
      } else {
        READ_INT_VALUE(long, base);
      }
    } else {
      if (type == 'x' || type == 'X' || type == 'o' || type == 'u') {
        READ_INT_VALUE(unsigned, base);
      } else {
        READ_INT_VALUE(int, base);
      }
    }
  } else if (type == 'f' || type == 'e' || type == 'E' || type == 'g' ||
             type == 'G') {
    if (type == 'g' || type == 'G') {
      if (len == 'L') {
        READ_FLOAT_VALUE(long double);
      } else {
        READ_FLOAT_VALUE(float);
      }
    } else {
      if (len == 'L') {
        READ_FLOAT_VALUE(long double);
      } else {
        READ_FLOAT_VALUE(float);
      }
    }
  } else if (type == 'p') {
    void* res = (void*)strtol(buffer, NULL, 16);
    void** ptr = va_arg(*argus, void**);
    *ptr = res;
  } else if (type == 'n') {
    int* ptr = va_arg(*argus, int*);
    *ptr = str_tracker;
    success = 0;
  }

  return success;
}

void fill_buffer(char* buffer, const char* str, char type, int w, char len,
                 int b4fmt) {
  if (type == 'd' || type == 'i' || type == 'o' || type == 'x' || type == 'X' ||
      type == 'u' || type == 'f' || type == 'e' || type == 'E' || type == 'g' ||
      type == 'G') {
    if (w == 0)
      s21_strncpy(buffer, str, b4fmt);
    else
      s21_strncpy(buffer, str, w);
  } else if (type == 'p' || type == 's' || type == 'c') {
    if (type == 's' && len == 'l')
      s21_strncpy(buffer, str, 1);
    else
      s21_strncpy(buffer, str, b4fmt);
  }
}

// returns number of '%' in the format, skipping "%%"
int get_num_specs(const char* fmt) {
  int num = 0;

  while (*fmt) {
    if (*fmt == '%' && *(fmt + 1) != '%' && *(fmt + 1) != '\0') num++;
    fmt++;
  }

  return num;
}

int get_base(const char* str, const char type) {
  int base = 10;
  if (type == 'd' || type == 'i' || type == 'o' || type == 'x' || type == 'X' ||
      type == 'u')
    base = guess_base(str);
  if (type == 'o') base = 8;
  if (type == 'x' || type == 'X' || type == 'p') base = 16;
  return base;
}

int get_b4fmt(const char* fmt, const char* str, const char type, const int base,
              int w) {
  int b4fmt = 0;
  if (type == 'c')
    b4fmt = 1;
  else if (type == 'd' || type == 'i' || type == 'o' || type == 'x' ||
           type == 'X' || type == 'u' || type == 'p') {
    if (base == 10)
      b4fmt = get_int_len(str, fmt);
    else if (base == 8)
      b4fmt = get_8_len(str, fmt);
    else if (base == 16 || type == 'p')
      b4fmt = get_x_len(str, fmt);
  } else if (type == 'f' || type == 'e' || type == 'E' || type == 'g' ||
             type == 'G')
    b4fmt = get_float_len(str, fmt);
  else if (type == 's')
    b4fmt = get_str_len(str, w);

  return b4fmt;
}

// counts symbols in a float or exp considering possible chars
int get_float_len(const char* str, const char* fmt) {
  int len = 0;
  int dot = 0;
  int mant_m = str[0] == '-';
  int e = 0;
  int exp_m = 0;

  while (*str != *fmt && (isdigit(*str) || *str == '.' || *str == '-' ||
                          *str == '+' || *str == 'e' || *str == 'E')) {
    // skips unrelated minuses
    if (!mant_m && !e && *(str + 1) == '-') len++;

    // skips the second dot
    if (*(str + 1) == '.' && dot) {
      len++;
      break;
    }
    // sets the dot
    if (*str == '.') dot = 1;

    // skips minus after exponent
    if (e && exp_m && *(str + 1) == '-') {
      len++;
      break;
    }

    // deal the e
    if (*str == 'e' || *str == 'E') e = 1;

    // set exponent minus
    if (e && *(str + 1) == '-') exp_m = 1;

    // move the wheel
    str++;
    len++;
  }

  return len;
}

int get_int_len(const char* str, const char* fmt) {
  int len = 0;

  while (*str != *fmt && (isdigit(*str) || *str == '-' || *str == '+') &&
         *(str + 1) != '-') {
    // move the wheel
    str++;
    len++;
  }

  return len;
}

// counts symbols until the next ' ' or '\0'
// but if limit is set breaks at ' '
int get_str_len(const char* str, int limit) {
  int len = 0;

  if (limit == 0 || limit == -1) {
    while (*str != ' ' && *str != '\0') {
      // move the wheel
      len++;
      str++;
      if (*str == '\t') break;
    }
  } else {
    while (len < limit) {
      // move the wheel
      len++;
      str++;
      if (*str == ' ') break;
      if (*str == '\t') break;
    }
  }

  return len;
}

// counts number of chars in a x16 string
int get_x_len(const char* str, const char* fmt) {
  int len = 0;
  int x = 0;
  while (*str != *fmt && (isxdigit(*str) || *str == 'x' || *str == 'X')) {
    if (x && *(str + 1) == '0' && (*(str + 2) == 'x' || *(str + 2) == 'X'))
      break;
    if ((*str == 'x' || *str == 'X') && !x) x = 1;
    // move the wheel
    str++;
    len++;
  }
  return len;
}

int get_8_len(const char* str, const char* fmt) {
  int len = 0;

  while ((*str != *fmt && *str == '-') || (*str >= '0' && *str < '8')) {
    // move the wheel
    str++;
    len++;
  }

  return len;
}

// x16 if start with '0' followed by 'x' or 'X'
// x8 if starts with '0' and followed by digit
int guess_base(const char* str) {
  int res = 10;

  if (*str == '0') {
    if (*(str + 1) == 'x' || *(str + 1) == 'X')
      res = 16;
    else if (isdigit(*(str + 1)))
      res = 8;
  }
  return res;
}

// parses parts of format into meaningful chunks
// and sets an instance of struct Specifier
int parse_specifier(const char* format, struct Specifier* specifier) {
  // this is how we know the number of characters
  // between % and specifier (e.g. %02d will result in 3)
  specifier->outer_width = s21_strcspn(format, SPECIFIERS);
  // with the OW we can now have the specifier saved
  specifier->type = *(format + specifier->outer_width);

  // use OW to check if one of the 3 possible lengthes are provided
  if (*(format + specifier->outer_width - 1) == 'h' ||
      *(format + specifier->outer_width - 1) == 'l' ||
      *(format + specifier->outer_width - 1) == 'L') {
    // since it's always 1 position before the specifier
    specifier->length = *(format + specifier->outer_width - 1);
  }

  // checks if the next char after % is *
  if (*(format + 1) == '*') {
    // if so, W will be -1; will later use this value
    // to suppress variable assignment
    specifier->width = -1;
  }

  int additional_step = specifier->length > 0 ? 1 : 0;

  // here we figure out initial multiplier considering length
  // and -2 here stands for the specifier proper (i, d, e...)
  // and to get the right exponent we need to decrement it by 1
  int initial_multipier = specifier->outer_width - additional_step - 2;

  if (initial_multipier > -1) {
    // we need this line to make 1 step further after %
    // to start parsing proper
    format++;

    // and here we parse the number to its exponents
    if (specifier->width != -1) {
      for (int i = initial_multipier; i >= 0; i--, format++) {
        // here we add to W for later use outside in s21_sscanf
        specifier->width += (*(format)-48) * pow(10, i);
      }
    }
  } else {
    specifier->width = 0;
  }

  // also need to increment format outside
  // +1 because of % sign not being stepped over outside
  return specifier->outer_width + 1;
}