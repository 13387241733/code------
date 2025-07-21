#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <conio.h>
#include <windows.h>
#include <math.h>  

#define MAX_SNAKE_LENGTH 1000
#define MAX_FOOD 200  
#define FOOD_LIFETIME 50  
#define MAX_ENEMY_SNAKES 3  
#define INVINCIBLE_TIME 5000  
#define RESPAWN_TIME 5000  
#define LEVEL5_WIDTH 150  
#define LEVEL5_HEIGHT 40  
#define LEVEL5_TIMER 90  

// 方向控制
#define UP    0
#define RIGHT 1
#define DOWN  2
#define LEFT  3

// 食物类型
#define FOOD_WHITE 0
#define FOOD_RED 1
#define FOOD_GREEN 2

// 移动光标到指定位置
void gotoxy(int x, int y) {
    COORD coord;
    coord.X = x;
    coord.Y = y;
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
}

// 设置文本颜色
void setColor(int color) {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hConsole, color);
}

// 蛇结构体
typedef struct {
    int x[MAX_SNAKE_LENGTH];
    int y[MAX_SNAKE_LENGTH];
    int length;
    int direction;
    int speed;
    int baseSpeed;
    int isPlayer;
    int score;
    int lives;
    int maxLives;
    int invincible;
    DWORD invincibleEndTime;
    int respawning;
    DWORD respawnEndTime;
    char headChar;
    char bodyChar;
    int color;
} Snake;

// 食物结构体
typedef struct {
    int x, y;
    int lifetime;
    int active;
    int type;  // 0:白色, 1:红色, 2:绿色
    int forPlayer;  // 1:玩家食物(*), 0:电脑食物($)
} Food;

