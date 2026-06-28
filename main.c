/*
 * main.c -- vector 라이브러리 사용 예시 (C++ std::vector 스타일).
 *
 *  이 파일은 라이브러리를 "그냥 쓰는" 쪽이라 헤더만 include 한다.
 *  라이브러리 본체(구현)는 vector_impl.c 단 한 곳에서만 emit 된다.
 *  빌드:  gcc -O2 main.c vector_impl.c -o demo
 *
 *  (단일 .c 프로그램이라면 vector_impl.c 없이, 이 파일 맨 위에서
 *   #define VECTOR_IMPLEMENTATION 후 vector.h 를 include 해도 된다.)
 */
#include "vector.h"
#include "add_elements.h"

static int cmp_int_asc(const void *a, const void *b)
{
    int x = *(const int *)a, y = *(const int *)b;
    return (x > y) - (x < y);
}

int main(void)
{
    /* ---------- 1차원 vector 사용법 ---------- */
    vector arr         = vector_create(sizeof(int));
    vector string_arr  = vector_create(sizeof(char));
    vector string_arr2 = vector_create(sizeof(char));

    int temp;

    vector_get_string(&string_arr, 20);
    vector_copy(&string_arr2, &string_arr);

    vector_push_back(&arr, add_int(&temp, 10));
    vector_push_back(&arr, add_int(&temp, 20));
    vector_push_back(&arr, add_int(&temp, 30));
    vector_push_back(&arr, add_int(&temp, 40));

    int *p = vector_back(&arr);
    printf("back = %d\n", *p);
    printf("size = %zu\n", vector_size(&arr));   /* %zu : size_t */
    vector_draw(&arr);

    vector_pop_back(&arr);
    vector_pop_back(&arr);
    vector_erase(&arr, 0);

    printf("size = %zu\n", vector_size(&arr));
    vector_draw(&arr);
    vector_draw_string(&string_arr);
    vector_reverse(&string_arr2);
    vector_draw_string(&string_arr2);

    /* ---------- 새 기능 데모 ---------- */
    printf("\n-- new features --\n");

    /* (a) VEC_PUSH: 타입드 고속 추가 + vector_reserve 로 재할당 0회 */
    vector nums = vector_create(sizeof(int));
    vector_reserve(&nums, 8);
    for (int i = 5; i >= 1; --i)
        VEC_PUSH(&nums, int, i * i);     /* 25 16 9 4 1 */
    vector_draw(&nums);

    /* (b) vector_sort + vector_find(센티넬 VEC_NPOS) */
    vector_sort(&nums, cmp_int_asc);     /* 1 4 9 16 25 */
    vector_draw(&nums);
    int key = 9;
    size_t idx = vector_find(&nums, &key);
    if (idx != VEC_NPOS) printf("found %d at index %zu\n", key, idx);
    int missing = 7;
    printf("find(7) -> %s\n",
           vector_find(&nums, &missing) == VEC_NPOS ? "VEC_NPOS (없음)" : "found");

    /* (c) vector_pop_back_into: 꺼내면서 값 회수 */
    int popped;
    if (vector_pop_back_into(&nums, &popped))
        printf("popped %d, now size=%zu\n", popped, vector_size(&nums));

    /* (d) VEC_AT: 타입드 직접 접근(lvalue) == v[i] */
    VEC_AT(&nums, int, 0) = 100;
    vector_draw(&nums);

    vector_destroy(&nums);

    /* ---------- 2차원 배열 사용법 ---------- */
    int n, m;
    if (scanf("%d%d", &n, &m) != 2 || n <= 0 || m <= 0)
    {
        fprintf(stderr, "expected two positive integers n m\n");
        n = m = 0;
    }

    vector *board = malloc(sizeof(vector) * (size_t)n);
    if (board == NULL && n > 0)
    {
        fprintf(stderr, "Failed to allocate memory\n");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < n; ++i)
    {
        board[i] = vector_create(sizeof(char));
        vector_reserve(&board[i], (size_t)m);   /* C++: v.reserve(m) */
    }

    for (int i = 0; i < n; ++i)
        for (int j = 0; j < m; ++j)
        {
            char empty = 'E';
            vector_push_back(&board[i], &empty);
        }

    for (int i = 0; i < n; ++i)
    {
        printf("Row %d: ", i);
        for (int j = 0; j < m; ++j)
            printf("%c ", *(char *)vector_at(&board[i], (size_t)j));
        printf("\n");
    }

    for (int i = 0; i < n; ++i)
        vector_destroy(&board[i]);
    free(board);

    vector_destroy(&string_arr);
    vector_destroy(&string_arr2);
    vector_destroy(&arr);
    return 0;
}
