#include<stdio.h>
#include <graphics.h>
#include <conio.h>

#include<vector>
//实现三重背景图片透明
#include"tools.h"


using namespace std;  //声明命名空间

// 检查是否胜利
#define WIN_SCORE  100

//窗口宏大小
#define WIN_WIDTH 1012
#define WIN_HEIGHT 396
#define OBSTACLE_COUNT 10
//全局变量 
IMAGE imgBgs[3];  //背景图片
int bgX[3];  //背景图片的x坐标
int bgSpeed[3] = { 1,2,4 };

IMAGE imgHeros[12];
int heroX; //玩家的x坐标
int heroY;//玩家的y坐标
int heroIndex; //玩家奔跑图片的序号

bool heroJump;  //表示玩家正在跳跃


int jumpHeightMax; //最大跳跃高度
int heroJumpOff;  //跳跃速度
int update; //是否需要马上刷新画面

//IMAGE imgTortoise; //小乌龟
//int torToiseX; //小乌龟的水平坐标
//int torToiseY; //小乌龟的垂直坐标
//bool torToiseExist; //当前窗口是否有小乌龟

// 以下进一步封装
//typedef enum {
//	TORTOISE, //乌龟
//	LION,  //狮子
//	OBSTACLE_TYPE_COUNT
//} obstacle_type;
int heroBlood;
int score;  //分数

typedef enum {
	TORTOISE,
	LION,
	HOOK1,
	HOOK2,
	HOOK3,
	HOOK4,
	OBSTACLE_TYPE_COUNT
}obstacle_type;
//IMAGE obstacleImgs[3][12];
//可变数组
vector<vector<IMAGE>> obstacleImgs; //存放所有障碍物的各个图片

//typedef struct obstacle{
//	int  type;  //障碍物类型
//	int imgIndex;  //当前显示的图片的序号
//	int x, y;  //障碍物的坐标
//	int speed;
//	int power; //杀伤力
//	bool exist;
//}obstacle_t;
typedef struct obstacle {
	int type;//障碍物的类型
	int imgIndex;//当前显示的图片的序号
	int x, y;//障碍物的坐标
	int speed;//速度
	int power;//杀伤力
	bool exist;
	bool hited;//表示是否已经发生碰撞
	bool passed;//表示是否已经被通过
}obstacle_t;
obstacle_t obstacles[OBSTACLE_COUNT];  //障碍物数量
int  lastObsIndex;

IMAGE imgHeroDown[2];
bool heroDown; //表示玩家是否处于下蹲状态
IMAGE imgSZ[10];

//游戏初始化
void init() {
	//创建游戏窗口
	initgraph(WIN_WIDTH, WIN_HEIGHT);

	//加载游戏的背景资源
	char name[64];
	for (int i = 0; i < 3; i++) {
		sprintf(name, "res/bg%03d.png",i+1);
		loadimage(&imgBgs[i],name);

		bgX[i] = 0;
		heroJump = false;
	}
	// 加载Hero奔跑的图片帧素材
	for (int i = 0; i < 12; i++)
	{
		//res/hero1.png   ....res/hero12.png
		sprintf(name, "res/hero%d.png", i + 1);
		loadimage(&imgHeros[i], name);
	}
	heroX = WIN_WIDTH * 0.5 - imgHeros[0].getwidth() * 0.5;
	heroY = 345 - imgHeros[0].getheight();
	heroIndex = 0;

	heroJump = false;
	jumpHeightMax = 345 - imgHeros[0].getheight() - 120;
	heroJumpOff = -4;

	update = true;

	//加载小乌龟素材
	/*loadimage(&imgTortoise, "res/t1.png");
	torToiseExist = false;
	torToiseY = 345 - imgTortoise.getheight()+5;*/
	IMAGE imgTort;
	loadimage(&imgTort, "res/t1.png");
	vector<IMAGE> imgTortArray;
	imgTortArray.push_back(imgTort);
	obstacleImgs.push_back(imgTortArray);

	IMAGE imgLion;
	vector<IMAGE> imgLionArray;
	for (int i = 0; i < 6; i++)
	{
		sprintf(name, "res/p%d.png", i + 1);
		loadimage(&imgLion, name);
		imgLionArray.push_back(imgLion);
	}
	obstacleImgs.push_back(imgLionArray);
	//初始化障碍物池
	for (int i = 0; i < OBSTACLE_COUNT; i++)
	{
		obstacles[i].exist = false;
	}

	// 加载下蹲素材
	loadimage(&imgHeroDown[0], "res/d1.png");
	loadimage(&imgHeroDown[1], "res/d2.png");
	heroDown = false;

	IMAGE imgH;
	
	for (int i = 0; i < 4; i++)
	{
		vector<IMAGE> imgHookArray;
		sprintf(name,  "res/h%d.png", i + 1);
		loadimage(&imgH, name, 60, 260,true);
		imgHookArray.push_back(imgH);
		obstacleImgs.push_back(imgHookArray);
	}

	heroBlood = 100;
	// 预加载音效
	preLoadSound("res/hit.mp3");

	mciSendString("play res/bg.mp3 repeat", 0, 0, 0);//背景音乐

	lastObsIndex = -1;
	score = 0;

	// 加载数字图片
	for (int i = 0; i < 10; i++)
	{
		sprintf(name , "res/sz/%d.png", i);
		loadimage(&imgSZ[i], name);
	}
}

