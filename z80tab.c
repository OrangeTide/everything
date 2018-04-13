/* z80tab.c : taken from z80emu.h
 *
 * Copyright (c) 2012, 2016 Lin Ke-Fong
 *
 * This code is free, do whatever you want with it.
 */

#include <stdio.h>

 /* Z80's flags. */

#define Z80_S_FLAG_SHIFT        7
#define Z80_Z_FLAG_SHIFT        6
#define Z80_Y_FLAG_SHIFT        5
#define Z80_H_FLAG_SHIFT        4
#define Z80_X_FLAG_SHIFT        3
#define Z80_PV_FLAG_SHIFT       2
#define Z80_N_FLAG_SHIFT        1
#define Z80_C_FLAG_SHIFT        0

#define Z80_S_FLAG              (1 << Z80_S_FLAG_SHIFT)
#define Z80_Z_FLAG              (1 << Z80_Z_FLAG_SHIFT)
#define Z80_Y_FLAG              (1 << Z80_Y_FLAG_SHIFT)
#define Z80_H_FLAG              (1 << Z80_H_FLAG_SHIFT)
#define Z80_X_FLAG              (1 << Z80_X_FLAG_SHIFT)
#define Z80_PV_FLAG             (1 << Z80_PV_FLAG_SHIFT)
#define Z80_N_FLAG              (1 << Z80_N_FLAG_SHIFT)
#define Z80_C_FLAG              (1 << Z80_C_FLAG_SHIFT)

#define Z80_P_FLAG_SHIFT        Z80_PV_FLAG_SHIFT
#define Z80_V_FLAG_SHIFT        Z80_PV_FLAG_SHIFT
#define Z80_P_FLAG              Z80_PV_FLAG
#define Z80_V_FLAG              Z80_PV_FLAG

static void make_szyxp_flags_table (void)
{
        int     i;

        printf("static const unsigned char SZYXP_FLAGS_TABLE[256] = {\n");
        for (i = 0; i < 256; i++) {

                int     j, p, r;

                j = i;
                p = !0;
                while (j) {

                        if (j & 1)

                                p = !p;

                        j >>= 1;

                }
                r = i & (Z80_S_FLAG | Z80_Y_FLAG | Z80_X_FLAG);
                r |= !i ? Z80_Z_FLAG : 0;
                r |= p ? Z80_PV_FLAG : 0;

                if (!(i & 7))

                        printf("\n\t0x%02x, ", r);

                else

                        printf("0x%02x, ", r);

        }
        printf("\n\n};\n");
}

int main()
{
	make_szyxp_flags_table();
	return 0;
}