#ifndef MYDDMACROS_H
#define MYDDMACROS_H


// initializes a direct draw struct
#define DD_INIT_STRUCT(ddstruct) { memset(&ddstruct,0,sizeof(ddstruct)); ddstruct.dwSize=sizeof(ddstruct); }


// this builds 16 bit color value in 5.5.5 format
#define _RGB16BIT555(r,g,b) ((b&31)+((g&31)<<5)+((r&31)<<10));
// this builds 16 bit color value in 5.6.5 format
#define _RGB16BIT565(r,g,b) ((b&31)+((g&63)<<5)+((r&31)<<11));

//this builds 24 bit color value in 8.8.8 format
#define _RGB24BIT(r,g,b) ((b)+((g)<<8)+((r)<<16))

//this builds 32 bit color value in 8.8.8 format
#define _RGB32BIT(a,r,g,b) ((b) + ((g) << 8) + ((r) << 16) + ((a) << 24))


#endif