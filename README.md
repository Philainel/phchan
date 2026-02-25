# phchan

Simple header-only library for thread-safe buffered channels written in C

## Note

This project was written for my own needs, I do not guarantee any support of this project in future

## Usage

Place phchan.h file somewhere in your project and then put this in your C file. Note that #define must be present exactly once
```C
#define PHCHAN_IMPLEMENTATION
#include "phchan.h"
```

Example usage:
```C
phchan_t chan = {0};
phchan_init(&chan, 16); // You better adjust capacity for your own needs

phchan_send(&chan, 10);
phchan_send(&chan, 2);

phchan_close(&chan); // phchan_send() now impossible

int data;
data = phchan_recv(&chan); // 10
data = phchan_recv(&chan); // 2

phchan_destroy(&chan) // must be called after phchan_close() call and after all reading threads exit
```

## Roadmap

- Add non-blocking send/recv
- Add timed send/recv
- Add unit tests
- Add epoll/kqueue variant of channels
- Add C++ operator wrappers (>>)

