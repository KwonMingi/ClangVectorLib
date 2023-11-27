#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "addElements.h"
typedef struct
{
    void *data;
    size_t elementSize;
    size_t capacity;
    size_t size;
} Vector;
/**
 * 초기 용량과 요소 크기를 지정하여 새로운 Vector를 생성.
 * @param initialCapacity Vector의 초기 용량(저장할 수 있는 요소의 수)
 * @param elementSize 저장할 각 요소의 크기(바이트 단위)
 * @return 초기화된 Vector 구조체
 */
Vector createVector(size_t initialCapacity, size_t elementSize)
{
    Vector vec;
    vec.data = malloc(initialCapacity * elementSize);
    if (vec.data == NULL)
    {
        // 메모리 할당 실패 시, 초기화하지 않고 NULL을 반환
        return (Vector){NULL, 0, 0, 0};
    }

    memset(vec.data, 0, initialCapacity * elementSize);

    vec.elementSize = elementSize;
    vec.capacity = initialCapacity;
    vec.size = 0;
    return vec;
}
void *safeRealloc(void *ptr, size_t newSize)
{
    void *newPtr = realloc(ptr, newSize);
    if (newPtr == NULL)
    {
        return NULL;
    }
    return newPtr;
}
/**
 * Vector의 첫 번째 요소를 가리키는 포인터를 반환.
 * @param vec Vector 구조체의 포인터
 * @return Vector의 첫 번째 요소를 가리키는 포인터
 */
void *begin(Vector *vec)
{
    return vec->data;
}
/**
 * Vector의 마지막 요소 다음을 가리키는 포인터를 반환.
 * @param vec Vector 구조체의 포인터
 * @return Vector의 마지막 요소 다음을 가리키는 포인터
 */
void *end(Vector *vec)
{
    return (char *)vec->data + vec->size * vec->elementSize;
}
/**
 * Vector의 요소를 검색.
 * @param vec Vector 구조체의 포인터
 * @param compar 찾을 요소의 값
 */
int find(Vector *vec, void *element)
{
    for (size_t i = 0; i < vec->size; i++)
    {
        if (memcmp((char *)vec->data + i * vec->elementSize, element, vec->elementSize) == 0)
        {
            return i;
        }
    }
    return -1; // Not found
}
/**
 * Vector의 요소를 정렬.
 * @param vec Vector 구조체의 포인터
 * @param compar 비교 함수 포인터
 */
void sort(Vector *vec, int (*compar)(const void *, const void *))
{
    qsort(vec->data, vec->size, vec->elementSize, compar);
}
/**
 * Vector의 요소 순서를 반전.
 * @param vec Vector 구조체의 포인터
 */
void reverse(Vector *vec)
{
    char *left = vec->data;
    char *right = (char *)vec->data + (vec->size - 1) * vec->elementSize;

    char *buffer = (char *)malloc(vec->elementSize);
    if (buffer == NULL)
    {
        printf("Failed to allocate memory\n");
        return;
    }

    while (left < right)
    {
        memcpy(buffer, left, vec->elementSize);
        memcpy(left, right, vec->elementSize);
        memcpy(right, buffer, vec->elementSize);

        left += vec->elementSize;
        right -= vec->elementSize;
    }

    // 사용한 메모리 해제
    free(buffer);
}

/**
 * Vector에 새 요소를 삽입.
 * @param vec Vector 구조체의 포인터
 * @param index 삽입할 위치의 인덱스
 * @param element 삽입할 요소의 포인터
 */
void insert(Vector *vec, size_t index, void *element)
{
    if (index > vec->size)
    {
        printf("Index out of bounds\n");
        return;
    }
    if (vec->size == vec->capacity)
    {
        vec->capacity *= 2;
        vec->data = realloc(vec->data, vec->capacity * vec->elementSize);
    }
    memmove((char *)vec->data + (index + 1) * vec->elementSize,
            (char *)vec->data + index * vec->elementSize,
            (vec->size - index) * vec->elementSize);
    memcpy((char *)vec->data + index * vec->elementSize, element, vec->elementSize);
    vec->size++;
}
/**
 * Vector가 비어있는지 여부를 반환.
 * @param vec Vector 구조체의 포인터
 * @return 비어있으면 true, 아니면 false 반환
 */
