#ifndef OTHERMACROS_H
#define OTHERMACROS_H


// MACROS /////////////////////////////////////////////////
#define KEYDOWN(vk_code) ((GetAsyncKeyState(vk_code) & 0x8000) ? 1 : 0)

#endif