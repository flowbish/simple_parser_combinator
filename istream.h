#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

struct istream;

/*
 * Read a single byte from the istream. istream_get_uint8 will return an error
 * if the istream is closed when the function is called.
 */
typedef struct error *istream_get_next_uint8_t(struct istream *, uint8_t *);
#define istream_get_next_uint8(is, uint8) \
    is->get_next_uint8((is), (uint8))

/*
 * Returns true if the istream has closed and has no more input.
 */
typedef bool istream_eof_t(struct istream *);
#define istream_eof(is) \
    is->eof((is))

struct istream {
    istream_get_next_uint8_t *get_next_uint8;
    istream_eof_t *eof;
};

/*
 * cstr_istream is an implementation of istream that uses a backing cstr to
 * draw input from. The cstr_istream borrows a reference to the cstr passed in.
 */

struct cstr_istream;

struct cstr_istream *
cstr_istream_new();

void
cstr_istream_free(struct cstr_istream *);

/*
 * Set the inner string used by the cstr_istream. The string must not be NULL.
 */
void
cstr_istream_set(struct cstr_istream *, const char *, size_t);

struct istream *
istream_from_cstr_istream(struct cstr_istream *);
