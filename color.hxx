#ifndef _MY_COLORIZE_
#define _MY_COLORIZE_

#include <iostream>

enum Color {
    black,
    red,
    green,
    yellow,
    blue,
    purple
};

struct ColorStream : std::ostream, std::streambuf {
    ColorStream() : std::ostream(this) {}

    int overflow(int c) {
        std::cout.put(c);
        return 0;
    }
};

#endif
