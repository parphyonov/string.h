# Клон стандартной библиотеки `string.h`

В рамках учебного проекта Школы 21 были реализован клон стандартной библиотеки **string.h**

Это был групповой проект, потому привожу только код реализованных мной функций и комментарии к коду

### strntok

```C
/*
  char *strtok(char *s, const char *delim);

      извлекает токены из строки s
      на основе делимитеров в виде любого из байтов в delim

      ВАЖНО! strtok изменяет оригинальную строку s,
      заменяя символы-делимитеры терминирующим нулем
      Если оригинальная строка потребуется в коде в дальнейшем,
      рекомендуется сделать копию прежде использования strtok
*/
char *s21_strtok(char *s, const char *delim) {
  char *out = NULL;
  static char *last_token = NULL;

  if (s != NULL) {
    last_token = s;
    while (last_token != NULL && *last_token != '\0' &&
           s21_strchr(delim, *last_token) != NULL)
      last_token++;
  }

  if (last_token != NULL && *last_token != '\0') {
    char *token_start = last_token;
    char *token_end = s21_strpbrk(last_token, delim);

    if (token_end != NULL) {
      *token_end = '\0';
      last_token = token_end + 1;
      out = token_start;
    } else {
      out = token_start;
      last_token = NULL;
    }

    while (last_token != NULL && *last_token != '\0' &&
           s21_strchr(delim, *last_token) != NULL)
      last_token++;
  }

  return out;
}
```

### strerror

Папка `error_messages` с кодами сообщений для Linux и Apple и соответствующие заголовочные файлы находятся в `code-samples`

```C
#ifdef __linux__
#include "./error_messages/s21_linux_error_messages.h"
#define SYSTEM 0

#elif __APPLE__
#include "./error_messages/s21_mac_error_messages.h"
#define SYSTEM 1

#endif


#define ERR_SIZE 64
#define STRERR_STACK 256

/*
  char *s21_strerror(int errnum);

    выполняет поиск во внутреннем массиве номера ошибки errnum
    и возвращает указатель на строку с сообщением об ошибке

    нужно объявить макросы, содержащие массивы сообщений об ошибке
    для операционных систем mac и linux

    описания ошибок есть в оригинальной библиотеке
    проверка текущей ОС осуществляется с помощью директив
*/

char *s21_strerror(int errnum) {
  static char call_stack[STRERR_STACK][ERR_SIZE];
  static int stack_ind = 0;

  char *error_array[ERROR_ARRAY_SIZE] = {ERROR_ARRAY};

  if (errnum < 0 || errnum > ERROR_ARRAY_SIZE - 1) {
    if (SYSTEM == 0)
      snprintf(call_stack[stack_ind], ERR_SIZE, "Unknown error %d", errnum);
    else if (SYSTEM == 1)
      snprintf(call_stack[stack_ind], ERR_SIZE, "Unknown error: %d", errnum);
  } else {
    if (SYSTEM == 0 || SYSTEM == 1)
      snprintf(call_stack[stack_ind], ERR_SIZE, "%s", error_array[errnum]);
    else
      snprintf(call_stack[stack_ind], ERR_SIZE, "%s",
               "your system is not supported");
  }

  stack_ind %= STRERR_STACK;
  return call_stack[stack_ind++];
}
```

### strncat

> 🚨 **ВНИМАНИЕ!** результаты автотестов школы не совпали с результатами, сформировнными этой функцией

```C
/*
  char *strncat(char *dest, const char *src, size_t n);

  прибавляет максимум n байтов из строки src к строке dest,
  возвращает указатель на dest
*/
char *s21_strncat(char *dest, const char *src, s21_size_t n) {
  // in case src is empty \0
  if (s21_strlen(src) == 0) return dest;

  // main scenario
  if ((dest != NULL) && (src != NULL)) {
    s21_size_t dest_len = s21_strlen(dest);

    s21_size_t total_len = dest_len + n + 1;

    if (total_len < dest_len + s21_strlen(src) + 2) {
      // поиск терминирующего нуля в строке назначения
      while (*dest) dest++;

      // прибавляем до n символов из строки отправления
      for (s21_size_t i = 0; i < n && src[i] != '\0'; i++) *dest++ = src[i];

      // ставим новый терминирующий нуль в строке назначения
      *dest = '\0';
    } else {
      s21_strncpy(dest, src, s21_strlen(src));
      dest[s21_strlen(src)] = '\0';
      return dest;
    }
  }

  return dest;
}
```

