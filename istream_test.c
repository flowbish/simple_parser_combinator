#include <assert.h>
#include <error.h>
#include <istream.h>
#include <test.h>

new_test(cstr_istream_eof_returns_true_on_empty_string)
{
    struct cstr_istream *cis = cstr_istream_new();
    cstr_istream_set(cis, "", 0);

    struct istream *is = istream_from_cstr_istream(cis);
    error_try(assert(istream_eof(is)));

    cstr_istream_free(cis);
    return NULL;
}

new_test(cstr_istream_eof_returns_false_on_non_empty_string)
{
    struct cstr_istream *cis = cstr_istream_new();
    cstr_istream_set(cis, "test", 4);

    struct istream *is = istream_from_cstr_istream(cis);
    error_try(assert(!istream_eof(is)));

    cstr_istream_free(cis);
    return NULL;
}

new_test(cstr_istream_get_next_uint8_returns_error_on_empty_string)
{
    struct cstr_istream *cis = cstr_istream_new();
    cstr_istream_set(cis, "", 0);

    struct istream *is = istream_from_cstr_istream(cis);
    uint8_t unused;
    error_try(assert_error_with_message(istream_get_next_uint8(is, &unused), "EOF"));

    cstr_istream_free(cis);
    return NULL;
}

new_test(cstr_istream_get_next_uint8_returns_error_after_reading_full_string)
{
    struct cstr_istream *cis = cstr_istream_new();
    cstr_istream_set(cis, "ok", 2);

    struct istream *is = istream_from_cstr_istream(cis);
    uint8_t ch;
    error_try(assert_null(istream_get_next_uint8(is, &ch)));
    error_try(assert_null(istream_get_next_uint8(is, &ch)));
    error_try(assert_error_with_message(istream_get_next_uint8(is, &ch), "EOF"));

    cstr_istream_free(cis);
    return NULL;
}

new_test(cstr_istream_get_next_uint8_returns_first_character)
{
    struct cstr_istream *cis = cstr_istream_new();
    cstr_istream_set(cis, "test", 4);

    struct istream *is = istream_from_cstr_istream(cis);
    uint8_t ch;
    error_try(assert_null(istream_get_next_uint8(is, &ch)));
    error_try(assert_uint8_equal(ch, 't'));

    cstr_istream_free(cis);
    return NULL;
}

new_test(cstr_istream_resets_position_on_set)
{
    struct cstr_istream *cis = cstr_istream_new();
    cstr_istream_set(cis, "test", 4);

    struct istream *is = istream_from_cstr_istream(cis);
    uint8_t ch;
    error_try(assert_null(istream_get_next_uint8(is, &ch)));
    error_try(assert_uint8_equal(ch, 't'));
    error_try(assert_null(istream_get_next_uint8(is, &ch)));
    error_try(assert_uint8_equal(ch, 'e'));

    cstr_istream_set(cis, "ok", 2);
    error_try(assert_null(istream_get_next_uint8(is, &ch)));
    error_try(assert_uint8_equal(ch, 'o'));
    error_try(assert_null(istream_get_next_uint8(is, &ch)));
    error_try(assert_uint8_equal(ch, 'k'));

    cstr_istream_free(cis);
    return NULL;
}