bool empty(Vector *vec)
{
    return vec->size == 0;
}

/**
 * Vector의 첫 번째 요소를 반환.
 * @param vec Vector 구조체의 포인터
 * @return 첫 번째 요소를 가리키는 포인터
 */
void *front(Vector *vec)
{
    if (empty(vec))
        return NULL;
    return vec->data;
}
/**
 * Vector의 마지막 요소를 반환.
 * @param vec Vector 구조체의 포인터
 * @return 마지막 요소를 가리키는 포인터
 */
void *back(Vector *vec)
{
    if (empty(vec))
        return NULL;
    return (char *)vec->data + (vec->size - 1) * vec->elementSize;
}

/**
 * Vector의 모든 요소를 제거.
 * @param vec Vector 구조체의 포인터
 */
void clear(Vector *vec)
{
    vec->size = 0;
}

/**
 * Vector의 현재 길이(저장된 요소의 수)를 반환.
 * @param vec Vector 구조체의 포인터
 * @return Vector의 현재 길이
 */
int length(Vector *vec)
{
    return vec->size;
}
/**
 * Vector에 새 요소를 추가.
 * @param vec Vector 구조체의 포인터
 * @param element 추가할 요소의 포인터
 */
void pushBack(Vector *vec, void *element)
{
    if (vec->size == vec->capacity)
    {
        size_t newCapacity = vec->capacity == 0 ? 1 : vec->capacity * 2;
        void *newData = safeRealloc(vec->data, newCapacity * vec->elementSize);
        if (newData == NULL)
        {
            return;
        }
        vec->data = newData;
        vec->capacity = newCapacity;
    }
    memcpy((char *)vec->data + vec->size * vec->elementSize, element, vec->elementSize);
    vec->size++;
}
/**
 * Vector의 특정 위치에 있는 요소를 새 값으로 설정.
 * @param vec Vector 구조체의 포인터
 * @param index 설정할 요소의 위치
 * @param element 새로 설정할 요소의 포인터
 */
void set(Vector *vec, size_t index, void *element)
{
    if (index >= vec->size)
    {
        printf("Index out of bounds\n");
        return;
    }

    memcpy((char *)vec->data + index * vec->elementSize, element, vec->elementSize);
}
/**
 * Vector의 특정 위치에 있는 요소를 반환.
 * @param vec Vector 구조체의 포인터
 * @param index 가져올 요소의 위치
 * @return 요청한 위치의 요소를 가리키는 포인터
 */
void *get(Vector *vec, size_t index)
{
    if (index < vec->size)
    {
        return (char *)vec->data + index * vec->elementSize;
    }
    printf("Index out of bounds\n");
    return NULL;
}
/**
 * Vector에 할당된 메모리를 해제.
 * @param vec Vector 구조체의 포인터
 */
void delete(Vector *vec)
{
    free(vec->data);
    vec->data = NULL;
    vec->capacity = 0;
    vec->size = 0;
}
/**
 * Vector의 마지막 요소를 제거.
 * @param vec Vector 구조체의 포인터
 */
void popBack(Vector *vec)
{
    if (vec->size == 0)
    {
        printf("Vector is empty, cannot pop back.\n");
        return;
    }
    vec->size--;
}
/**
 * Vector의 특정 위치에 있는 요소를 제거.
 * @param vec Vector 구조체의 포인터
 * @param index 제거할 요소의 위치
 */
void removeAt(Vector *vec, size_t index)
{
    if (index >= vec->size)
    {
        printf("Index out of bounds\n");
        return;
    }

    // 요소를 메모리에서 이동시킴
    memmove((char *)vec->data + index * vec->elementSize,
            (char *)vec->data + (index + 1) * vec->elementSize,
            (vec->size - index - 1) * vec->elementSize);

    vec->size--;
}
/**
 * Vector의 크기를 조정.
 * @param vec Vector 구조체의 포인터
 * @param newSize 새로운 크기
 */
void resize(Vector *vec, size_t newSize)
{
    if (newSize > vec->capacity)
    {
        vec->capacity = newSize;
        vec->data = realloc(vec->data, vec->capacity * vec->elementSize);
        if (vec->data == NULL)
        {
            printf("Failed to allocate memory\n");
            exit(EXIT_FAILURE);
        }
    }
    else if (newSize < vec->size)
    {
        vec->size = newSize;
    }
    else
    {
        vec->size = newSize;
    }
}
/**
 * Vector의 내용을 출력.
 * @param vec Vector 구조체의 포인터
 */
