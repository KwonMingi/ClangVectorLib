/*
 * vector.h -- C용 제네릭 동적 배열 (C++ std::vector 스타일 API)
 * ---------------------------------------------------------------------------
 *  C++ 의 std::vector 와 최대한 비슷한 이름/사용법을 따른다. 단, C 에는
 *  템플릿/네임스페이스/연산자 오버로딩이 없으므로:
 *    - 타입 대신 요소 크기(elem_size)를 생성 시 넘긴다.
 *    - 메서드는 vector_<name>(&v, ...) 형태의 자유 함수다.
 *    - v[i] 같은 첨자는 매크로 VEC_AT(&v, type, i) 로 대신한다.
 *
 *  C++ <-> 이 라이브러리 대응:
 *    std::vector<int> v;          ->  vector v = vector_create(sizeof(int));
 *    v.push_back(x);              ->  vector_push_back(&v, &x);
 *    v.size();                    ->  vector_size(&v);
 *    v.at(i);                     ->  vector_at(&v, i);        // 범위검사
 *    v[i];                        ->  VEC_AT(&v, int, i);      // 비검사, lvalue
 *    v.reserve(n);                ->  vector_reserve(&v, n);
 *    (소멸자)                      ->  vector_destroy(&v);
 *
 * ---------------------------------------------------------------------------
 *  빌드 (stb 스타일 단일 헤더):
 *
 *   - 일반 소스 파일: 그냥 include (선언 + 인라인 핫패스).
 *         #include "vector.h"
 *
 *   - 프로젝트 전체에서 "정확히 한 개"의 .c 에서만, include 직전에 구현
 *     매크로를 정의해 본체를 emit 한다.
 *         #define VECTOR_IMPLEMENTATION
 *         #include "vector.h"
 *
 *     이렇게 하면 여러 파일에서 include 해도 중복 정의(ODR) 링크 에러가 없다.
 */
#ifndef VECTOR_H
#define VECTOR_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>   /* SIZE_MAX */