void createObstacle() {
	int i;
	for (i = 0; i < OBSTACLE_COUNT; i++) {
		if (obstacles[i].exist == false) {
			break;
		}
	}
	if (i >= OBSTACLE_COUNT) {
		return;
	}
	obstacles[i].exist = true;
	obstacles[i].hited = false;
	obstacles[i].imgIndex = 0;
	//obstacles[i].type =(obstacle_type) (rand() % OBSTACLE_TYPE_COUNT);

	obstacles[i].type = (obstacle_type)(rand() % 3);
	// 解决 上面和下面障碍物同时出现的问题
	if (lastObsIndex >= 0 &&
		obstacles[lastObsIndex].type >= HOOK1 &&
		obstacles[lastObsIndex].type <= HOOK4 &&
		obstacles[i].type == LION &&
		obstacles[lastObsIndex].x > (WIN_WIDTH - 500)) {
		obstacles[i].type = TORTOISE;
	}
	lastObsIndex = i;
	if (obstacles[i].type == HOOK1) {
		obstacles[i].type +=  rand() % 4; //0-3
	}

	/*obstacles[i].exist = true;
	obstacles[i].imgIndex = 0;*/
	obstacles[i].type = (obstacle_type)(rand() % OBSTACLE_TYPE_COUNT);
	obstacles[i].x = WIN_WIDTH;
	obstacles[i].y = 345 + 5 - obstacleImgs[obstacles[i].type][0].getheight();
	if (obstacles[i].type == TORTOISE) {
		obstacles[i].speed = 0;
		obstacles[i].power = 5;
	}
	else if (obstacles[i].type == LION) {
		obstacles[i].speed = 4;
		obstacles[i].power = 20;
	}
	else if (obstacles[i].type >= HOOK1 && obstacles[i].type <= HOOK4) {
		obstacles[i].speed = 0;
		obstacles[i].power = 20;
		obstacles[i].y = 0;
	}


	obstacles[i].passed = false;
}
// 碰撞检测
void checkHit() {
	for (int i = 0; i < OBSTACLE_COUNT; i++)
	{
		if (obstacles[i].exist && obstacles[i].hited == false) {
			int a1x, a1y, a2x, a2y;
			int off = 30; //核心碰撞区域
			if (!heroDown) {
				a1x = heroX + off;
				a1y = heroY + off;
				a2x = heroX + imgHeros[heroIndex].getwidth() - off;
				a2y = heroY + imgHeros[heroIndex].getheight();
			}
			else {
				a1x = heroX + off;
				a1y = 345 - imgHeros[heroIndex].getheight();
				a2x = heroX + imgHeroDown[heroIndex].getwidth() - off;
				a2y = 345;
			}
			IMAGE img = obstacleImgs[obstacles[i].type][obstacles[i].imgIndex];
			int b1x = obstacles[i].x + off;
			int b1y = obstacles[i].y + off;
			int b2x = obstacles[i].x + img.getwidth() - off;
			int b2y = obstacles[i].y + img.getheight() - 10;

			if(rectIntersect(a1x,a1y,a2x,a2y,b1x,b1y,b2x,b2y)){
				heroBlood -= obstacles[i].power;
				printf("血量剩余 %d\n", heroBlood);
				playSound("res/hit.mp3");  // 播放音效
				obstacles[i].hited = true;
			}
		}
	}
}

void fly(){
	for (int i = 0; i < 3; i++)
	{
		bgX[i] -= bgSpeed[i];
		if (bgX[i] < -WIN_WIDTH) {
			bgX[i] = 0;
		}
	}
	/*heroIndex = (heroIndex + 1) % 12;*/

	//实现跳跃
	if (heroJump) {
		if (heroY < jumpHeightMax) {
			heroJumpOff = 4;
		}
		heroY += heroJumpOff;
		if (heroY > 345 - imgHeros[0].getheight()) {
			heroJump = false;
			heroJumpOff = -4;
		}
	}
	else if (heroDown) {
		//下蹲
		/*heroIndex = (heroIndex + 1) % 12;*/
		static int count = 0;
		int delays[2] = { 8, 30 };
		count++;
		if (count >= delays[heroIndex]) {
			count = 0;
			heroIndex++;
			if (heroIndex >= 2) {
				heroIndex = 0;
				heroDown = false;
			}
		}
	}else {
		//不跳跃
		heroIndex = (heroIndex + 1) % 12;
	}

	//创建障碍物
	static int frameCount = 0;
	static int enemyFre = 50;
	frameCount++;
	if (frameCount > enemyFre) {
		frameCount = 0;
		enemyFre = 50 + rand() % 50;//50-99随机数
		createObstacle();
	}
	for (int i = 0; i < OBSTACLE_COUNT; i++) {
		if (obstacles[i].exist) {
			obstacles[i].x -= obstacles[i].speed + bgSpeed[2];
			if (obstacles[i].x < -obstacleImgs[obstacles[i].type][0].getwidth() * 2) {//obstacleImgs[obstacles[i].type][0].getwidth() * 2
				obstacles[i].exist = false;
			}
			int len = obstacleImgs[obstacles[i].type].size();
			obstacles[i].imgIndex = (obstacles[i].imgIndex + 1) % len;
		}
	}

	//创建小乌龟
	//if (frameCount > torToiseFre) {
	//	frameCount = 0;
	//	if (!torToiseExist) {
	//		torToiseExist = true;
	//		torToiseX = WIN_WIDTH;

	//		torToiseFre = 200 + rand() % 300;
	//	}
	//}

	//if (torToiseExist) {
	//	torToiseX -= bgSpeed[2];
	//	if (torToiseX < -imgTortoise.getwidth()) {
	//		torToiseExist = false;
	//	}
	//}
	checkHit();
}

