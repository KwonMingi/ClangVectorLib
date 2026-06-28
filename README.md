# C vector 라이브러리 (C++ std::vector 스타일)

C용 제네릭 동적 배열. C++ `std::vector` 와 최대한 비슷한 이름·사용법을 따른다. `void* + elem_size` 방식이라 어떤 타입이든 담을 수 있고, 자주 쓰는 타입을 위한 고속 경로(타입드 매크로)도 함께 제공한다.

원본 라이브러리를 **버그 수정 → 성능 측정 → 구조 개선 → C++ 스타일 API 정리**의 순서로 다듬은 결과물이다.

---

## 0. C++ ↔ 이 라이브러리 대응표

C 에는 템플릿·네임스페이스·연산자 오버로딩이 없으므로, 타입 대신 요소 크기를 넘기고 메서드는 `vector_<name>(&v, ...)` 자유 함수로 제공한다.

| C++ `std::vector<int>` | 이 라이브러리 |
|---|---|
| `std::vector<int> v;` | `vector v = vector_create(sizeof(int));` |
| `v.push_back(x);` | `vector_push_back(&v, &x);` |
| `v.pop_back();` | `vector_pop_back(&v);` |
| `v.size();` | `vector_size(&v);` |
| `v.empty();` | `vector_empty(&v);` |
| `v.capacity();` | `vector_capacity(&v);` |
| `v.reserve(n);` | `vector_reserve(&v, n);` |
| `v.resize(n);` | `vector_resize(&v, n);` |
| `v.shrink_to_fit();` | `vector_shrink_to_fit(&v);` |
| `v.clear();` | `vector_clear(&v);` |
| `v.at(i);` (범위검사) | `vector_at(&v, i);` |
| `v[i];` (비검사) | `VEC_AT(&v, int, i);` |
| `v.front();` / `v.back();` | `vector_front(&v);` / `vector_back(&v);` |
| `v.begin();` / `v.end();` | `vector_begin(&v);` / `vector_end(&v);` |
| `v.data();` | `vector_data(&v);` |
| `v.insert(v.begin()+i, x);` | `vector_insert(&v, i, &x);` |
| `v.erase(v.begin()+i);` | `vector_erase(&v, i);` |
| `a.swap(b);` | `vector_swap(&a, &b);` |
| `a == b` | `vector_equal(&a, &b);` |
| `b = a;` (복사) | `vector_copy(&b, &a);` |
| `std::find(...)` | `vector_find(&v, &x);` → `VEC_NPOS` if absent |
| `std::sort(...)` | `vector_sort(&v, cmp);` |
| `std::reverse(...)` | `vector_reverse(&v);` |
| (소멸자) | `vector_destroy(&v);` |

> **주의 — 생성자 의미 차이**: C++ `std::vector<int> v(n)` 은 *요소 n개*(size==n)를 만든다. 이 라이브러리의 `vector_create` 는 인자로 **요소 개수가 아니라 요소 크기**만 받고 항상 빈 벡터로 시작한다. 미리 자리를 잡고 싶으면 `vector_reserve`(용량) 또는 `vector_resize`(크기)를 쓴다 — 이건 C++ 의 `v.reserve(n)` / `v.resize(n)` 과 정확히 같은 관용구다.

---

## 1. 무엇이 바뀌었나 (요약)

가장 중요한 건 성능이 아니라 **원본이 컴파일/링크부터 안 되고, 자료구조 연산 몇 개가 조용히 틀렸다는 점**이다.

### 빌드를 막던 문제
- `main.c` 가 `delete(&x)` 를 호출 — `delete` 는 C++ 키워드라 C에서는 미정의. `undefined reference to delete` 로 **링크 실패**. → `vector_destroy` 로 교체.
- `vector.h` 가 헤더에 함수 **정의**를 담고 인클루드 가드도 없음. 두 개 이상의 `.c` 에서 include 하면 `multiple definition` **링크 에러**. → 가드 추가 + stb 스타일 구현 분리(2절).

