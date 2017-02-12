#pragma once


#define __GLOBL1(sym)   __asm__(".globl " #sym)
#define __GLOBL(sym) __GLOBL1(sym)

#define __used __attribute__((__used__))
#define __weak __attribute__((weak))
#define __section(x) __attribute__((__section__(x)))

#define __MAKE_SET(set, sym)                      \
  __GLOBL(__CONCAT(__start_set_,set));            \
  __GLOBL(__CONCAT(__stop_set_,set));             \
  static void const * const                       \
  __set_##set##_sym_##sym __section("set_" #set)  \
       __used = &(sym)

#define SET_ENTRY(set, sym) __MAKE_SET(set, sym)

#define SET_DECLARE(set, ptype)                           \
  extern ptype __weak * __start_set_##set; \
  extern ptype __weak * __stop_set_##set

#define SET_BEGIN(set)                          \
  (&__start_set_##set)

#define SET_LIMIT(set)                          \
  (&__stop_set_##set)

#define SET_FOREACH(pvar, set)                                \
  for (pvar = SET_BEGIN(set); pvar < SET_LIMIT(set); pvar++)

#define SET_ITEM(set, i)                        \
  ((SET_BEGIN(set))[i])

#define SET_COUNT(set)                          \
  (SET_LIMIT(set) - SET_BEGIN(set))