//渲染游戏背景
void updateBg() {
	
	putimagePNG2(bgX[0], 0, &imgBgs[0]);
	putimagePNG2(bgX[1], 119, &imgBgs[1]);
	putimagePNG2(bgX[2], 330, &imgBgs[2]);
}


void jump() {
	heroDown = true;
	update = true;
}

void down() {
	heroJump = true;
	update = true;
	heroIndex = 0;
}

//处理用户输入按键
void keyEvent() {
	char ch;
	if (_kbhit()) {   //如何有按键输入 _kbhit() 为真
		ch = _getch(); //不需要按下回车即可直接读取
		if (ch == 's') {
			jump();
		}
		else if (ch == 'w') {
			down();
		}
	}
	
}

void updateEnemy() {
	//if (torToiseExist) {
	//	putimagePNG2(torToiseX, torToiseY,WIN_WIDTH,&imgTortoise);
	//}
	for (int i = 0; i < OBSTACLE_COUNT; i++)
	{
		if (obstacles[i].exist) {
			putimagePNG2(obstacles[i].x, obstacles[i].y,WIN_WIDTH,
				&obstacleImgs[obstacles[i].type][obstacles[i].imgIndex]);
		}
	}
}
// 更新角色状态
void updateHero() {
	if (!heroDown) {
		putimagePNG2(heroX, heroY, &imgHeros[heroIndex]);
	}
	else {
		int y = 345 - imgHeroDown[heroIndex].getheight();
		putimagePNG2(heroX, y, &imgHeroDown[heroIndex]);
	}
}

// 血条   直接使用接口
void updateBloodBar() {
	drawBloodBar(10, 10, 200, 10, 2, BLUE, DARKGRAY, RED, heroBlood/100.0);
}

//判断游戏是否结束
void checkOver() {
	if (heroBlood <= 0) {
		loadimage(0, "res/over.png");
		FlushBatchDraw();
		mciSendString("stop res/bg.mp3", 0, 0, 0);
		system("pause");
		// 结束后  选择
		heroBlood = 100;
		score = 0;
		mciSendString("play res/bg.mp3 repeat", 0, 0, 0);
	}
}

//检查分数   过障碍后加分
void checkScore() {
	for (int i = 0; i < OBSTACLE_COUNT; i++)
	{
		if(obstacles[i].exist && 
			obstacles[i].passed == false &&
			obstacles[i].hited == false &&
			obstacles[i].x + obstacleImgs[obstacles[i].type][0].getwidth() < heroX){
			score++;
			obstacles[i].passed = true;
			printf("score:%d\n", score);
		}
	}
}

// 更新分数
void updateScore() {
	// 50 => "50"
	char str[8];
	sprintf(str, "%d", score);

	int x = 20;
	int y = 25;

	for (int i = 0; str[i]; i++)
	{
		int sz = str[i] - '0';
		putimagePNG2(x, y, &imgSZ[sz]);
		x += imgSZ[sz].getwidth() + 5;
	}
}
//是否胜利
void checkWin() {
	if (score >= WIN_SCORE) {
		FlushBatchDraw();
		mciSendString("play res/win.mp3", 0, 0, 0);
		Sleep(2000);
		loadimage(0, "res/win.png");
		FlushBatchDraw();
		mciSendString("stop res/bg.mp3", 0, 0, 0);
		system("pause");

		heroBlood = 100;
		score = 0;
		mciSendString("play res/bg.mp3 repeat", 0, 0, 0);
	}
}

int main(void) {
	init();

	loadimage(0, "res/over.png");
	system("pause");

	int timer = 0;//优化帧  不让画面快速刷新
	while (1) {
		keyEvent();
		timer += getDelay();
		if (timer > 30) {
			timer = 0;
			update = true;
		}
		 
		if (update) {
			update = false;
			BeginBatchDraw();
			updateBg();
			updateHero();
			updateEnemy();
			updateBloodBar();
			updateScore();
			checkWin();
			EndBatchDraw();

			checkOver();
			checkScore();

			fly();
		}
		/*Sleep(30);*/
	}
	
	system("pause");
	return 0;
}