#ifdef __cplusplus
extern "C" {
#endif

/* vector_find() 가 못 찾았을 때 돌려주는 센티넬. (size_t 라 -1 은 SIZE_MAX) */
#define VEC_NPOS ((size_t)-1)

typedef struct
{
    void  *data;       /* 요소 버퍼 (capacity*elem_size 바이트)              */
    size_t elem_size;  /* 요소 1개의 크기(바이트)                            */
    size_t capacity;   /* 재할당 없이 담을 수 있는 요소 수                   */
    size_t size;       /* 현재 담긴 요소 수                                 */
} vector;

/* ===================== 선언부 (모든 TU에서 보임) ===================== */

/* 생성/소멸 */
vector vector_create(size_t elem_size);   /* 빈 벡터(지연 할당). std::vector<T> v; */
void   vector_destroy(vector *v);         /* 버퍼 해제 후 0 리셋. (소멸자)         */

/* 용량/크기 관리 */
bool   vector_reserve(vector *v, size_t new_capacity);  /* 용량만 확보(데이터 보존) */
bool   vector_resize(vector *v, size_t new_size);       /* 크기 변경(증가분 0초기화)*/
void   vector_shrink_to_fit(vector *v);                 /* capacity -> size 로 축소 */
bool   vector_grow(vector *v);                          /* 내부 2배 성장(매크로용)  */

/* 추가/삭제 */
bool   vector_push_back(vector *v, const void *element);
void   vector_pop_back(vector *v);
bool   vector_pop_back_into(vector *v, void *out);      /* 꺼내면서 값 복사(비표준) */
bool   vector_insert(vector *v, size_t index, const void *element);
void   vector_erase(vector *v, size_t index);           /* C++ erase(pos)           */
void   vector_clear(vector *v);

/* 조회/검색/알고리즘
 *  (find/sort/reverse 는 std::vector 멤버가 아니라 <algorithm> 자유함수지만
 *   원본 호환 + 편의를 위해 멤버 스타일로 제공한다.) */
size_t vector_find(const vector *v, const void *element); /* 못 찾으면 VEC_NPOS    */
void   vector_sort(vector *v, int (*compar)(const void *, const void *));
void   vector_reverse(vector *v);
void   vector_swap(vector *a, vector *b);                /* C++ swap                 */
bool   vector_copy(vector *dest, const vector *src);     /* C++ 복사 대입 / assign   */
bool   vector_equal(const vector *a, const vector *b);   /* C++ operator==           */
void   vector_for_each(vector *v,
                       void (*fn)(void *element, size_t index, void *user),
                       void *user);

/* 타입 특화 편의 함수 (원본 호환 유지, std 와 무관) */
void   vector_draw(vector *v);         /* int 벡터를 {a, b, c} 형태로 출력          */
void   vector_draw_string(vector *v);  /* char 벡터를 문자열로 출력                 */
void   vector_get_string(vector *v, int range); /* stdin 에서 문자열 읽기           */

/* ===================== 인라인 핫패스 ===================== */

static inline size_t vector_size(const vector *v)     { return v->size; }
static inline bool   vector_empty(const vector *v)    { return v->size == 0; }
static inline size_t vector_capacity(const vector *v) { return v->capacity; }
static inline void  *vector_data(vector *v)           { return v->data; }

static inline void *vector_begin(vector *v) { return v->data; }
static inline void *vector_end(vector *v)
{
    return (char *)v->data + v->size * v->elem_size;
}

/* 범위 검사를 하지 않는 초고속 접근(인덱스 유효성은 호출자 보장). C++ operator[]. */
static inline void *vector_at_unchecked(vector *v, size_t index)
{
    return (char *)v->data + index * v->elem_size;
}

/* 범위 검사 접근. OOB 면 stderr 에 알리고 NULL 반환. C++ at() (throw 대신 NULL). */
static inline void *vector_at(vector *v, size_t index)
{
    if (index >= v->size)
    {
        fprintf(stderr, "vector_at: index %zu out of bounds (size=%zu)\n",
                index, v->size);
        return NULL;
    }
    return (char *)v->data + index * v->elem_size;
}

static inline void *vector_front(vector *v)
{
    return vector_empty(v) ? NULL : v->data;
}
static inline void *vector_back(vector *v)
{
    return vector_empty(v) ? NULL
                           : (char *)v->data + (v->size - 1) * v->elem_size;
}

/* ===================== 타입드 매크로(제네릭 memcpy 우회) ===================== */
/*
 * 요소 타입을 아는 핫루프에서 void* + memcpy(elem_size) 경로를 건너뛰고
 * 직접 타입 저장/로드를 한다. elem_size == sizeof(type) 이어야 한다(호출자 책임).
 *
 *   VEC_AT(&v, int, i)        -> i 번째 요소를 int& 로 (lvalue, 범위검사 X) == v[i]
 *   VEC_PUSH(&v, int, 42)     -> 42 를 끝에 추가(필요시 자동 성장) == push_back
 *
 * 멀티파일 빌드(LTO 미사용)에서 일반 vector_push_back/vector_at 은 함수 호출
 * 경계 뒤라 인라인되지 못하지만, 이 매크로는 항상 인라인되어 훨씬 빠르다.
 */
#define VEC_AT(vecptr, type, index) (((type *)(vecptr)->data)[(index)])

#define VEC_PUSH(vecptr, type, value)                                    \
    do {                                                                 \
        vector *vec__ = (vecptr);                                        \
        if (vec__->size < vec__->capacity || vector_grow(vec__))         \
        {                                                                \
            ((type *)vec__->data)[vec__->size++] = (value);              \
        }                                                                \
    } while (0)

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* VECTOR_H */


/* ===========================================================================
 *                            구 현 부 (IMPLEMENTATION)
 *  정확히 한 개의 .c 에서 #define VECTOR_IMPLEMENTATION 후 include.
 * ======================================================================== */
#ifdef VECTOR_IMPLEMENTATION
#ifndef VECTOR_IMPLEMENTATION_DONE
#define VECTOR_IMPLEMENTATION_DONE

#ifdef __cplusplus
extern "C" {
#endif

/* 빈/실패 vector (요소 크기는 유지해 후속 reserve 가 동작하도록). */
static vector vector__empty(size_t elem_size)
{
    vector v;
    v.data = NULL;
    v.elem_size = elem_size;
    v.capacity = 0;
    v.size = 0;
    return v;
}

vector vector_create(size_t elem_size)
{
    /* 지연 할당: 버퍼는 첫 push_back/reserve 때 잡는다.
     * (std::vector<T> v; 처럼 size==0, capacity==0 으로 시작.) */
    return vector__empty(elem_size);
}

void vector_destroy(vector *v)
{
    free(v->data);
    v->data = NULL;
    v->capacity = 0;
    v->size = 0;
}

bool vector_reserve(vector *v, size_t new_capacity)
{
    if (new_capacity <= v->capacity)
        return true;
    if (v->elem_size != 0 && new_capacity > SIZE_MAX / v->elem_size)
        return false; /* 오버플로 */

    void *p = realloc(v->data, new_capacity * v->elem_size);
    if (p == NULL)
        return false; /* 실패 시 기존 v->data 는 그대로 유효(누수 없음) */

    v->data = p;
    v->capacity = new_capacity;
    return true;
}

bool vector_grow(vector *v)
{
    size_t new_cap;
    if (v->capacity == 0)
        new_cap = 1;
    else if (v->capacity > SIZE_MAX / 2)
        new_cap = (v->elem_size ? SIZE_MAX / v->elem_size : v->capacity);
    else
        new_cap = v->capacity * 2;

    if (new_cap <= v->capacity)
        return false; /* 더 키울 수 없음 */
    return vector_reserve(v, new_cap);
}

bool vector_resize(vector *v, size_t new_size)
{
    if (new_size > v->capacity)
    {
        if (!vector_reserve(v, new_size))
            return false;
    }
    if (new_size > v->size)
    {
        /* 새로 늘어난 [size, new_size) 구간만 0 초기화 */
        memset((char *)v->data + v->size * v->elem_size, 0,
               (new_size - v->size) * v->elem_size);
    }
    v->size = new_size; /* 원본은 커질 때 size 를 갱신하지 않는 버그가 있었다 */
    return true;
}

void vector_shrink_to_fit(vector *v)
{
    if (v->size == v->capacity)
        return;
    if (v->size == 0)
    {
        free(v->data);
        v->data = NULL;
        v->capacity = 0;
        return;
    }
    void *p = realloc(v->data, v->size * v->elem_size);
    if (p != NULL) /* 실패해도 기능상 문제는 없으니 조용히 유지 */
    {
        v->data = p;
        v->capacity = v->size;
    }
}

bool vector_push_back(vector *v, const void *element)
{
    if (v->size == v->capacity)
    {
        if (!vector_grow(v))
            return false;
    }
    memcpy((char *)v->data + v->size * v->elem_size,
           element, v->elem_size);
    v->size++;
    return true;
}

void vector_pop_back(vector *v)
{
    if (v->size == 0)
    {
        fprintf(stderr, "vector_pop_back: empty\n");
        return;
    }
    v->size--;
}

bool vector_pop_back_into(vector *v, void *out)
{
    if (v->size == 0)
        return false;
    if (out != NULL)
        memcpy(out, (char *)v->data + (v->size - 1) * v->elem_size,
               v->elem_size);
    v->size--;
    return true;
}

bool vector_insert(vector *v, size_t index, const void *element)
{
    if (index > v->size)
    {
        fprintf(stderr, "vector_insert: index %zu out of bounds (size=%zu)\n",
                index, v->size);
        return false;
    }
    if (v->size == v->capacity)
    {
        if (!vector_grow(v)) /* 원본은 raw realloc 결과를 바로 대입해
                                실패 시 포인터를 잃어버렸다(누수+크래시) */
            return false;
    }
    memmove((char *)v->data + (index + 1) * v->elem_size,
            (char *)v->data + index * v->elem_size,
            (v->size - index) * v->elem_size);
    memcpy((char *)v->data + index * v->elem_size,
           element, v->elem_size);
    v->size++;
    return true;
}

void vector_erase(vector *v, size_t index)
{
    if (index >= v->size)
    {
        fprintf(stderr, "vector_erase: index %zu out of bounds (size=%zu)\n",
                index, v->size);
        return;
    }
    memmove((char *)v->data + index * v->elem_size,
            (char *)v->data + (index + 1) * v->elem_size,
            (v->size - index - 1) * v->elem_size);
    v->size--;
}

void vector_clear(vector *v) { v->size = 0; }

size_t vector_find(const vector *v, const void *element)
{
    const char *base = (const char *)v->data;
    for (size_t i = 0; i < v->size; i++)
    {
        if (memcmp(base + i * v->elem_size, element, v->elem_size) == 0)
            return i;
    }
    return VEC_NPOS;
}

void vector_sort(vector *v, int (*compar)(const void *, const void *))
{
    if (v->size > 1)
        qsort(v->data, v->size, v->elem_size, compar);
}

void vector_reverse(vector *v)
{
    if (v->size < 2)
        return; /* 빈/단일 요소 가드: 원본은 size==0 일 때 size-1 이
                   SIZE_MAX 로 언더플로해 와일드 포인터를 만들었다(UB). */

    /* 작은 요소는 스택 버퍼로 처리해 호출당 malloc/free 를 피한다. */
    char  stack_buf[64];
    char *buffer = (v->elem_size <= sizeof(stack_buf))
                       ? stack_buf
                       : (char *)malloc(v->elem_size);
    if (buffer == NULL)
    {
        fprintf(stderr, "vector_reverse: allocation failed\n");
        return;
    }

    char *left  = (char *)v->data;
    char *right = (char *)v->data + (v->size - 1) * v->elem_size;
    while (left < right)
    {
        memcpy(buffer, left, v->elem_size);
        memcpy(left, right, v->elem_size);
        memcpy(right, buffer, v->elem_size);
        left  += v->elem_size;
        right -= v->elem_size;
    }

    if (buffer != stack_buf)
        free(buffer);
}

void vector_swap(vector *a, vector *b)
{
    vector t = *a;
    *a = *b;
    *b = t;
}

bool vector_copy(vector *dest, const vector *src)
{
    if (src->elem_size != dest->elem_size)
    {
        fprintf(stderr, "vector_copy: element size mismatch (%zu vs %zu)\n",
                dest->elem_size, src->elem_size);
        return false;
    }
    if (dest->capacity < src->size)
    {
        if (!vector_reserve(dest, src->size))
            return false;
    }
    if (src->size > 0) /* memcpy 에 NULL 전달 회피 */
        memcpy(dest->data, src->data, src->size * src->elem_size);
    dest->size = src->size;
    return true;
}

bool vector_equal(const vector *a, const vector *b)
{
    if (a->elem_size != b->elem_size || a->size != b->size)
        return false;
    if (a->size == 0)
        return true;
    return memcmp(a->data, b->data, a->size * a->elem_size) == 0;
}

void vector_for_each(vector *v,
                     void (*fn)(void *element, size_t index, void *user),
                     void *user)
{
    for (size_t i = 0; i < v->size; i++)
        fn((char *)v->data + i * v->elem_size, i, user);
}

void vector_draw(vector *v)
{
    putchar('{');
    for (size_t i = 0; i < v->size; ++i)
    {
        printf("%d", *(int *)vector_at_unchecked(v, i));
        if (i + 1 < v->size)
            printf(", ");
    }
    printf("}\n");
}

void vector_draw_string(vector *v)
{
    for (size_t i = 0; i < v->size; ++i)
        putchar(*(char *)vector_at_unchecked(v, i));
    putchar('\n');
}

void vector_get_string(vector *v, int range)
{
    if (range < 0)
        range = 0;
    /* range 글자 + NULL 1 글자 만큼 미리 확보 -> 입력 중 재할당 0회 */
    vector_reserve(v, (size_t)range + 1);

    printf("Enter a string: ");
    fflush(stdout);

    int count = 0, c;
    /* scanf("%c") 대신 getchar: EOF(-1)가 truthy 라 원본은 종료 판정이
       샜다. 여기선 EOF/개행/한도에서 정확히 멈춘다. */
    while (count < range && (c = getchar()) != EOF && c != '\n')
    {
        char ch = (char)c;
        vector_push_back(v, &ch);
        count++;
    }
    char nul = '\0';
    vector_push_back(v, &nul);
}

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* VECTOR_IMPLEMENTATION_DONE */
#endif /* VECTOR_IMPLEMENTATION */
