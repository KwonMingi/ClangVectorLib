#include "vector.h"
#include "addElements.h"
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
