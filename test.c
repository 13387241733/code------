#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <conio.h>
#include <windows.h>

#define MAX_SNAKE_LENGTH 1000
#define MAX_FOOD 5
#define FOOD_LIFETIME 50

//方向控制
#define UP    0
#define RIGHT 1
#define DOWN  2
#define LEFT  3

int main()
{
    //*****************************************************************************绘制游戏场地**************************************************************************************************//
    int W, H;
    char fill_char = '#';
    printf("请输入宽度（若为弹出的cmd窗口的默认大小，最大为120；若全屏cmd，则最高156）: ");
    if (scanf_s("%d", &W) != 1 || W < 2)
    {
        printf("宽度必须至少为2!\n");
        return 1;
    }
    printf("请输入高度（建议43）: ");
    if (scanf_s("%d", &H) != 1 || H < 2)
    {
        printf("高度必须至少为2!\n");
        return 1;
    }
    // 输出顶部
    for (int i = 0; i < W; i++)
    {
        printf("%c", fill_char);
    }
    printf("\n");
    // 输出中间
    for (int i = 0; i < H - 2; i++)
    {
        printf("%c", fill_char);
        for (int j = 0; j < W - 2; j++)
        {
            printf(" ");
        }
        printf("%c\n", fill_char);
    }
    // 输出底部
    for (int i = 0; i < W; i++)
    {
        printf("%c", fill_char);
    }
    printf("\n");

    //********************************************************************************蛇的绘制**********************************************************************************************************//
    srand(time(NULL));
    int snakeX[MAX_SNAKE_LENGTH];
    int snakeY[MAX_SNAKE_LENGTH];
    int snakeLength = 3;
    int direction = RIGHT;
    //位置
    for (int i = 0; i < snakeLength; i++)
    {
        snakeX[i] = W / 2 - i;
        snakeY[i] = H / 2;
    }

    //*********************************************************************************食物的产生***************************************************************************************************************//
    struct Food
    {
        int x, y;
        int lifetime;
        int active;
    } foods[MAX_FOOD];
    for (int i = 0; i < MAX_FOOD; i++)
    {
        foods[i].active = 0;
    }
    //初始食物
    for (int i = 0; i < 2; i++)
    {
        int idx = rand() % MAX_FOOD;
        if (!foods[idx].active)
        {
            foods[idx].x = rand() % (W - 2) + 1;
            foods[idx].y = rand() % (H - 2) + 1;
            foods[idx].lifetime = FOOD_LIFETIME;
            foods[idx].active = 1;
        }
    }

    int gameOver = 0;
    int score = 0;
    int speed = 100;
    //光标隐藏
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO cursorInfo;
    GetConsoleCursorInfo(hOut, &cursorInfo);
    cursorInfo.bVisible = 0;
    SetConsoleCursorInfo(hOut, &cursorInfo);

    //************************************************************************************移动的处理控制*********************************************************************************************************//
    while (!gameOver)
    {
        // 处理输入（修改为大写 WASD）
        if (_kbhit())
        {
            char key = _getch();
            switch (key)
            {
            case 'W': direction = (direction != DOWN) ? UP : direction; break;
            case 'D': direction = (direction != LEFT) ? RIGHT : direction; break;
            case 'S': direction = (direction != UP) ? DOWN : direction; break;
            case 'A': direction = (direction != RIGHT) ? LEFT : direction; break;
            case 'Q': gameOver = 1; break;
            case '+': if (speed > 20) speed -= 10; break;
            case '-': if (speed < 300) speed += 10; break;
            }
        }

        // 移动蛇
        for (int i = snakeLength - 1; i > 0; i--)
        {
            snakeX[i] = snakeX[i - 1];
            snakeY[i] = snakeY[i - 1];
        }

        switch (direction)
        {
        case UP:    snakeY[0]--; break;
        case RIGHT: snakeX[0]++; break;
        case DOWN:  snakeY[0]++; break;
        case LEFT:  snakeX[0]--; break;
        }

        // 墙
        if (snakeX[0] <= 0 || snakeX[0] >= W - 1 || snakeY[0] <= 0 || snakeY[0] >= H - 1)
        {
            gameOver = 1;
        }

        // 检查撞到自己
        for (int i = 1; i < snakeLength; i++)
        {
            if (snakeX[0] == snakeX[i] && snakeY[0] == snakeY[i])
            {
                gameOver = 1;
            }
        }

        // 检查吃食物
        int ateFood = 0;
        for (int i = 0; i < MAX_FOOD; i++)
        {
            if (foods[i].active && snakeX[0] == foods[i].x && snakeY[0] == foods[i].y)
            {
                foods[i].active = 0;
                score += 10;
                ateFood = 1;

                int grow = rand() % 3 + 1;
                snakeLength = (snakeLength + grow > MAX_SNAKE_LENGTH) ? MAX_SNAKE_LENGTH : snakeLength + grow;
            }
        }

        // 生成新食物
        if (ateFood)
        {
            for (int i = 0; i < MAX_FOOD; i++)
            {
                if (!foods[i].active)
                {
                    int valid;
                    do
                    {
                        valid = 1;
                        foods[i].x = rand() % (W - 2) + 1;
                        foods[i].y = rand() % (H - 2) + 1;

                        // 检查是否与蛇重叠
                        for (int j = 0; j < snakeLength; j++)
                        {
                            if (foods[i].x == snakeX[j] && foods[i].y == snakeY[j])
                            {
                                valid = 0;
                                break;
                            }
                        }

                        // 检查是否与其他食物重叠
                        for (int j = 0; j < MAX_FOOD; j++)
                        {
                            if (i != j && foods[j].active &&
                                foods[i].x == foods[j].x && foods[i].y == foods[j].y)
                            {
                                valid = 0;
                                break;
                            }
                        }
                    } while (!valid);

                    foods[i].lifetime = FOOD_LIFETIME;
                    foods[i].active = 1;
                    break;
                }
            }
        }

        // 更新食物
        for (int i = 0; i < MAX_FOOD; i++)
        {
            if (foods[i].active)
            {
                foods[i].lifetime--;
                if (foods[i].lifetime <= 0)
                {
                    foods[i].active = 0;

                    int valid;
                    do
                    {
                        valid = 1;
                        foods[i].x = rand() % (W - 2) + 1;
                        foods[i].y = rand() % (H - 2) + 1;

                        // 检查是否与蛇重叠
                        for (int j = 0; j < snakeLength; j++)
                        {
                            if (foods[i].x == snakeX[j] && foods[i].y == snakeY[j])
                            {
                                valid = 0;
                                break;
                            }
                        }

                        // 检查是否与其他食物重叠
                        for (int j = 0; j < MAX_FOOD; j++)
                        {
                            if (i != j && foods[j].active &&
                                foods[i].x == foods[j].x && foods[i].y == foods[j].y)
                            {
                                valid = 0;
                                break;
                            }
                        }
                    } while (!valid);

                    foods[i].lifetime = FOOD_LIFETIME;
                    foods[i].active = 1;
                }
            }
        }

        // 清屏
        system("cls");

        // 绘制顶部边框
        for (int i = 0; i < W; i++)
        {
            printf("%c", fill_char);
        }
        printf("\n");

        // 绘制中间部分
        for (int y = 1; y < H - 1; y++)
        {
            printf("%c", fill_char);

            for (int x = 1; x < W - 1; x++)
            {
                int isSnake = 0;

                // 绘制蛇
                for (int i = 0; i < snakeLength; i++)
                {
                    if (snakeX[i] == x && snakeY[i] == y)
                    {
                        if (i == 0)
                        {
                            printf("O");  // 蛇头
                        }
                        else
                        {
                            printf("o");  // 蛇身
                        }
                        isSnake = 1;
                        break;
                    }
                }

                // 绘制食物
                if (!isSnake)
                {
                    int hasFood = 0;
                    for (int i = 0; i < MAX_FOOD; i++)
                    {
                        if (foods[i].active && foods[i].x == x && foods[i].y == y)
                        {
                            printf("*");  // 食物（半角字符 *）
                            hasFood = 1;
                            break;
                        }
                    }

                    if (!hasFood)
                    {
                        printf(" ");
                    }
                }
            }

            printf("%c\n", fill_char);
        }

        // 绘制底部边框
        for (int i = 0; i < W; i++)
        {
            printf("%c", fill_char);
        }
        printf("\n");

        // 显示游戏信息（修改为大写 WASD）
        printf("分数: %d  长度: %d  速度: %d\n", score, snakeLength, 310 - speed);
        printf("控制: WASD 加速:+ 减速:- 退出:Q\n");  // 改为大写 WASD

        // 控制游戏速度
        Sleep(speed);
    }

    // 游戏结束
    system("cls");
    printf("\n\n");
    printf("  游戏结束！最终分数: %d\n\n", score);
    printf("        **********\n");
    printf("        * GAME   *\n");
    printf("        * OVER   *\n");
    printf("        **********\n\n");

    return 0;
}