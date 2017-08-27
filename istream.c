#include <istream.h>

#include <error.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

struct cstr_istream {
    struct istream parent;
    const char *str;
    size_t len;
    size_t pos;
};

static bool
cstr_istream_eof(struct istream *_self)
{
   struct cstr_istream *self = (struct cstr_istream *)_self;
   return self->pos == self->len;
}

static struct error *
cstr_istream_get_next_uint8(struct istream *_self, uint8_t *char_out)
{
   struct cstr_istream *self = (struct cstr_istream *)_self;
   if (self->pos >= self->len)
     raise("EOF");
   *char_out = self->str[self->pos];
   self->pos += 1;
   return NULL;
}

struct cstr_istream *
cstr_istream_new()
{
    struct cstr_istream *self = malloc(sizeof(struct cstr_istream));
    self->parent.eof = cstr_istream_eof;
    self->parent.get_next_uint8 = cstr_istream_get_next_uint8;
    self->str = NULL;
    self->pos = 0;
    return self;
}

void
cstr_istream_free(struct cstr_istream *self)
{
    free(self);
}

void
cstr_istream_set(struct cstr_istream *self, const char *str, size_t len)
{
    self->len = len;
    self->str = str;
    self->pos = 0;
}

struct istream *
istream_from_cstr_istream(struct cstr_istream *self)
{
    return (struct istream *)self;
}
