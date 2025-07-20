#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <conio.h>
#include <windows.h>
#include <math.h>  

#define MAX_SNAKE_LENGTH 1000
#define MAX_FOOD 200  
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

    // 计算场地面积，用于智能适配Q键加速
    int fieldArea = (W - 2) * (H - 2);
    // 基础速度固定（避免影响刷新）
    int baseSpeed = 100;
    // Q键加速速度：场地越大，加速后速度越低（加速越明显）
    // 公式：面积每增加200，加速速度降低10（最低20）
    int qSpeed = baseSpeed - (fieldArea / 200) * 10;
    if (qSpeed < 20) qSpeed = 20;  // 限制最低加速速度
    int speed = baseSpeed;

    // 食物数量计算
    int targetFoodCount = fieldArea / 50;
    if (targetFoodCount < 5) targetFoodCount = 5;

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
    int currentFoodCount = 0;
    while (currentFoodCount < targetFoodCount && currentFoodCount < MAX_FOOD)
    {
        int idx = rand() % MAX_FOOD;
        if (!foods[idx].active)
        {
            int valid;
            do
            {
                valid = 1;
                foods[idx].x = rand() % (W - 2) + 1;
                foods[idx].y = rand() % (H - 2) + 1;

                // 检查与蛇重叠
                for (int j = 0; j < snakeLength; j++)
                {
                    if (foods[idx].x == snakeX[j] && foods[idx].y == snakeY[j])
                    {
                        valid = 0;
                        break;
                    }
                }

                // 检查与其他食物重叠（分散分布）
                for (int j = 0; j < MAX_FOOD; j++)
                {
                    if (idx != j && foods[j].active)
                    {
                        int distX = abs(foods[idx].x - foods[j].x);
                        int distY = abs(foods[idx].y - foods[j].y);
                        if (distX < 3 && distY < 3)
                        {
                            valid = 0;
                            break;
                        }
                    }
                }
            } while (!valid);

            foods[idx].lifetime = FOOD_LIFETIME + rand() % 30;
            foods[idx].active = 1;
            currentFoodCount++;
        }
    }

    int gameOver = 0;
    int score = 0;

    //光标隐藏
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO cursorInfo;
    GetConsoleCursorInfo(hOut, &cursorInfo);
    cursorInfo.bVisible = 0;
    SetConsoleCursorInfo(hOut, &cursorInfo);

    //************************************************************************************移动的处理控制*********************************************************************************************************//
    while (!gameOver)
    {
        // 处理输入（Q键加速按场地智能适配）
        if (_kbhit())
        {
            char key = _getch();
            switch (key)
            {
            case 'W': direction = (direction != DOWN) ? UP : direction; break;
            case 'D': direction = (direction != LEFT) ? RIGHT : direction; break;
            case 'S': direction = (direction != UP) ? DOWN : direction; break;
            case 'A': direction = (direction != RIGHT) ? LEFT : direction; break;
            case 'Q': speed = qSpeed; break;  // 使用场地适配的加速速度
            case 'E': gameOver = 1; break;
            case '+': break;
            case '-': break;
            }
        }
        else
        {
            speed = baseSpeed;  // 恢复基础速度
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
                ateFood++;
                currentFoodCount--;

                // 蛇生长
                int grow = rand() % 3 + 1;
                snakeLength = (snakeLength + grow > MAX_SNAKE_LENGTH) ? MAX_SNAKE_LENGTH : snakeLength + grow;
            }
        }

        // 生成新食物,保持目标数量
        while (ateFood > 0 && currentFoodCount < targetFoodCount && currentFoodCount < MAX_FOOD)
        {
            int idx = rand() % MAX_FOOD;
            if (!foods[idx].active)
            {
                int valid;
                do
                {
                    valid = 1;
                    foods[idx].x = rand() % (W - 2) + 1;
                    foods[idx].y = rand() % (H - 2) + 1;

                    // 检查与蛇重叠
                    for (int j = 0; j < snakeLength; j++)
                    {
                        if (foods[idx].x == snakeX[j] && foods[idx].y == snakeY[j])
                        {
                            valid = 0;
                            break;
                        }
                    }

                    // 检查与其他食物重叠（保持分散）
                    for (int j = 0; j < MAX_FOOD; j++)
                    {
                        if (idx != j && foods[j].active)
                        {
                            int distX = abs(foods[idx].x - foods[j].x);
                            int distY = abs(foods[idx].y - foods[j].y);
                            if (distX < 3 && distY < 3)
                            {
                                valid = 0;
                                break;
                            }
                        }
                    }
                } while (!valid);

                foods[idx].lifetime = FOOD_LIFETIME + rand() % 30;
                foods[idx].active = 1;
                currentFoodCount++;
                ateFood--;
            }
        }

        // 更新食物生命周期：过期补充新食物，保持数量
        for (int i = 0; i < MAX_FOOD; i++)
        {
            if (foods[i].active)
            {
                foods[i].lifetime--;
                if (foods[i].lifetime <= 0)
                {
                    foods[i].active = 0;
                    currentFoodCount--;

                    // 补充新食物
                    if (currentFoodCount < targetFoodCount && currentFoodCount < MAX_FOOD)
                    {
                        int valid;
                        do
                        {
                            valid = 1;
                            foods[i].x = rand() % (W - 2) + 1;
                            foods[i].y = rand() % (H - 2) + 1;

                            // 检查与蛇重叠
                            for (int j = 0; j < snakeLength; j++)
                            {
                                if (foods[i].x == snakeX[j] && foods[i].y == snakeY[j])
                                {
                                    valid = 0;
                                    break;
                                }
                            }

                            // 检查与其他食物分散
                            for (int j = 0; j < MAX_FOOD; j++)
                            {
                                if (i != j && foods[j].active)
                                {
                                    int distX = abs(foods[i].x - foods[j].x);
                                    int distY = abs(foods[i].y - foods[j].y);
                                    if (distX < 3 && distY < 3)
                                    {
                                        valid = 0;
                                        break;
                                    }
                                }
                            }
                        } while (!valid);

                        foods[i].lifetime = FOOD_LIFETIME + rand() % 30;
                        foods[i].active = 1;
                        currentFoodCount++;
                    }
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
                            printf("*");  // 食物
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

        // 显示游戏信息（展示场地适配的加速速度）
        printf("分数: %d  长度: %d  基础速度: %d  Q加速速度: %d  食物数量: %d\n",
            score, snakeLength, baseSpeed, qSpeed, currentFoodCount);
        printf("控制: WASD 加速:按住Q（按场地智能适配） 退出:E\n");

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