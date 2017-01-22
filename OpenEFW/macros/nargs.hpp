/*
 * Copyright (c) 2015, Mario Link
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 
 * * Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 * 
 * * Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 * 
 * * Neither the name of OpenEFW nor the names of its
 *   contributors may be used to endorse or promote products derived from
 *   this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#pragma once

#ifndef NARGS
#define NARGS
#define NARGS_EMPTY()

#define NARGS_ARG(x) x
#define NARGS_CAT(x, y) NARGS_ARG(x) ## NARGS_ARG(y)
#define NARGS_COUNT(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, N, ...) N

#define NARGS_1(s, m, x1) m(x1)
#define NARGS_2(s, m, x1, x2) m(x1) ## s() ## m(x2)
#define NARGS_3(s, m, x1, x2, x3) m(x1) ## s() ## NARGS_2(s, m, x2, x3)
#define NARGS_4(s, m, x1, x2, x3, x4) m(x1) ## s() ## NARGS_3(s, m, x2, x3, x4)
#define NARGS_5(s, m, x1, x2, x3, x4, x5) m(x1) ## s() ## NARGS_4(s, m, x2, x3, x4, x5)
#define NARGS_6(s, m, x1, x2, x3, x4, x5, x6) m(x1) ## s() ## NARGS_5(s, m, x2, x3, x4, x5, x6)
#define NARGS_7(s, m, x1, x2, x3, x4, x5, x6, x7) m(x1) ## s() ## NARGS_6(s, m, x2, x3, x4, x5, x6, x7)
#define NARGS_8(s, m, x1, x2, x3, x4, x5, x6, x7, x8) m(x1) ## s() ## NARGS_7(s, m, x2, x3, x4, x5, x6, x7, x8)
#define NARGS_9(s, m, x1, x2, x3, x4, x5, x6, x7, x8, x9) m(x1) ## s() ## NARGS_8(s, m, x2, x3, x4, x5, x6, x7, x8, x9)
#define NARGS_10(s, m, x1, x2, x3, x4, x5, x6, x7, x8, x9, x10) m(x1) ## s() ## NARGS_9(s, m, x2, x3, x4, x5, x6, x7, x8, x9, x10)

#define NARGS_S(split,macro,...) NARGS_CAT(NARGS_, NARGS_COUNT(__VA_ARGS__, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1))(split, macro,__VA_ARGS__)

#define NARGS_SEPARATOR() ,
#define NARGS_MACRO(macro,...) NARGS_S(NARGS_SEPARATOR, macro,__VA_ARGS__)

#define NARGS
#endif