void draw(Vector *vec)
{
    printf("{");
    for (int i = 0; i < length(vec); ++i)
    {
        printf("%d", *(int *)get(vec, i));
        if (i < length(vec) - 1)
        {
            printf(", ");
        }
    }
    printf("}\n");
}
/**
 * Char 형 Vector의 문자열을 출력.
 * @param vec Vector 구조체의 포인터
 */
void drawString(Vector *vec)
{
    for (int i = 0; i < length(vec); ++i)
    {
        printf("%c", *(char *)get(vec, i));
    }
    printf("\n");
}
/**
 * Char 형 Vector에 문자열을 입력받습니다.
 * @param vec Vector 구조체의 포인터
 * @param range 입력받을 최대 길이
 */
void getString(Vector *vec, int range)
{
    resize(vec, range);
    printf("Enter a string: ");
    char input;
    int count = 0;

    while (scanf("%c", &input) && input != '\n' && count < range)
    {
        pushBack(vec, &input);
        count++;
    }
    char nullChar = '\0';
    pushBack(vec, &nullChar); // NULL 문자 추가
}
/**
 * 한 Vector의 내용을 다른 Vector에 복사.
 * @param dest 복사 받을 Vector 구조체의 포인터
 * @param src 복사할 Vector 구조체의 포인터
 */
void copyVector(Vector *dest, Vector *src)
{
    if (src->elementSize != dest->elementSize)
    {
        printf("Error: Vectors are not of the same element type\n");
        return;
    }

    if (dest->capacity < src->size)
    {
        dest->capacity = src->size;
        dest->data = realloc(dest->data, dest->capacity * dest->elementSize);
        if (dest->data == NULL)
        {
            printf("Failed to allocate memory\n");
            exit(EXIT_FAILURE);
        }
    }

    memcpy(dest->data, src->data, src->size * src->elementSize);
    dest->size = src->size;
}
// 사용 예시
int main()
{
    // 1차원 Vector사용법
    Vector arr = createVector(1, sizeof(int));
    Vector stringArr = createVector(1, sizeof(char));
    Vector stringArr2 = createVector(1, sizeof(char));

    char *charTemp;
    int temp;

    getString(&stringArr, 20);

    copyVector(&stringArr2, &stringArr);

    pushBack(&arr, addInt(&temp, 10));
    pushBack(&arr, addInt(&temp, 20));
    pushBack(&arr, addInt(&temp, 30));
    pushBack(&arr, addInt(&temp, 40));
    int *p = back(&arr);
    p = p;
    printf("%d\n", *p);

    printf("%d\n", length(&arr));
    draw(&arr);

    popBack(&arr);
    popBack(&arr);
    removeAt(&arr, 0);

    printf("%d\n", length(&arr));
    draw(&arr);
    drawString(&stringArr);
    reverse(&stringArr2);
    drawString(&stringArr2);

    // 2차원 배열 사용법
    int n, m;
    scanf("%d%d", &n, &m);

    // Vector 배열을 동적으로 할당
    Vector *board = malloc(sizeof(Vector) * n);
    if (board == NULL)
    {
        printf("Failed to allocate memory\n");
        exit(EXIT_FAILURE);
    }

    // 각 Vector 요소를 초기화
    for (int i = 0; i < n; ++i)
    {
        board[i] = createVector(m, sizeof(char));
    }

    for (int i = 0; i < n; ++i)
    {
        for (int j = 0; j < m; ++j)
        { // 예를 들어 각 행에 10개의 요소를 추가한다고 가정
            char empty = 'E';
            pushBack(&board[i], &empty);
        }
    }

    for (int i = 0; i < n; ++i)
    {
        printf("Row %d: ", i);
        for (int j = 0; j < m; ++j)
        {
            char *element = (char *)get(&board[i], j);
            printf("%c ", *element);
        }
        printf("\n");
    }

    for (int i = 0; i < n; ++i)
    {
        delete (&board[i]);
    }
    free(board);

    delete (&stringArr);
    delete (&stringArr2);
    delete (&arr);
    return 0;
}
