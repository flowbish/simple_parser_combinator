# Simple Parser Combinator

## Usage

Not really ready for use in other applications yet! But you can try copying the .c files and adding to your build system... probably not the best idea though.

## Examples

Here's a simple example to parse roman numerals with X, V, and I:

```C
parser roman_numeral_simple() {
  parser parse_x = many(ch('X'));
  parser parse_v = optional(ch('V'))
  parser parse_i = or3(try(and(ch('I'), ch('X'))),
                       try(and(ch('I'), ch('V'))),
                       optional(many(ch('I'))));
  return and4(parse_x, parse_v, parse_i, eof);
}

int main() {
  char *input = "IX", *output = NULL;
  bool success = run(roman_numeral_simple(), input, &output);
  if (success) {
    printf("Parser parsed: %s\n", output);
  } else {
    printf("Parser did not parse successfully.");
  }
}
```

Here's a more involved example leveraging the ability to attach function to parsed targets:

```C
int value(char c) {
  switch(c) {
  case 'I':
    return 1;
  case 'V':
    return 5;
  case 'X':
    return 10;
  case 'L':
    return 50;
  case 'C':
    return 100;
  case 'D':
    return 500;
  case 'M':
    return 1000;
  default:
    return -1;
  }
}

bool add_value(char *letter, void *total) {
  *(size_t *)total += value(*letter);
  info("adding %d total %zu", value(*letter), *(size_t *)total);
  return true;
}

bool sub_value(char *letter, void *total) {
  *(size_t *)total -= value(*letter);
  return true;
}

parser pair(char a, char b, void *total) {
  return and(exe(ch(a), sub_value, total),
             exe(ch(b), add_value, total));
}

parser single(char a, size_t *total) {
  return optional(many(exe(ch(a), add_value, total)));
}

parser roman_numeral_complex(size_t *total) {
  parser parse_m = single('M', total);
  parser parse_d = single('D', total);
  parser parse_c = or3(try(pair('C', 'M', total)),
                       try(pair('C', 'D', total)),
                       single('C', total));
  parser parse_l = single('L', total);
  parser parse_x = or3(try(pair('X', 'C', total)),
                       try(pair('X', 'L', total)),
                       single('X', total));
  parser parse_v = single('V', total);
  parser parse_i = or3(try(pair('I', 'X', total)),
                       try(pair('I', 'V', total)),
                       single('I', total));
  return and8(parse_m, parse_d, parse_c, parse_l, parse_x,
              parse_v, parse_i, eof);
}

int main() {
  char *input = "CCXCIV", *output = NULL;
  size_t *total = 0;
  bool success = run(roman_numeral_complex(&total), input, &output);
  if (success) {
    printf("Parser parsed: %s with value %zu\n", output, total);
  } else {
    printf("Parser did not parse successfully.");
  }
}
```
