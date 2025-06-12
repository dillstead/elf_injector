#ifndef INJECT_INFO
#define INJECT_INFO

#define II_NULL            0
#define II_CNK_POS         1
#define II_CNK_LEN         2
#define II_CNK_ENT_OFF     3

struct inject_info
{
    unsigned int type;
    unsigned int val;
};
    
#endif
