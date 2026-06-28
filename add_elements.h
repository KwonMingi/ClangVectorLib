/*
 * add_elements.h -- 리터럴 값의 주소를 손쉽게 만들기 위한 도우미.
 * ---------------------------------------------------------------------------
 *  vector_push_back 등은 void* 를 받으므로 임시 변수의 주소가 필요하다.
 *  add_int(&tmp, 10) 은 tmp 에 10 을 넣고 &tmp 를 돌려준다.
 *
 *  [참고] C99 이상에서는 복합 리터럴(compound literal)로 임시 변수 없이
 *         바로 주소를 넘길 수 있어 이 헤더 없이도 된다:
 *             vector_push_back(&v, &(int){10});
 *             vector_push_back(&v, &(double){3.14});
 *         타입 매크로를 쓰면 더 빠르다:
 *             VEC_PUSH(&v, int, 10);
 */
#ifndef ADD_ELEMENTS_H
#define ADD_ELEMENTS_H

#include <stdbool.h>

static inline int *add_int(int *value_ptr, int value)
{
    *value_ptr = value;
    return value_ptr;
}
static inline long *add_long(long *value_ptr, long value)
{
    *value_ptr = value;
    return value_ptr;
}
static inline long long *add_long_long(long long *value_ptr, long long value)
{
    *value_ptr = value;
    return value_ptr;
}
static inline double *add_double(double *value_ptr, double value)
{
    *value_ptr = value;
    return value_ptr;
}
static inline long double *add_long_double(long double *value_ptr, long double value)
{
    *value_ptr = value;
    return value_ptr;
}
static inline short *add_short(short *value_ptr, short value)
{
    *value_ptr = value;
    return value_ptr;
}
static inline char *add_char(char *value_ptr, char value)
{
    *value_ptr = value;
    return value_ptr;
}
static inline bool *add_bool(bool *value_ptr, bool value)
{
    *value_ptr = value;
    return value_ptr;
}
static inline float *add_float(float *value_ptr, float value)
{
    *value_ptr = value;
    return value_ptr;
}

#endif /* ADD_ELEMENTS_H */
