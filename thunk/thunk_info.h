#ifndef THUNK_INFO
#define THUNK_INFO

#define TNK_LEN            280
// Offset of entry point in thunk
#define TNK_ENT_OFF        172
// Offsets inside of thunk of data that will be patched in
// at injection time (see thunk.c for how each one is used).
#define OFF_HOST_ENT       232
#define OFF_CNK_LEN        236
#define OFF_CNK_OFF        240
#define OFF_CNK_ENT_OFF    244
// Offsets of thunk that store injection information to pass
// along to the chunk.
#define OFF_II_CNK_POS     252
#define OFF_II_CNK_LEN     260
#define OFF_II_CNK_ENT_OFF 268

#endif