int main()
{
    // 初始化随机数种子
    srand(time(NULL));

    // 难度等级选择
    int difficulty;
    system("cls");
    printf("==================== 难度选择 ====================\n");
    printf("游戏场地尺寸建议：以弹出的窗口全屏游玩，场地150*40");
    printf("0级：基础模式\n");
    printf("   - 仅白色食物*，吃后增加长度和10分\n");
    printf("   - 无敌人、无生命系统，撞墙或自撞直接结束\n\n");

    printf("1级：多色食物模式\n");
    printf("   - 在0级基础上，新增红色*（扣1条命）和绿色*（加1条命）\n");
    printf("   - 初始1条命，上限10条命，满命时绿色*可获30分\n");
    printf("   - 扣命后获得5秒无敌时间\n\n");

    printf("2级：加速惩罚模式\n");
    printf("   - 继承1级所有规则\n");
    printf("   - 按Q加速时，蛇长度缩短1节并扣5分（长度≥3时生效）\n\n");

    printf("3级：单敌人模式\n");
    printf("   - 继承2级所有规则\n");
    printf("   - 新增1条紫色电脑蛇，以$为食物（规则同*）\n");
    printf("   - 电脑蛇有攻击性，蛇头撞对方身体则扣命并重生\n");
    printf("   - 击杀敌人可获其当前积分的50%%，敌人死亡5秒后重生\n\n");

    printf("4级：双敌人模式\n");
    printf("   - 继承3级所有规则\n");
    printf("   - 电脑蛇数量增加到2条，攻击欲望更强\n\n");

    printf("5级：限时生存模式\n");
    printf("   - 继承4级所有规则，强制场地为150*40\n");
    printf("   - 电脑蛇数量增加到3条，初始生命额外+1，无生命上限\n");
    printf("   - 倒计时1分30秒，结束时玩家积分需高于至少1条电脑蛇才算获胜\n\n");

    printf("请输入难度等级(0-5)：");
    if (scanf_s("%d", &difficulty) != 1) {
        printf("无效输入，默认使用0级\n");
        difficulty = 0;
    }
    if (difficulty < 0 || difficulty > 5) {
        printf("无效等级，默认使用0级\n");
        difficulty = 0;
    }
    getchar();  // 清除输入缓冲区

    // 绘制游戏场地
    int W, H;
    char fill_char = '#';
    if (difficulty == 5) {
        W = LEVEL5_WIDTH;
        H = LEVEL5_HEIGHT;
        printf("第5级强制使用%d*%d的游戏场地\n", W, H);
        Sleep(1000);
    }
    else {
        printf("请输入宽度（默认最大120，全屏156）: ");
        if (scanf_s("%d", &W) != 1 || W < 5) {
            printf("宽度必须至少为5!\n");
            return 1;
        }
        printf("请输入高度（建议43）: ");
        if (scanf_s("%d", &H) != 1 || H < 5) {
            printf("高度必须至少为5!\n");
            return 1;
        }
        getchar();  // 清除输入缓冲区
    }

    // 隐藏光标
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO cursorInfo;
    GetConsoleCursorInfo(hOut, &cursorInfo);
    cursorInfo.bVisible = 0;
    SetConsoleCursorInfo(hOut, &cursorInfo);

    // 计算场地参数
    int fieldArea = (W - 2) * (H - 2);
    int baseSpeed = 100;
    int qSpeed = baseSpeed - (fieldArea / 200) * 10;
    if (qSpeed < 20) qSpeed = 20;

    int targetFoodLevel0 = fieldArea / 50;
    if (targetFoodLevel0 < 5) targetFoodLevel0 = 5;
    int targetFoodCount = targetFoodLevel0;

    // 初始化玩家蛇
    Snake player;
    player.length = 3;
    player.direction = RIGHT;
    player.baseSpeed = baseSpeed;
    player.speed = baseSpeed;
    player.isPlayer = 1;
    player.score = 0;
    player.invincible = 0;
    player.invincibleEndTime = 0;
    player.respawning = 0;
    player.respawnEndTime = 0;
    player.headChar = 'O';
    player.bodyChar = 'o';
    player.color = 7;  // 白色
    if (difficulty == 5) {
        player.lives = 2;
        player.maxLives = -1;  // 无上限
    }
    else {
        player.lives = 1;
        player.maxLives = 10;
    }
    for (int i = 0; i < player.length; i++) {
        player.x[i] = W / 2 - i;
        player.y[i] = H / 2;
    }

    // 初始化敌人蛇
    Snake enemySnakes[MAX_ENEMY_SNAKES];
    int enemyCount = 0;
    if (difficulty == 3) enemyCount = 1;
    else if (difficulty == 4) enemyCount = 2;
    else if (difficulty == 5) enemyCount = 3;

    for (int i = 0; i < enemyCount; i++) {
        enemySnakes[i].length = 3;
        enemySnakes[i].direction = LEFT;
        enemySnakes[i].baseSpeed = baseSpeed + 20;
        enemySnakes[i].speed = enemySnakes[i].baseSpeed;
        enemySnakes[i].isPlayer = 0;
        enemySnakes[i].score = 0;
        enemySnakes[i].lives = 1;
        enemySnakes[i].maxLives = 1;
        enemySnakes[i].invincible = 0;
        enemySnakes[i].invincibleEndTime = 0;
        enemySnakes[i].respawning = 0;
        enemySnakes[i].respawnEndTime = 0;
        enemySnakes[i].headChar = '@';
        enemySnakes[i].bodyChar = '#';
        enemySnakes[i].color = 13;  // 紫色

        // 初始位置错开，避免重叠
        enemySnakes[i].x[0] = W / (3 + i);
        enemySnakes[i].y[0] = H / (3 + i);
        for (int j = 1; j < enemySnakes[i].length; j++) {
            enemySnakes[i].x[j] = enemySnakes[i].x[j - 1] + 1;
            enemySnakes[i].y[j] = enemySnakes[i].y[j - 1];
        }
    }

    // 初始化食物
    Food foods[MAX_FOOD];
    for (int i = 0; i < MAX_FOOD; i++) foods[i].active = 0;
    int currentFoodCount = 0;
    while (currentFoodCount < targetFoodCount && currentFoodCount < MAX_FOOD) {
        int idx = rand() % MAX_FOOD;
        if (!foods[idx].active) {
            int valid = 0;
            do {
                valid = 1;
                foods[idx].x = rand() % (W - 2) + 1;
                foods[idx].y = rand() % (H - 2) + 1;
                foods[idx].lifetime = FOOD_LIFETIME + rand() % 30;
                foods[idx].type = 0;
                if (difficulty >= 1) {
                    int r = rand() % 10;
                    if (r < 2) foods[idx].type = 1;  // 20%红色
                    else if (r < 3) foods[idx].type = 2;  // 10%绿色
                }
                // 根据难度和概率分配食物归属
                if (difficulty >= 3) {
                    // 玩家食物和敌人食物各占50%
                    foods[idx].forPlayer = rand() % 2 == 0;
                }
                else {
                    // 难度低于3级时，所有食物都是玩家的
                    foods[idx].forPlayer = 1;
                }

                // 检查食物是否与蛇重叠
                if (player.x[0] == foods[idx].x && player.y[0] == foods[idx].y) valid = 0;
                for (int j = 1; j < player.length; j++) {
                    if (player.x[j] == foods[idx].x && player.y[j] == foods[idx].y) valid = 0;
                }
                for (int k = 0; k < enemyCount; k++) {
                    if (!enemySnakes[k].respawning) {
                        if (enemySnakes[k].x[0] == foods[idx].x && enemySnakes[k].y[0] == foods[idx].y) valid = 0;
                        for (int j = 1; j < enemySnakes[k].length; j++) {
                            if (enemySnakes[k].x[j] == foods[idx].x && enemySnakes[k].y[j] == foods[idx].y) valid = 0;
                        }
                    }
                }
            } while (!valid);

            foods[idx].active = 1;
            currentFoodCount++;
        }
    }

    // 初始化游戏界面
    system("cls");
    // 绘制边界
    for (int i = 0; i < W; i++) {
        gotoxy(i, 0);
        setColor(15);
        printf("%c", fill_char);
        gotoxy(i, H - 1);
        printf("%c", fill_char);
    }
    for (int i = 0; i < H; i++) {
        gotoxy(0, i);
        printf("%c", fill_char);
        gotoxy(W - 1, i);
        printf("%c", fill_char);
    }

    // 显示食物归属提示
    gotoxy(0, H);
    setColor(7);
    printf("玩家食物: ");
    setColor(15); printf("白色*");
    setColor(12); printf(" 红色*");
    setColor(10); printf(" 绿色*");
    setColor(7);
    printf("    敌人食物: ");
    setColor(15); printf("白色$");
    setColor(12); printf(" 红色$");
    setColor(10); printf(" 绿色$");

    // 显示初始分数
    gotoxy(W - 20, H);
    setColor(7);
    printf("分数: %d", player.score);

    // 显示生命
    gotoxy(W - 40, H);
    setColor(7);
    printf("生命: %d", player.lives);

    // 显示难度
    gotoxy(W - 60, H);
    setColor(7);
    printf("难度: %d级", difficulty);

    // 显示控制说明
    gotoxy(0, H + 1);
    setColor(7);
    printf("控制: W(上) S(下) A(左) D(右) Q(加速)");

    // 绘制初始蛇
    setColor(player.color);
    for (int i = 0; i < player.length; i++) {
        gotoxy(player.x[i], player.y[i]);
        if (i == 0) printf("%c", player.headChar);
        else printf("%c", player.bodyChar);
    }

    // 绘制初始食物
    for (int i = 0; i < MAX_FOOD; i++) {
        if (foods[i].active) {
            setColor(foods[i].type == 0 ? 15 : (foods[i].type == 1 ? 12 : 10));
            gotoxy(foods[i].x, foods[i].y);
            printf("%c", foods[i].forPlayer ? '*' : '$');
        }
    }

    // 绘制初始敌人
    for (int k = 0; k < enemyCount; k++) {
        setColor(enemySnakes[k].color);
        for (int i = 0; i < enemySnakes[k].length; i++) {
            gotoxy(enemySnakes[k].x[i], enemySnakes[k].y[i]);
            if (i == 0) printf("%c", enemySnakes[k].headChar);
            else printf("%c", enemySnakes[k].bodyChar);
        }
    }

    // 游戏主循环
    char key = 'D';  // 默认向右
    DWORD lastTime = GetTickCount();
    int gameOver = 0;
    int level5Timer = LEVEL5_TIMER * 1000;  // 转换为毫秒
    DWORD level5StartTime = GetTickCount();

    while (!gameOver) {
        // 处理键盘输入
        if (_kbhit()) {
            key = _getch();
            // 玩家控制（大写WASD）
            if (key == 'W' && player.direction != DOWN) player.direction = UP;
            if (key == 'D' && player.direction != LEFT) player.direction = RIGHT;
            if (key == 'S' && player.direction != UP) player.direction = DOWN;
            if (key == 'A' && player.direction != RIGHT) player.direction = LEFT;
            // 加速功能
            if (key == 'Q') {
                if (player.length >= 3) {
                    player.length--;
                    player.score -= 5;
                    if (player.score < 0) player.score = 0;
                    gotoxy(W - 20, H);
                    setColor(7);
                    printf("分数: %d", player.score);
                }
                player.speed = qSpeed;
            }
            else {
                player.speed = player.baseSpeed;
            }
        }

        // 控制游戏速度
        DWORD currentTime = GetTickCount();
        if (currentTime - lastTime < player.speed) continue;
        lastTime = currentTime;

        // 清除玩家蛇旧位置
        for (int i = 0; i < player.length; i++) {
            gotoxy(player.x[i], player.y[i]);
            printf(" ");
        }

        // 移动玩家蛇（尾部跟随头部）
        for (int i = player.length - 1; i > 0; i--) {
            player.x[i] = player.x[i - 1];
            player.y[i] = player.y[i - 1];
        }

        // 移动蛇头
        switch (player.direction) {
        case UP:    player.y[0]--; break;
        case RIGHT: player.x[0]++; break;
        case DOWN:  player.y[0]++; break;
        case LEFT:  player.x[0]--; break;
        }

        // 检查无敌状态
        if (player.invincible && currentTime >= player.invincibleEndTime) {
            player.invincible = 0;
        }

        // 处理重生
        if (player.respawning) {
            if (currentTime >= player.respawnEndTime) {
                player.respawning = 0;
                // 重生位置初始化
                player.x[0] = W / 2;
                player.y[0] = H / 2;
                player.length = 3;
                for (int i = 1; i < player.length; i++) {
                    player.x[i] = player.x[i - 1] - 1;
                    player.y[i] = player.y[i - 1];
                }
                player.direction = RIGHT;
                player.invincible = 1;
                player.invincibleEndTime = currentTime + INVINCIBLE_TIME;
            }
            else {
                continue;
            }
        }

        // 碰撞检测（撞墙/自撞）
        int collision = 0;
        if (player.x[0] <= 0 || player.x[0] >= W - 1 || player.y[0] <= 0 || player.y[0] >= H - 1) {
            collision = 1;
        }
        if (!player.invincible) {
            for (int i = 1; i < player.length; i++) {
                if (player.x[0] == player.x[i] && player.y[0] == player.y[i]) {
                    collision = 1;
                    break;
                }
            }
        }

        // 处理碰撞结果
        if (collision) {
            if (difficulty >= 1) {
                player.lives--;
                if (player.lives <= 0) {
                    gameOver = 1;
                }
                else {
                    player.respawning = 1;
                    player.respawnEndTime = currentTime + RESPAWN_TIME;
                }
                gotoxy(W - 40, H);
                setColor(7);
                printf("生命: %d", player.lives);
            }
            else {
                gameOver = 1;
            }
        }

        if (gameOver) continue;

        // 检测玩家吃食物
        int ateFood = 0;
        for (int i = 0; i < MAX_FOOD; i++) {
            if (foods[i].active && player.x[0] == foods[i].x && player.y[0] == foods[i].y) {
                if (foods[i].forPlayer) {
                    // 正确食物处理
                    switch (foods[i].type) {
                    case FOOD_WHITE:
                        player.length++;
                        player.score += 10;
                        break;
                    case FOOD_RED:
                        if (difficulty >= 1) {
                            player.lives--;
                            if (player.lives <= 0) gameOver = 1;
                            else {
                                player.invincible = 1;
                                player.invincibleEndTime = currentTime + INVINCIBLE_TIME;
                            }
                            gotoxy(W - 40, H);
                            setColor(7);
                            printf("生命: %d", player.lives);
                        }
                        break;
                    case FOOD_GREEN:
                        if (difficulty >= 1) {
                            if (player.maxLives == -1 || player.lives < player.maxLives) {
                                player.lives++;
                                gotoxy(W - 40, H);
                                setColor(7);
                                printf("生命: %d", player.lives);
                            }
                            else {
                                player.score += 30;
                            }
                        }
                        break;
                    }
                }
                else {
                    // 错误食物（扣10分）
                    player.score -= 10;
                    if (player.score < 0) player.score = 0;
                }
                foods[i].active = 0;
                currentFoodCount--;
                ateFood = 1;
                gotoxy(W - 20, H);
                setColor(7);
                printf("分数: %d", player.score);
                break;
            }
        }

        // 吃到食物后增长尾部
        if (ateFood) {
            int tailX = player.x[player.length - 1];
            int tailY = player.y[player.length - 1];
            int prevX = player.x[player.length - 2];
            int prevY = player.y[player.length - 2];

            // 根据移动方向添加尾部
            if (tailX > prevX) player.x[player.length] = tailX + 1;
            else if (tailX < prevX) player.x[player.length] = tailX - 1;
            else player.x[player.length] = tailX;

            if (tailY > prevY) player.y[player.length] = tailY + 1;
            else if (tailY < prevY) player.y[player.length] = tailY - 1;
            else player.y[player.length] = tailY;

            player.length++;
        }

        // 生成新食物
        while (currentFoodCount < targetFoodCount && currentFoodCount < MAX_FOOD) {
            int idx = rand() % MAX_FOOD;
            if (!foods[idx].active) {
                int valid = 0;
                do {
                    valid = 1;
                    foods[idx].x = rand() % (W - 2) + 1;
                    foods[idx].y = rand() % (H - 2) + 1;
                    foods[idx].lifetime = FOOD_LIFETIME + rand() % 30;
                    foods[idx].type = 0;
                    if (difficulty >= 1) {
                        int r = rand() % 10;
                        if (r < 2) foods[idx].type = 1;
                        else if (r < 3) foods[idx].type = 2;
                    }
                    foods[idx].forPlayer = (difficulty >= 3) ? (rand() % 2 == 0) : 1;

                    // 避免食物与蛇重叠
                    if (player.x[0] == foods[idx].x && player.y[0] == foods[idx].y) valid = 0;
                    for (int j = 1; j < player.length; j++) {
                        if (player.x[j] == foods[idx].x && player.y[j] == foods[idx].y) valid = 0;
                    }
                    for (int k = 0; k < enemyCount; k++) {
                        if (!enemySnakes[k].respawning) {
                            if (enemySnakes[k].x[0] == foods[idx].x && enemySnakes[k].y[0] == foods[idx].y) valid = 0;
                            for (int j = 1; j < enemySnakes[k].length; j++) {
                                if (enemySnakes[k].x[j] == foods[idx].x && enemySnakes[k].y[j] == foods[idx].y) valid = 0;
                            }
                        }
                    }
                } while (!valid);

                foods[idx].active = 1;
                currentFoodCount++;
                setColor(foods[idx].type == 0 ? 15 : (foods[idx].type == 1 ? 12 : 10));
                gotoxy(foods[idx].x, foods[idx].y);
                printf("%c", foods[idx].forPlayer ? '*' : '$');
            }
        }

        // 绘制玩家蛇新位置
        if (player.invincible) {
            setColor((currentTime / 100) % 2 == 0 ? 14 : player.color);
        }
        else {
            setColor(player.color);
        }
        for (int i = 0; i < player.length; i++) {
            gotoxy(player.x[i], player.y[i]);
            printf("%c", i == 0 ? player.headChar : player.bodyChar);
        }

        // 处理敌人蛇
        for (int k = 0; k < enemyCount; k++) {
            if (enemySnakes[k].respawning) {
                if (currentTime >= enemySnakes[k].respawnEndTime) {
                    enemySnakes[k].respawning = 0;
                    // 敌人重生
                    enemySnakes[k].x[0] = W / (4 + k);
                    enemySnakes[k].y[0] = H / (4 + k);
                    enemySnakes[k].length = 3;
                    for (int i = 1; i < enemySnakes[k].length; i++) {
                        enemySnakes[k].x[i] = enemySnakes[k].x[i - 1] + 1;
                        enemySnakes[k].y[i] = enemySnakes[k].y[i - 1];
                    }
                    enemySnakes[k].direction = LEFT;
                    enemySnakes[k].invincible = 1;
                    enemySnakes[k].invincibleEndTime = currentTime + INVINCIBLE_TIME;
                }
                else {
                    continue;
                }
            }

            // 清除敌人蛇旧位置
            for (int i = 0; i < enemySnakes[k].length; i++) {
                gotoxy(enemySnakes[k].x[i], enemySnakes[k].y[i]);
                printf(" ");
            }

            // 敌人AI寻路
            int foodX = -1, foodY = -1, minDist = 9999;
            // 找最近的敌人食物
            for (int i = 0; i < MAX_FOOD; i++) {
                if (foods[i].active && !foods[i].forPlayer) {
                    int dist = abs(enemySnakes[k].x[0] - foods[i].x) + abs(enemySnakes[k].y[0] - foods[i].y);
                    if (dist < minDist) {
                        minDist = dist;
                        foodX = foods[i].x;
                        foodY = foods[i].y;
                    }
                }
            }

            // 攻击玩家判断
            int playerX = player.x[0], playerY = player.y[0];
            int playerDist = abs(enemySnakes[k].x[0] - playerX) + abs(enemySnakes[k].y[0] - playerY);
            int attackPlayer = 0;
            if (playerDist < 15 && !player.invincible) {
                int attackChance = 30 + difficulty * 10;
                if (rand() % 100 < attackChance) attackPlayer = 1;
            }

            // 确定移动目标
            int targetX = attackPlayer ? playerX : foodX;
            int targetY = attackPlayer ? playerY : foodY;

            // 计算移动方向
            if (targetX != -1 && targetY != -1) {
                int dx = targetX - enemySnakes[k].x[0];
                int dy = targetY - enemySnakes[k].y[0];

                // 随机转向（20%概率）
                if (rand() % 10 < 2) {
                    int newDir = rand() % 4;
                    if ((newDir == UP && enemySnakes[k].direction != DOWN) ||
                        (newDir == RIGHT && enemySnakes[k].direction != LEFT) ||
                        (newDir == DOWN && enemySnakes[k].direction != UP) ||
                        (newDir == LEFT && enemySnakes[k].direction != RIGHT)) {
                        enemySnakes[k].direction = newDir;
                    }
                }
                else {
                    // 优先水平或垂直移动
                    if (abs(dx) > abs(dy)) {
                        if (dx > 0 && enemySnakes[k].direction != LEFT) enemySnakes[k].direction = RIGHT;
                        else if (dx < 0 && enemySnakes[k].direction != RIGHT) enemySnakes[k].direction = LEFT;
                        else {
                            if (dy > 0 && enemySnakes[k].direction != UP) enemySnakes[k].direction = DOWN;
                            else if (dy < 0 && enemySnakes[k].direction != DOWN) enemySnakes[k].direction = UP;
                        }
                    }
                    else {
                        if (dy > 0 && enemySnakes[k].direction != UP) enemySnakes[k].direction = DOWN;
                        else if (dy < 0 && enemySnakes[k].direction != DOWN) enemySnakes[k].direction = UP;
                        else {
                            if (dx > 0 && enemySnakes[k].direction != LEFT) enemySnakes[k].direction = RIGHT;
                            else if (dx < 0 && enemySnakes[k].direction != RIGHT) enemySnakes[k].direction = LEFT;
                        }
                    }
                }
            }
            else {
                // 无目标时随机移动
                int newDir = rand() % 4;
                if ((newDir == UP && enemySnakes[k].direction != DOWN) ||
                    (newDir == RIGHT && enemySnakes[k].direction != LEFT) ||
                    (newDir == DOWN && enemySnakes[k].direction != UP) ||
                    (newDir == LEFT && enemySnakes[k].direction != RIGHT)) {
                    enemySnakes[k].direction = newDir;
                }
            }

            // 移动敌人蛇（尾部跟随）
            for (int i = enemySnakes[k].length - 1; i > 0; i--) {
                enemySnakes[k].x[i] = enemySnakes[k].x[i - 1];
                enemySnakes[k].y[i] = enemySnakes[k].y[i - 1];
            }

            // 移动蛇头
            switch (enemySnakes[k].direction) {
            case UP:    enemySnakes[k].y[0]--; break;
            case RIGHT: enemySnakes[k].x[0]++; break;
            case DOWN:  enemySnakes[k].y[0]++; break;
            case LEFT:  enemySnakes[k].x[0]--; break;
            }

            // 边界碰撞处理（防止穿墙）
            if (enemySnakes[k].x[0] <= 1) {
                enemySnakes[k].x[0] = 1;
                if (enemySnakes[k].direction == LEFT)
                    enemySnakes[k].direction = rand() % 2 ? UP : DOWN;
            }
            if (enemySnakes[k].x[0] >= W - 2) {
                enemySnakes[k].x[0] = W - 2;
                if (enemySnakes[k].direction == RIGHT)
                    enemySnakes[k].direction = rand() % 2 ? UP : DOWN;
            }
            if (enemySnakes[k].y[0] <= 1) {
                enemySnakes[k].y[0] = 1;
                if (enemySnakes[k].direction == UP)
                    enemySnakes[k].direction = rand() % 2 ? LEFT : RIGHT;
            }
            if (enemySnakes[k].y[0] >= H - 2) {
                enemySnakes[k].y[0] = H - 2;
                if (enemySnakes[k].direction == DOWN)
                    enemySnakes[k].direction = rand() % 2 ? LEFT : RIGHT;
            }

            // 敌人无敌状态结束
            if (enemySnakes[k].invincible && currentTime >= enemySnakes[k].invincibleEndTime) {
                enemySnakes[k].invincible = 0;
            }

            // 敌人碰撞检测
            int enemyCollision = 0;
            // 撞墙检测
            if (enemySnakes[k].x[0] <= 0 || enemySnakes[k].x[0] >= W - 1 ||
                enemySnakes[k].y[0] <= 0 || enemySnakes[k].y[0] >= H - 1) {
                enemyCollision = 1;
            }

            // 自撞检测
            if (!enemySnakes[k].invincible) {
                for (int i = 1; i < enemySnakes[k].length; i++) {
                    if (enemySnakes[k].x[0] == enemySnakes[k].x[i] &&
                        enemySnakes[k].y[0] == enemySnakes[k].y[i]) {
                        enemyCollision = 1;
                        break;
                    }
                }
            }

            // 敌人间碰撞
            for (int m = 0; m < enemyCount; m++) {
                if (m != k && !enemySnakes[m].respawning) {
                    for (int i = 1; i < enemySnakes[m].length; i++) {
                        if (enemySnakes[k].x[0] == enemySnakes[m].x[i] &&
                            enemySnakes[k].y[0] == enemySnakes[m].y[i]) {
                            enemyCollision = 1;
                            break;
                        }
                    }
                }
            }

            // 敌人撞玩家
            if (!player.respawning && !player.invincible) {
                for (int i = 1; i < player.length; i++) {
                    if (enemySnakes[k].x[0] == player.x[i] &&
                        enemySnakes[k].y[0] == player.y[i]) {
                        player.lives--;
                        if (player.lives <= 0) {
                            gameOver = 1;
                        }
                        else {
                            player.invincible = 1;
                            player.invincibleEndTime = currentTime + INVINCIBLE_TIME;
                            player.respawning = 1;
                            player.respawnEndTime = currentTime + RESPAWN_TIME;
                        }
                        gotoxy(W - 40, H);
                        setColor(7);
                        printf("生命: %d", player.lives);
                        enemyCollision = 1;
                        break;
                    }
                }
            }

            // 处理敌人碰撞
            if (enemyCollision) {
                enemySnakes[k].lives--;
                if (enemySnakes[k].lives <= 0) {
                    player.score += enemySnakes[k].score / 2;  // 玩家获得一半分数
                    gotoxy(W - 20, H);
                    setColor(7);
                    printf("分数: %d", player.score);

                    enemySnakes[k].lives = 1;
                    enemySnakes[k].respawning = 1;
                    enemySnakes[k].respawnEndTime = currentTime + RESPAWN_TIME;
                }
            }

            if (enemySnakes[k].respawning) continue;

            // 敌人吃食物
            int enemyAte = 0;
            for (int i = 0; i < MAX_FOOD; i++) {
                if (foods[i].active && enemySnakes[k].x[0] == foods[i].x &&
                    enemySnakes[k].y[0] == foods[i].y) {
                    if (!foods[i].forPlayer) {
                        // 正确食物
                        switch (foods[i].type) {
                        case FOOD_WHITE:
                            enemySnakes[k].length++;
                            enemySnakes[k].score += 10;
                            break;
                        case FOOD_RED:
                            enemySnakes[k].score -= 10;
                            if (enemySnakes[k].score < 0) enemySnakes[k].score = 0;
                            break;
                        case FOOD_GREEN:
                            enemySnakes[k].length++;
                            enemySnakes[k].score += 20;
                            break;
                        }
                    }
                    else {
                        // 错误食物（扣10分）
                        enemySnakes[k].score -= 10;
                        if (enemySnakes[k].score < 0) enemySnakes[k].score = 0;
                    }
                    foods[i].active = 0;
                    currentFoodCount--;
                    enemyAte = 1;
                    break;
                }
            }

            // 敌人增长尾部
            if (enemyAte) {
                int tailX = enemySnakes[k].x[enemySnakes[k].length - 1];
                int tailY = enemySnakes[k].y[enemySnakes[k].length - 1];
                int prevX = enemySnakes[k].x[enemySnakes[k].length - 2];
                int prevY = enemySnakes[k].y[enemySnakes[k].length - 2];

                if (tailX > prevX) enemySnakes[k].x[enemySnakes[k].length] = tailX + 1;
                else if (tailX < prevX) enemySnakes[k].x[enemySnakes[k].length] = tailX - 1;
                else enemySnakes[k].x[enemySnakes[k].length] = tailX;

                if (tailY > prevY) enemySnakes[k].y[enemySnakes[k].length] = tailY + 1;
                else if (tailY < prevY) enemySnakes[k].y[enemySnakes[k].length] = tailY - 1;
                else enemySnakes[k].y[enemySnakes[k].length] = tailY;

                enemySnakes[k].length++;
            }

            // 绘制敌人蛇
            if (enemySnakes[k].invincible) {
                setColor((currentTime / 100) % 2 == 0 ? 14 : enemySnakes[k].color);
            }
            else {
                setColor(enemySnakes[k].color);
            }
            for (int i = 0; i < enemySnakes[k].length; i++) {
                gotoxy(enemySnakes[k].x[i], enemySnakes[k].y[i]);
                printf("%c", i == 0 ? enemySnakes[k].headChar : enemySnakes[k].bodyChar);
            }
        }

        // 显示敌人分数
        if (enemyCount > 0) {
            int displayX = 0;
            for (int k = 0; k < enemyCount; k++) {
                gotoxy(displayX, H + 2);
                setColor(enemySnakes[k].color);
                printf("敌人%d分数: %d    ", k + 1, enemySnakes[k].score);
                displayX += 20;
            }
        }

        // 处理食物生命周期
        for (int i = 0; i < MAX_FOOD; i++) {
            if (foods[i].active) {
                foods[i].lifetime--;
                if (foods[i].lifetime <= 0) {
                    gotoxy(foods[i].x, foods[i].y);
                    printf(" ");
                    foods[i].active = 0;
                    currentFoodCount--;
                }
            }
        }

        // 5级限时模式
        if (difficulty == 5) {
            int remainingTime = (level5Timer - (currentTime - level5StartTime)) / 1000;
            if (remainingTime <= 0) {
                gameOver = 1;
                int playerWon = 0;
                for (int k = 0; k < enemyCount; k++) {
                    if (player.score > enemySnakes[k].score) {
                        playerWon = 1;
                        break;
                    }
                }

                system("cls");
                if (playerWon) {
                    setColor(10);
                    printf("恭喜获胜！你的分数超过了至少一个敌人！\n");
                }
                else {
                    setColor(12);
                    printf("游戏结束！你的分数未超过任何敌人！\n");
                }
                setColor(7);
                printf("你的分数: %d\n", player.score);
                for (int k = 0; k < enemyCount; k++) {
                    printf("敌人%d分数: %d\n", k + 1, enemySnakes[k].score);
                }
                printf("按任意键退出...");
                _getch();
                break;
            }
            else {
                gotoxy(W - 30, H + 1);
                setColor(14);
                printf("剩余时间: %02d:%02d", remainingTime / 60, remainingTime % 60);
            }
        }
    }

    // 游戏结束处理
    if (gameOver && difficulty != 5) {
        system("cls");
        setColor(12);
        printf("游戏结束！\n");
        setColor(7);
        printf("你的最终分数: %d\n", player.score);
        if (enemyCount > 0) {
            for (int k = 0; k < enemyCount; k++) {
                printf("敌人%d分数: %d\n", k + 1, enemySnakes[k].score);
            }
        }
        printf("按任意键退出...");
        _getch();
    }

    return 0;
}