### 조용히 틀려 있던 동작
- **`vector_resize()`** (원본 `resize`): 크기를 늘릴 때 `realloc` 만 하고 `size` 를 갱신하지 않았다. `resize(v,10)` 후에도 `size` 가 그대로라 늘린 영역에 접근 불가. 새 영역 0초기화도 없었다. → `size` 갱신 + 늘어난 `[size, new_size)` 구간만 0으로 채움. 용량만 확보하는 용도는 `vector_reserve()` 로 분리.
- **`vector_reverse()`**: 빈 벡터에서 `size - 1` 이 `size_t` 라 `SIZE_MAX` 로 언더플로 → 와일드 포인터(UB). → `size < 2` 가드.
- **`vector_insert()`**: `data = realloc(data, ...)` 직접 대입 → 실패 시 원본 포인터를 잃어 **누수 + NULL 역참조 크래시**. → 안전한 성장 경로로 통일.
- **용량 계산 오버플로 무방비**: `capacity*2`, `n*elem_size` 오버플로 시 작은 버퍼를 잡고 그 위에 씀(힙 오버플로). → 모든 성장 경로에 `SIZE_MAX` 검사.
- **`vector_size`/`vector_find` 의 정수 폭**: 원본 `length`/`find` 가 `int` 반환이라 거대 벡터에서 잘림. `find` 의 `-1` 도 모호. → `size_t` 로, `find` 는 못 찾으면 `VEC_NPOS`.
- **`vector_get_string()`**: `scanf("%c")` 의 EOF(-1)가 truthy 라 종료 판정이 샘. → `getchar()` + `EOF` 검사.

### 그 밖의 정리
- 진단 메시지를 `stdout` → **`stderr`** 로 (파이프 출력 오염 방지).
- 읽기 전용 연산에 **`const`**.
- 실패 가능 연산(`push_back`/`insert`/`resize`/`reserve`/`copy`)은 **`bool` 반환**.
- 무의미한 `safeRealloc` 래퍼 제거.

---

## 2. 빌드 & 사용법

이 라이브러리는 **stb 스타일 단일 헤더**다. 구현은 헤더 안에 있고 `VECTOR_IMPLEMENTATION` 매크로로 본체를 emit 한다.

### 여러 `.c` 로 나뉜 프로젝트 (권장)
프로젝트 전체에서 **딱 한 개**의 `.c` 가 구현을 emit 한다:

```c
/* vector_impl.c — 이 파일 하나만 */
#define VECTOR_IMPLEMENTATION
#include "vector.h"
```

나머지 파일은 헤더만:

```c
#include "vector.h"   /* 선언 + 인라인 핫패스만 */
```

빌드:
```sh
gcc -O2 main.c vector_impl.c -o demo
```

### 단일 `.c` 프로그램
파일이 하나뿐이면 그 파일 맨 위에서 바로:

```c
#define VECTOR_IMPLEMENTATION
#include "vector.h"
```

이 저장소의 `main.c` + `vector_impl.c` 가 다중 파일 패턴 예시다:
```sh
gcc -Wall -Wextra -O2 main.c vector_impl.c -o demo
printf 'Hello world\n3 4\n' | ./demo
```

---

## 3. 사용 예시

```c
#include "vector.h"
#include "add_elements.h"

int main(void) {
    vector v = vector_create(sizeof(int));   /* std::vector<int> v; */

    int x;
    vector_push_back(&v, add_int(&x, 5));     /* v.push_back(5);     */
    vector_push_back(&v, add_int(&x, 10));

    int *p = (int *)vector_at(&v, 0);         /* v.at(0), OOB면 NULL */
    if (p) *p = 42;

    VEC_AT(&v, int, 1) = 99;                  /* v[1] = 99;          */

    size_t i = vector_find(&v, add_int(&x, 99));
    if (i != VEC_NPOS) printf("found at %zu\n", i);

    vector_destroy(&v);                       /* (소멸자)            */
    return 0;
}
```

> `add_elements.h` 는 임시 변수에 값을 써서 그 주소를 돌려주는 헬퍼 모음(`add_int`, `add_double`, ...)이다. `vector_push_back` 이 포인터를 받으므로 리터럴을 바로 넣을 때 편하다. C99 복합 리터럴 `&(int){5}` 를 써도 되고, 타입을 아는 핫루프라면 `VEC_PUSH(&v, int, 5)` 가 더 빠르다.

---

## 4. 성능 (측정값)

GCC 13.3, `-O2`, x86_64 단일 코어 VM, best-of-N. **상대 비교용이며 절대치는 환경마다 다르다.**

