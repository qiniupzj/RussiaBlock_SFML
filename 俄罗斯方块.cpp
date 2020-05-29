#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <time.h>
using namespace sf;

const int ROW_COUNT = 20;
const int COL_COUNT = 10;

const float SPEED_NORMAL = 0.3;
const float SPEED_QUICK = 0.05;

int table[ROW_COUNT][COL_COUNT] = { 0 };

struct Point {
    int x, y;
} curBlock[4], BakBlock[4];

int blocks[7][4] = {
    1,3,5,7, // I
    2,4,5,7, // Z 1型
    3,5,4,6, // Z 2型
    3,5,4,7, // T
    2,3,5,7, // L
    3,5,7,6, // J
    2,3,4,5, // 田
};

int blockIndex;
float delay = SPEED_NORMAL;

sf::Sound sou;

// 显示分数
Font font;
Text textScore;
int score = 0;


bool check()
{
    for (int i = 0; i < 4; i++)
        if (curBlock[i].x < 0 || curBlock[i].x >= COL_COUNT || curBlock[i].y >= ROW_COUNT || curBlock[i].y <= 0
            || table[curBlock[i].y][curBlock[i].x]) {
            return 0;
        }
    return 1;
};

void doRotate(void) {
    if (blockIndex == 7) {  // 田字形，不需要旋转
        return;
    }

    for (int i = 0; i < 4; i++) {
        BakBlock[i] = curBlock[i];  //备份当前方块
    }

    Point p = curBlock[1]; //center of rotation
    for (int i = 0; i < 4; i++)
    {
        struct Point tmp = curBlock[i];
        curBlock[i].x = p.x - tmp.y + p.y;
        curBlock[i].y = p.y + tmp.x - p.x;
    }
    if (!check()) {
        for (int i = 0; i < 4; i++) {
            curBlock[i] = BakBlock[i];
        }
    }
}

void moveLeftRight(int offeset) {
    for (int i = 0; i < 4; i++) {
        BakBlock[i] = curBlock[i];  //备份当前方块
        curBlock[i].x += offeset;   //更新当前方块
    }

    if (!check()) {
        // 把当前方块还原到备份方块
        for (int i = 0; i < 4; i++) {
            curBlock[i] = BakBlock[i];
        }
    }
}

void keyEvent(RenderWindow * window) {
    Event e;
    int dx = 0;
    bool rotate = false;

    // pollEvent 从事件队列中取出一个事件
    while (window->pollEvent(e))
    {
        if (e.type == Event::Closed)
            window->close();

        if (e.type == Event::KeyPressed) {
            switch (e.key.code) {
            case Keyboard::Up:
                rotate = true;
                break;
            case Keyboard::Left:
                dx = -1;
                break;
            case Keyboard::Right:
                dx = 1;
                break;
            default:
                break;
            }
        }     
    }
    // 按键事件处理，下降
    if (Keyboard::isKeyPressed(Keyboard::Down)) {
        delay = SPEED_QUICK;
    }

    if (dx != 0) {
        moveLeftRight(dx);
    }
    
    //////Rotate//////
    if (rotate) {
        doRotate();
    }
}

void newBlock() {
    blockIndex = 1 + rand() % 7; // 1..7
    int n = blockIndex - 1;//rand() % 7;
    for (int i = 0; i < 4; i++)
    {
        curBlock[i].x = blocks[n][i] % 2;
        curBlock[i].y = blocks[n][i] / 2;
    }
}

void clearLine() {
    /////check lines//////////
    int k = ROW_COUNT - 1;
    for (int i = ROW_COUNT - 1; i > 0; i--)
    {
        int count = 0;
        for (int j = 0; j < COL_COUNT; j++)
        {
            if (table[i][j]) count++;

            table[k][j] = table[i][j];
        }

        // 如果没有满行,就继续扫描上一行
        // 如果已经满行,k不变,在这个满行内,存放下一次的扫描结果
        if (count < COL_COUNT) k--;
        else {
            score += 10;
            sou.play();
        }
    }

    char tmp[16];
    sprintf_s(tmp, "%d", score);
    textScore.setString(tmp);
}

void drawBlocks(RenderWindow *window, Sprite *spriteBlock) {
    // 绘制已降落完毕的方块
    for (int i = 0; i < ROW_COUNT; i++)
        for (int j = 0; j < COL_COUNT; j++)
        {
            if (table[i][j] == 0) continue;
            spriteBlock->setTextureRect(IntRect(table[i][j] * 18, 0, 18, 18));
            spriteBlock->setPosition(j * 18, i * 18);
            spriteBlock->move(28, 31); //offset
            window->draw(*spriteBlock);
        }

    // 绘制当前方块
    for (int i = 0; i < 4; i++)
    {
        spriteBlock->setTextureRect(IntRect(blockIndex * 18, 0, 18, 18));
        spriteBlock->setPosition(curBlock[i].x * 18, curBlock[i].y * 18);
        spriteBlock->move(28, 31); //offset
        window->draw(*spriteBlock);
    }
}

void drop() {
    for (int i = 0; i < 4; i++) { 
        BakBlock[i] = curBlock[i]; 
        curBlock[i].y += 1; 
    }

    if (!check()) {
        for (int i = 0; i < 4; i++) {
            // 设置标记，“固化”对应位置
            table[BakBlock[i].y][BakBlock[i].x] = blockIndex;
        }
        newBlock();
    } 
}

void initScore() {
    if (!font.loadFromFile("Sansation.ttf")) {
        exit(1);
    }

    textScore.setFont(font); // font is a sf::Font
    textScore.setCharacterSize(30);// set the character size
    textScore.setFillColor(sf::Color::White); // set the color
    textScore.setStyle(sf::Text::Bold); // set the text style
    textScore.setPosition(255, 175);
    textScore.setString("0");
}

int main()
{
    // 播放背景音乐
    sf::Music music;
    if (!music.openFromFile("bg2.wav"))
        return -1; // error
    music.setLoop(true);
    music.play();

    // 加载音效
    sf::SoundBuffer xiaochu;
    if (!xiaochu.loadFromFile("xiaochu.wav")) {
        return -1;
    }
    
    // 设置声音缓存
    sou.setBuffer(xiaochu);
    //sou.play(); // 播放音频

    srand(time(0));

    // 创建窗口
    RenderWindow window(
        VideoMode(320, 416),   //窗口模式
        "Rock");   //标题

    // 创建表示图片的精灵
    Texture t1, t2, t3;
    t1.loadFromFile("images/tiles.png");
    t2.loadFromFile("images/bg2.jpg");
    t3.loadFromFile("images/frame.png");
    Sprite spriteBlock(t1);
    Sprite spriteBg(t2);
    Sprite spriteFrame(t3);
    
    initScore();

    // 用来计算时间
    float timer = 0; //delay = 0.3;

    // 计时器
    Clock clock;

    newBlock(); // 生成第一个方块

    while (window.isOpen())
    {
        float time = clock.getElapsedTime().asSeconds();
        clock.restart(); //计时器重启及时
        timer += time;

        keyEvent(&window);

        if (timer > delay) {
            drop(); //下降一个位置
            timer = 0;
        }

        clearLine(); // 清除满行

        delay = SPEED_NORMAL;

        /////////draw//////////
        window.clear(Color::White);
        window.draw(spriteBg);
        drawBlocks(&window, &spriteBlock);
        window.draw(spriteFrame);
        window.draw(textScore); //显示分数
       
        window.display();
    }

    return 0;
}
