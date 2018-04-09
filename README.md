# Everything Project

Everything I'm working on or have half-complete, all in a single project.

## Getting Started

You should be able to run 'make' and get some executables out.

## Prerequisites

Windows:

* GCC (mingw32 recommended)

Linux:

* X11 libraries

## Installing

### Cross-compiling

To build Windows/wine from Linux:
```
make OS=Windows_NT RM=rm CC=x86_64-w64-mingw32-gcc
```

## Running the Tests

Unit tests are self-contained executables named test\_XXX.

## Coding Style

* Files from other projects keep their original style.
* New files should follow a [BSD kernel normal form](https://www.freebsd.org/cgi/man.cgi?query=style&sektion=9).
* Except where I have deviated. Unless I've made a mistake.

## Contributing

Send diff through email or a github pull request, either will do.

## Credits

Developed by [Jon Mayo](http://orangetide.com/code/)

Contains software from multiple Public Domain sources.

## License

```
------------------------------------------------------------------------------
This software is available under 2 licenses -- choose whichever you prefer.
------------------------------------------------------------------------------
ALTERNATIVE A - MIT License
Copyright (c) 2018 Jon Mayo
Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
of the Software, and to permit persons to whom the Software is furnished to do
so, subject to the following conditions:
The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
------------------------------------------------------------------------------
ALTERNATIVE B - Public Domain (CC0)
To the extent possible under law, I have waived all copyright and related or
neighboring rights to this source code.

For full legal text see
	https://creativecommons.org/publicdomain/zero/1.0/legalcode.txt
-----------------------------------------------------------------------------
```
![CC0 Public Domain](cc0.png)

## Acknowledgments

* [Sean Barrett](https://nothings.org/) - for inspiring many of us by releasing lots of single file public domain source code.
* [David Reid](https://github.com/dr-soft) - for mintaro and mini\_al single file public domain source code.
* [Micha Mettke](https://github.com/vurtun) - for starting his most excellent Nuklear GUI library, as well as the many contributors to that project.
