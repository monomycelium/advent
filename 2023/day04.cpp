#include <cassert>
#include <cinttypes>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <numeric>
#include <ostream>
#include <unordered_set>
#include <vector>

#include "common.h"

extern "C" const std::uint16_t year = 2023;
extern "C" const std::uint8_t day = 04;

/** Representation of a card. */
typedef struct {
    std::unordered_set<std::uintmax_t> winning; /** winning numbers */
    std::unordered_set<std::uintmax_t> nu_have; /** numbers you have */
} Card;

/** Representation of a processed card. */
typedef struct {
    std::vector<std::uintmax_t> winning; /** winning numbers you have */
} Result;

/** Parse the input to obtain a list of the count of winning numbers in each
 * card. */
std::vector<std::uintmax_t> parse(buf_t input);

/** Obtain a list of number of recursive copies won by each card. */
void copies(std::vector<std::uintmax_t> &cards);

void parse_numbers(std::size_t n, char *ptr,
                   std::unordered_set<std::uintmax_t> &a);
std::size_t position_of(char c, buf_t input);
std::size_t len_max(char *ptr);

extern "C" buf_t solve1(buf_t input) {
    std::vector<std::uintmax_t> cards = parse(input);
    std::uintmax_t sum = 0;
    for (std::uintmax_t n : cards) sum += (n > 0) << (n - 1);
    return {.len = 0, .ptr = (uint8_t *)sum};
}

extern "C" buf_t solve2(buf_t input) {
    std::vector<std::uintmax_t> cards = parse(input);

    /* // is this more efficient?
    std::vector<std::size_t> count(cards.size());
    std::uintmax_t sum = 0;
    for (std::size_t i = 0; i < cards.size(); i++) {
        count[i]++;
        for (std::size_t j = 0, max = cards[i]; j < max; j++)
            count[i + j + 1] += count[i];
        sum += count[i];
    }
     */

    copies(cards);
    std::uintmax_t sum = std::accumulate(cards.begin(), cards.end(), 0);

    return {.len = 0, .ptr = (uint8_t *)sum};
}

void copies(std::vector<std::uintmax_t> &cards) {
    size_t i = cards.size();
    while (i--) {
        std::size_t max = cards[i];
        cards[i] = 1;
        for (std::size_t j = 0; j < max; j++) {
            assert(i + j + 1 < cards.size());
            cards[i] += cards[i + j + 1];  // not checking bounds
        }
    }
}

std::vector<std::uintmax_t> parse(buf_t input) {
    std::vector<std::uintmax_t> vec;
    std::size_t len_line;
    std::size_t len_winning;
    std::size_t len_nu_have;
    std::uint8_t len_no_winning;
    std::uint8_t len_no_nu_have;
    std::size_t vertical_bar;
    std::size_t colon;

    len_line = position_of('\n', input);
    vertical_bar = position_of('|', input);
    colon = position_of(':', input);
    len_no_winning = len_max((char *)input.ptr + colon + 2);
    len_no_nu_have = len_max((char *)input.ptr + vertical_bar + 2);
    len_winning = (vertical_bar - colon - 2) / (len_no_winning + 1);
    len_nu_have = (len_line - vertical_bar - 1) / (len_no_nu_have + 1);

    for (char *p = (char *)input.ptr; *p != '\0'; p += len_line + 1) {
        Card card;
        size_t sum = 0;

        parse_numbers(len_winning, p + colon + 2, card.winning);
        parse_numbers(len_nu_have, p + vertical_bar + 2, card.nu_have);

        for (auto &n : card.nu_have) sum += card.winning.contains(n);
        vec.push_back(sum);
    }

    return vec;
}

std::size_t position_of(char c, buf_t input) {
    std::uint8_t *ptr = (std::uint8_t *)std::memchr(input.ptr, c, input.len);
    // if (ptr == NULL) return 0; // -_(•_•)_-
    return ptr - input.ptr;
}

void parse_numbers(std::size_t n, char *ptr,
                   std::unordered_set<std::uintmax_t> &a) {
    a.reserve(a.size() + n);

    for (size_t i = 0; i < n; i++) {
        char *endptr;
        std::uintmax_t num;
        num = std::strtoumax((char *)ptr, &endptr, 10);
        ptr = endptr + 1;
        a.insert(num);
    }

    // for (auto i : a) std::cout << i << ' ';
    // std::cout << std::endl;
}

std::size_t len_max(char *ptr) {
    char *p = ptr;

    for (uint8_t i = 0; i < 2; i++)
        while ((*p == ' ') ^ i) p++;

    return p - ptr;
}
