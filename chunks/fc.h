#ifndef FC_H
#define FC_H

#define FC_1(arr, a1) (arr)[0] = (a1)
#define FC_2(arr, a1, a2) (arr)[0] = (a1); (arr)[1] = (a2)
#define FC_3(arr, a1, a2, a3) (arr)[0] = (a1); (arr)[1] = (a2); (arr)[2] = (a3)
#define FC_4(arr, a1, a2, a3, a4) (arr)[0] = (a1); (arr)[1] = (a2); (arr)[2] = (a3); (arr)[3] = (a4)
#define FC_5(arr, a1, a2, a3, a4, a5) (arr)[0] = (a1); (arr)[1] = (a2); (arr)[2] = (a3); (arr)[3] = (a4); (arr)[4] = (a5)
#define FC_6(arr, a1, a2, a3, a4, a5, a6) (arr)[0] = (a1); (arr)[1] = (a2); (arr)[2] = (a3); (arr)[3] = (a4); (arr)[4] = (a5); (arr)[5] = (a6)
#define FC_7(arr, a1, a2, a3, a4, a5, a6, a7) (arr)[0] = (a1); (arr)[1] = (a2); (arr)[2] = (a3); (arr)[3] = (a4); (arr)[4] = (a5); (arr)[5] = (a6); (arr)[6] = (a7)
#define FC_8(arr, a1, a2, a3, a4, a5, a6, a7, a8) (arr)[0] = (a1); (arr)[1] = (a2); (arr)[2] = (a3); (arr)[3] = (a4); (arr)[4] = (a5); (arr)[5] = (a6); (arr)[6] = (a7); (arr)[7] = (a8)
#define FC_9(arr, a1, a2, a3, a4, a5, a6, a7, a8, a9) (arr)[0] = (a1); (arr)[1] = (a2); (arr)[2] = (a3); (arr)[3] = (a4); (arr)[4] = (a5); (arr)[5] = (a6); (arr)[6] = (a7); (arr)[7] = (a8); (arr)[8] = (a9)
#define FC_10(arr, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10) (arr)[0] = (a1); (arr)[1] = (a2); (arr)[2] = (a3); (arr)[3] = (a4); (arr)[4] = (a5); (arr)[5] = (a6); (arr)[6] = (a7); (arr)[7] = (a8); (arr)[8] = (a9); (arr)[9] = (a10)
#define FC_11(arr, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11) (arr)[0] = (a1); (arr)[1] = (a2); (arr)[2] = (a3); (arr)[3] = (a4); (arr)[4] = (a5); (arr)[5] = (a6); (arr)[6] = (a7); (arr)[7] = (a8); (arr)[8] = (a9); (arr)[9] = (a10); (arr)[10] = (a11)
#define FC_12(arr, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12) (arr)[0] = (a1); (arr)[1] = (a2); (arr)[2] = (a3); (arr)[3] = (a4); (arr)[4] = (a5); (arr)[5] = (a6); (arr)[6] = (a7); (arr)[7] = (a8); (arr)[8] = (a9); (arr)[9] = (a10); (arr)[10] = (a11); (arr)[11] = (a12)

#define GET_FC_MACRO(_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,NAME,...) NAME
#define FILL_CHARS(arr, ...) GET_FC_MACRO(__VA_ARGS__, FC_12, FC_11, FC_10, FC_9, FC_8, FC_7, FC_6, FC_5, FC_4, FC_3, FC_2, FC_1)(arr, __VA_ARGS__)

#endif
