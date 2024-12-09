
#if 0
void LoadUserCode(void) {
  int i;
  for (i=0;i<CODE_SIZE;i++) {
    WriteVS10xxRegister(atab[i], dtab[i]);
  }
}
#endif

#define CODE_SIZE 113
const unsigned char atab[113] = { /* Register addresses */
    0x7, 0x6, 0x6, 0x6, 0x6, 0x6, 0x6, 0x6, 0x6, 0x6, 0x6, 0x6, 0x6, 0x6, 0x6, 0x6,
    0x6, 0x6, 0x6, 0x6, 0x6, 0x6, 0x6, 0x6, 0x6, 0x6, 0x6, 0x6, 0x6, 0x6, 0x6, 0x6,
    0x6, 0x6, 0x6, 0x6, 0x6, 0x6, 0x6, 0x6, 0x6, 0x6, 0x6, 0x6, 0x6, 0x6, 0x6, 0x6,
    0x6, 0x6, 0x6, 0x6, 0x6, 0x6, 0x6, 0x6, 0x6, 0x6, 0x6, 0x6, 0x6, 0x6, 0x6, 0x6,
    0x6, 0x6, 0x6, 0x6, 0x6, 0x6, 0x6, 0x6, 0x6, 0x6, 0x6, 0x6, 0x6, 0x6, 0x6, 0x6,
    0x6, 0x6, 0x6, 0x6, 0x6, 0x6, 0x6, 0x6, 0x6, 0x6, 0x6, 0x6, 0x6, 0x6, 0x6, 0x6,
    0x6, 0x6, 0x6, 0x6, 0x6, 0x6, 0x6, 0x6, 0x6, 0x6, 0x6, 0x6, 0x6, 0x6, 0x6, 0x6,
    0x6
};
const unsigned short dtab[113] = { /* Data to write */
    0x8f00, 0x2803, 0xc2c0, 0x0030, 0x0697, 0x0fff, 0xfdc0, 0x3700,
    0x4024, 0xb100, 0x0024, 0xbc82, 0x3c00, 0x0030, 0x1297, 0x3f10,
    0x0024, 0x3f00, 0x0024, 0x2803, 0xc540, 0x0003, 0xc795, 0x0000,
    0x0200, 0x3700, 0x4024, 0xc100, 0x0024, 0x3f00, 0x0024, 0x0000,
    0x0040, 0x0004, 0x07c1, 0x0003, 0xc7d5, 0x0030, 0x1097, 0x3f70,
    0x0024, 0x3f00, 0x4024, 0xf400, 0x5540, 0x0000, 0x08d7, 0xf400,
    0x57c0, 0x0007, 0x9257, 0x0000, 0x0000, 0x3f00, 0x0024, 0x0030,
    0x0297, 0x2000, 0x0000, 0x3f00, 0x0024, 0x2a08, 0x2bce, 0x2a03,
    0xc80e, 0x3e12, 0xb817, 0x3e14, 0xf806, 0x3e00, 0xb804, 0x0030,
    0x0317, 0x3701, 0x0024, 0x0030, 0x10d7, 0x0030, 0x1293, 0xf148,
    0x1c46, 0xa64c, 0x428a, 0x3700, 0x8024, 0x2803, 0xcc11, 0xa244,
    0x0024, 0xf168, 0x0024, 0x464c, 0x0024, 0xf128, 0x0024, 0x4244,
    0x0024, 0x3b11, 0x8024, 0x3b00, 0x8024, 0x36f0, 0x9804, 0x36f4,
    0xd806, 0x3602, 0x8024, 0x0030, 0x0717, 0x2100, 0x0000, 0x3f05,
    0xdbd7
};