| 항목 | 측정 | 해석 |
|---|---|---|
| `vector_push_back` (제네릭) 20M | ~120 ms | 원본과 **동일**. 같은 memcpy 경로라 빨라질 이유가 없다 — 회귀도 없음. |
| `VEC_PUSH` (타입드) 20M | **~57 ms (다중 TU) / ~37 ms (단일 TU)** | 제네릭 대비 **2.2x / 3.3x**. 타입드 저장이 memcpy+함수호출을 없앤다. |
| `vector_at()` 스트리밍 1M×200 | ~338 ms | 메모리 바운드. 인라인/타입드 여부와 무관. |
| `VEC_AT` 스트리밍 1M×200 | ~337 ms | 위와 **동일** — 큰 데이터 1회 순회는 메모리 대역폭이 한계. |
| `vector_at()` 핫셋(L1) ×400k | ~1030 ms | 범위 검사 분기가 자동 벡터화를 막는다. |
| `VEC_AT` 핫셋(L1) ×400k | **~165 ms** | 벡터화 가능 → **~6.3x**. |
| `vector_find()` 1M×60 | ~170 ms | 같은 알고리즘 → 원본과 동일. |
| `vector_reverse()` 1M×200 | ~800 ms | 같은 알고리즘 → 원본과 동일. |
| `vector_create` 용량 0초기화 | with ~316 ms / without ~0.1 ms | 2000× (1M int 용량) 기준. **큰 용량을 미리 잡을 때만** 차이. |

### 솔직한 결론
- **일반 API(`void*` 경로)는 원본과 같은 속도다.** 마법은 없다. 가치는 *정확성*과 *멀티파일에서 깨지지 않는 구조*, 그리고 *친숙한 C++ 스타일 API* 에 있다.
- **빨라지는 건 "타입을 아는 핫루프에 타입드 매크로를 쓸 때"뿐이다.** 그것도 상황별로:
  - 추가 루프(`VEC_PUSH`): 2~3배. 꽤 안정적.
  - 합산/축약처럼 컴파일러가 **벡터화할 여지가 있을 때**(`VEC_AT`, 작업셋이 캐시에 들어가고 누산기가 `volatile` 이 아닐 때): 최대 ~6배.
  - **큰 데이터를 한 번 훑는 스트리밍**은 메모리 바운드라 무엇을 써도 똑같다.
- `vector_create` 의 용량 0초기화 제거는 **큰 초기 용량을 미리 잡는 경우에만** 의미가 있다.
- `find`/`reverse` 는 알고리즘이 같으니 같은 속도다. 다르게 보였다면 메모리 레이아웃 노이즈다.

재현:
```sh
gcc -O2 bench.c vector_impl.c -o bench && ./bench            # 다중 TU
gcc -O2 -DBENCH_SINGLE_TU bench.c -o bench && ./bench         # 단일 TU
```

---

## 5. 파일

| 파일 | 설명 |
|---|---|
| `vector.h` | 라이브러리 본체(단일 헤더). |
| `vector_impl.c` | 구현 emit용 TU(한 줄). 멀티파일 빌드 시 함께 컴파일. |
| `add_elements.h` | 타입별 값 헬퍼(`add_int` 등). |
| `main.c` | 사용 예시 + 새 기능 데모. |
| `bench.c` | 성능 재현 하니스. |

---

## 6. 원본에서 옮길 때 (네이밍 변경표)

| 원본 | 현재 |
|---|---|
| `Vector` (타입) | `vector` |
| `createVector(cap, sz)` | `vector_create(sz)` + 필요 시 `vector_reserve(&v, cap)` |
| `deleteVector` | `vector_destroy` |
| `pushBack` | `vector_push_back` |
| `popBack` | `vector_pop_back` |
| `length` | `vector_size` (반환형 `int`→`size_t`) |
| `get` (범위검사) | `vector_at` |
| `getUnchecked` | `vector_at_unchecked` (또는 `VEC_AT` 매크로) |
| `set` | (제거) `vector_at`/`VEC_AT` 로 직접 대입 |
| `insert` | `vector_insert` |
| `removeAt` | `vector_erase` |
| `find` (`-1`) | `vector_find` (`VEC_NPOS`) |
| `sort` / `reverse` | `vector_sort` / `vector_reverse` |
| `copyVector` | `vector_copy` |
| `swapVector` | `vector_swap` |
| `vectorEqual` | `vector_equal` |
| `forEach` | `vector_for_each` |
| `reserveVector` | `vector_reserve` |
| `shrinkToFit` | `vector_shrink_to_fit` |
| `draw` / `drawString` | `vector_draw` / `vector_draw_string` |
| `getString` | `vector_get_string` |
| `addInt`, `addDouble`, ... | `add_int`, `add_double`, ... |
| `addElements.h` (파일) | `add_elements.h` |
