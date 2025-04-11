#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

int main(void)
{
    // 乱数生成用に初期化
    srand((unsigned int)time(NULL));
    char filename[256];
    printf(".csvを抜いた形のファイル名を入力してください\n");
    scanf("%s", filename);

    // サイズ入力
    int size;
    while (1 == 1)
    {
        printf("縦、横の大きさを入力してください  ");
        scanf(" %d", &size);

        if (size <= 24 && size >= 4)
        {
            break;
        }
        printf("正常な入力ではありません\n");
    }

    // フィールド用の二次元配列を用意
    int **field = (int **)malloc(sizeof(int *) * size);
    for (int i = 0; i < size; i++)
    {
        field[i] = (int *)malloc(sizeof(int) * size);
    }

    //-1で初期化
    for (int i = 0; i < size; i++)
    {
        for (int j = 0; j < size; j++)
        {
            field[i][j] = -1;
        }
    }

    // 0からフィールドのマス数÷２－１までランダムな座標2つに番号を入れる
    for (int number = 0; number <= (int)(size * size / 2) - 1; number++)
    {
        int x, y;
        // 二つの座標両方埋まっていないことを確認するまで繰り返す
        while (1 == 1)
        {
            x = rand() % size;
            y = rand() % size;

            if (field[x][y] < 0)
            {
                break;
            }
        }
        field[x][y] = number;
        while (1 == 1)
        {
            x = rand() % size;
            y = rand() % size;

            if (field[x][y] < 0)
            {
                break;
            }
        }
        field[x][y] = number;
    }

    // 生成した問題を表示し、csvファイルにも書き出す
    strcat(filename, ".csv");
    FILE *fp = fopen(filename, "w");

    for (int i = 0; i < size; i++)
    {
        for (int j = 0; j < size; j++)
        {
            printf("%3d ", field[j][i]);
            if (j == size - 1)
            {
                fprintf(fp, "%d\n", field[j][i]);
            }
            else
            {
                fprintf(fp, "%d,", field[j][i]);
            }
        }
        printf("\n");
    }

    // メモリを解放
    for (int i = 0; i < size; i++)
    {
        free(field[i]);
    }
    free(field);

    fclose(fp);
    return 0;
}