### sscanf

Мною был написан клон небиблиотечной функции `sscanf` (требовалась по условиям проекта)

В полным текстом функции можно ознакомиться в папке `code-samples/21_sscanf.c`

На этапе финального тестирования выяснил, что функция неправильно считывала числа в восьмеричной системе счисления (8 тоже считалась)

Также это был первый групповой проект – каждый участник команды бросился реализовывать свои функции без продумывания совместного использования наработок друг друга. Другой участник, например, писал клон `sprinf` и в теории эти две «обратные» друг другу функции мы могли бы написать быстрее и эффективнее

> 🚨 **ВНИМАНИЕ!** эта функция не прошла автотесты школы. Подробнее ниже:

```
Project build: OK
1
Build result: 1
-------------------------------------------------------------------------------

Test number: 17, name: sscanf

Test output:
Functional test output: False
Memory test output:
==305== Memcheck, a memory error detector
==305== Copyright (C) 2002-2022, and GNU GPL'd, by Julian Seward et al.
==305== Using Valgrind-3.19.0 and LibVEX; rerun with -h for copyright info
==305== Command: /builds/pipelines/test/to/tests/unit-tests/part4_tests sscanf
==305== 
==305== Conditional jump or move depends on uninitialised value(s)
==305==    at 0x109321: test_sscanf (in /builds/pipelines/test/to/tests/unit-tests/part4_tests)
==305==    by 0x10942A: main (in /builds/pipelines/test/to/tests/unit-tests/part4_tests)
==305== 
==305== Conditional jump or move depends on uninitialised value(s)
==305==    at 0x109349: test_sscanf (in /builds/pipelines/test/to/tests/unit-tests/part4_tests)
==305==    by 0x10942A: main (in /builds/pipelines/test/to/tests/unit-tests/part4_tests)
==305== 
==305== Conditional jump or move depends on uninitialised value(s)
==305==    at 0x48A8494: strcmp (in /usr/libexec/valgrind/vgpreload_memcheck-amd64-linux.so)
==305==    by 0x10937D: test_sscanf (in /builds/pipelines/test/to/tests/unit-tests/part4_tests)
==305==    by 0x10942A: main (in /builds/pipelines/test/to/tests/unit-tests/part4_tests)
==305== 
==305== Conditional jump or move depends on uninitialised value(s)
==305==    at 0x109380: test_sscanf (in /builds/pipelines/test/to/tests/unit-tests/part4_tests)
==305==    by 0x10942A: main (in /builds/pipelines/test/to/tests/unit-tests/part4_tests)
==305== 
==305== Conditional jump or move depends on uninitialised value(s)
==305==    at 0x48A8494: strcmp (in /usr/libexec/valgrind/vgpreload_memcheck-amd64-linux.so)
==305==    by 0x109398: test_sscanf (in /builds/pipelines/test/to/tests/unit-tests/part4_tests)
==305==    by 0x10942A: main (in /builds/pipelines/test/to/tests/unit-tests/part4_tests)
==305== 
==305== Conditional jump or move depends on uninitialised value(s)
==305==    at 0x10939B: test_sscanf (in /builds/pipelines/test/to/tests/unit-tests/part4_tests)
==305==    by 0x10942A: main (in /builds/pipelines/test/to/tests/unit-tests/part4_tests)
==305== 
==305== 
==305== HEAP SUMMARY:
==305==     in use at exit: 0 bytes in 0 blocks
==305==   total heap usage: 0 allocs, 0 frees, 0 bytes allocated
==305== 
==305== All heap blocks were freed -- no leaks are possible
==305== 
==305== Use --track-origins=yes to see where uninitialised values come from
==305== For lists of detected and suppressed errors, rerun with: -s
==305== ERROR SUMMARY: 6 errors from 6 contexts (suppressed: 0 from 0)

Test result for the function s21_sscanf: FAIL

Memory test: FAIL 
0
Test result: 0
```