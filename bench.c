#define _POSIX_C_SOURCE 199309L
#include <time.h>
#include "vector.h"
/* 단일-TU 빌드일 때만 본체를 여기서 emit (-DBENCH_SINGLE_TU) */
#ifdef BENCH_SINGLE_TU
#define VECTOR_IMPLEMENTATION
#include "vector.h"
#endif

static double now(void){ struct timespec t; clock_gettime(CLOCK_MONOTONIC,&t);
    return t.tv_sec + t.tv_nsec*1e-9; }

int main(void){
    const size_t N   = 20000000;
    const size_t M   = 1000000;
    const int    Kg  = 200;
    const int    Kf  = 60;
    const int    Kr  = 200;
    volatile long sink = 0; double t;

    /* 1) push_back 제네릭 (원본과 동일 경로) */
    vector v = vector_create(sizeof(int));
    t = now();
    for (size_t i=0;i<N;i++){ int x=(int)i; vector_push_back(&v,&x); }
    { double e=now()-t; printf("push_back(generic) %8zu    : %8.1f ms  (%5.1f M/s)\n",
        N, e*1e3, N/1e6/e); }
    vector_destroy(&v);

    /* 1b) VEC_PUSH 타입드 매크로 */
    vector vt = vector_create(sizeof(int));
    t = now();
    for (size_t i=0;i<N;i++){ VEC_PUSH(&vt, int, (int)i); }
    { double e=now()-t; printf("push_back(VEC_PUSH) %8zu   : %8.1f ms  (%5.1f M/s)\n",
        N, e*1e3, N/1e6/e); }

    /* 2) vector_at() 제네릭(범위검사) 합산 */
    vector g = vector_create(sizeof(int));
    vector_reserve(&g, M);
    for (size_t i=0;i<M;i++){ int x=(int)i; vector_push_back(&g,&x);}
    t=now();
    for(int k=0;k<Kg;k++) for(size_t i=0;i<M;i++) sink += *(int*)vector_at(&g,i);
    printf("vector_at(generic) %8zu x%-3d: %8.1f ms\n", M,Kg,(now()-t)*1e3);

    /* 2b) VEC_AT 타입드 접근 합산 */
    t=now();
    for(int k=0;k<Kg;k++) for(size_t i=0;i<M;i++) sink += VEC_AT(&g,int,i);
    printf("VEC_AT(typed)      %8zu x%-3d: %8.1f ms\n", M,Kg,(now()-t)*1e3);

    /* 3) vector_find absent */
    int absent=-1;
    t=now();
    for(int k=0;k<Kf;k++) sink += (long)vector_find(&g,&absent);
    printf("vector_find        %8zu x%-3d: %8.1f ms\n", M,Kf,(now()-t)*1e3);

    /* 4) vector_reverse */
    t=now();
    for(int k=0;k<Kr;k++) vector_reverse(&g);
    printf("vector_reverse     %8zu x%-3d: %8.1f ms\n", M,Kr,(now()-t)*1e3);

    vector_destroy(&vt); vector_destroy(&g);
    fprintf(stderr,"sink=%ld\n",(long)sink);
    return 0;
}
