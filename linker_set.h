#pragma once

#define LINKERSET_START(_name)                  \
  &__start_##_name

#define LINKERSET_STOP(_name)                   \
  &__stop_##_name

#define LINKERSET_DECLARE(_name)                \
  extern _name##_t __attribute__((weak)) *__start_##_name;  \
  extern _name##_t __attribute__((weak)) *__stop_##_name;  \
  __asm__(".global __start_" #_name);           \
  __asm__(".global __stop_" #_name)

#define LINKERSET_ADD_ITEM(_name, _desc_name)         \
  static void const *__##_name##_ptr_##_desc_name     \
  __attribute__((section(#_name),used)) = &_desc_name

#define LINKERSET_ITERATE(_name, _var, _body)   \
  do {                                          \
    _name##_t **_beg = LINKERSET_START(_name);  \
    _name##_t **_end = LINKERSET_STOP(_name);   \
    while (_beg < _end) {                       \
      _name##_t *_var = *_beg;                  \
      _body;                                    \
      ++_beg;                                   \
    }                                           \
  } while (0)

#define LINKERSET_SIZE_PTRDIFF(_name)               \
  (LINKERSET_STOP(_name) - LINKERSET_START(_name))

#define LINKERSET_SIZE(_name, _type)            \
  (_type)LINKERSET_SIZE_PTRDIFF(_name)

#define LINKERSET_GET(_name, _var, _index)              \
  _name##_t *_var = *(LINKERSET_START(_name) + _index)
