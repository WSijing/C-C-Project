/*������־
* 1.ʵ�ֳ�ʼ����
* 2.ʵ��ֲ�￨��
* 3.ʵ��ֲ���ѡȡ���϶�
* 4.ʵ��ֲ�����ֲ
* 5.ʵ��ֲ���ҡ��
* 6.ʵ�ֲ˵�����
* 7.ʵ������������
* 8.ʵ���ռ��������ʾ����ֵ
* 9.������ʬ��ʵ��������
* 10.ʵ��������ɵ�����������Ĺ���
* 11.ʵ���㶹�ӵ��ķ������Ⱦ
* 12.ʵ���㶹�ӵ��뽩ʬ����ײ���
* 13.ʵ�ֽ�ʬ����
* 14.ʵ�ֽ�ʬ��ֲ��
* 15.ʵ�����տ���������
* 16.ƬͷѲ��
* 17.�ж���Ϸ����
*/

#include <stdio.h>
#include "tools.h"
#include <graphics.h>
#include <math.h>
#include <time.h>
#include <math.h>
#include <mmsystem.h>
#include "vector2.h"

#pragma comment(lib, "winmm.lib")

#define WIN_WIDTH 900
#define WIN_HEIGHT 600
#define ZOMBIE_MAX 10

enum {WAN_DOU, XIANG_RI_KUI, ZHI_WU_COUNT};
enum {SUNSHINE_DOWN, SUNSHINE_GROUND, SUNSHINE_COLLECT, SUNSHINE_PRODUCT};
enum {GOING, WIN, FAIL};

struct averageZhiwu {
	int type; //ֲ������ 0���� 1����һ��ֲ��
	int frameIndex; //֡���
	int deadTime; //������ʱ��

	int timer;
	int x, y;

	int shootTime;
} map[5][9];

struct visualZhiwu {
	int type; //ֲ������ 0���� 1����һ��ֲ��
	int x, y; //���ָ���λ��
} unmap[5][9], checkUnmap;

struct sunshineBall {
	int x, y; //Ʈ������е�λ��
	int frameIndex; //֡���
	int destY; //���λ��
	bool used; //�Ƿ�ʹ�ù���1��δʹ�ù� 0��ʹ�ù�
	int timer;

	float t; //������ʱ��� 0..1
	vector2 p1, p2, p3, p4;
	vector2 currentPresentation;
	float speed;
	int status;
} balls[20];

struct zombie {
	int x, y;
	int frameIndex;
	bool used;
	int speed;
	int row;
	int blood; //��ʬѪ��
	int dead; //������
	bool eating;
} zombies[10];

struct bullet {
	int x, y;
	bool used;
	int speed;
	int row;
	bool blast;
	int frameIndex;
} bullets[30];

IMAGE imgBackground; //����ͼƬ
IMAGE imgBar5; //ֲ�￨��
IMAGE imgCards[ZHI_WU_COUNT]; //ֲ�￨��
IMAGE* imgZhiWu[ZHI_WU_COUNT][20]; //ֲ��֡ͼ
IMAGE imgSunshineBall[29];
IMAGE imgZombie[22];
IMAGE imgBulletNormal;
IMAGE imgBulletBlast[4];
IMAGE imgZombieEating[21];
IMAGE imgZombieStand[11];
IMAGE imgZombieDead[10];

int curX, curY; //��ǰѡ��ֲ�����ƶ������е�λ��
int curZhiWu; //0��δѡ�� 1��ѡ���һ��ֲ��
bool firstDown = 0; // �ж�ץȡ���Ƿ���
int sunshine; //����ֵ
int killCount; //�ѻ�ɱ��ʬ��
int zombieCount; //�ѳ��ֽ�ʬ��
int gameStatus; //��Ϸ״̬

bool fileExist(const char* name) {
	FILE* fp = fopen(name, "r");
	if (fp) {
		fclose(fp);
		return 1;
	}
	else {
		return 0;
	}
}

void gameInit() {
	//���ر���ͼƬ
	loadimage(&imgBackground, "res/bg.jpg");
	loadimage(&imgBar5, "res/bar5.png");

	//��ʼ��ֲ�����״̬
	memset(imgZhiWu, 0, sizeof(imgZhiWu));
	memset(map, 0, sizeof(map));
	memset(unmap, 0, sizeof(unmap));

	killCount = 0;
	zombieCount = 0;
	gameStatus = GOING;

	//����ֲ�￨��
	char name[64];

	for (int i = 0; i < ZHI_WU_COUNT; i++) {
		//����ֲ�￨�Ƶ��ļ���
		sprintf_s(name, sizeof(name), "res/Cards/card_%d.png", i + 1);
		loadimage(&imgCards[i], name);

		//����ֲ��ҡ��֡ͼ���ļ���
		for (int j = 0; j < 20; j++) {
			sprintf_s(name, sizeof(name), "res/zhiwu/%d/%d.png", i, j);

			//�ж��ļ��Ĵ�����
			if (fileExist(name)) {
				imgZhiWu[i][j] = new IMAGE;
				loadimage(imgZhiWu[i][j], name);
			}
			else {
				break;
			}
		}
	}

	curZhiWu = 0; //��ʼ��ѡ��ֲ��״̬
	sunshine = 50; //��ʼ������ֵ

	
	memset(balls, 0, sizeof(balls));
	for (int i = 0; i < 29; i++) {
		sprintf_s(name, sizeof(name), "res/sunshine/%d.png", i + 1);
		loadimage(imgSunshineBall + i, name);
	}


	//�������
	srand(time(NULL));

	//������Ϸͼ�δ���
	initgraph(WIN_WIDTH, WIN_HEIGHT, 1);

	//��������
	LOGFONT font;
	getfont(&font);
	font.lfHeight = 30;
	font.lfWeight = 15;
	strcpy(font.lfFaceName, "Segoe UI Black");
	font.lfQuality = ANTIALIASED_QUALITY; //���������
	settextstyle(&font);
	setbkmode(TRANSPARENT);
	setcolor(BLACK);

	//��ʼ����ʬ����
	memset(zombies, 0, sizeof(zombies));
	for (int i = 0; i < 22; i++) {
		sprintf_s(name, sizeof(name), "res/zm/%d.png", i + 1);
		loadimage(&imgZombie[i], name);
	}

	//��ʼ���㶹�ӵ���֡ͼƬ����
	loadimage(&imgBulletNormal, "res/bullets/bullet_normal.png");
	memset(bullets, 0, sizeof(bullets));

	//��ʼ���㶹�ӵ�������֡ͼƬ����
	loadimage(&imgBulletBlast[3], "res/bullets/bullet_blast.png");
	for (int i = 0; i < 3; i++) {
		float k = (i + 1) * 0.2;
		loadimage(&imgBulletBlast[i], "res/bullets/bullet_blast.png", imgBulletBlast[3].getwidth() * k, imgBulletBlast[3].getheight() * k,1);
	}

	//��ʼ����ʬ��ֲ���֡ͼƬ����
	for (int i = 0; i < 21; i++) {
		sprintf_s(name, sizeof(name), "res/zm_eat/%d.png", i + 1);
		loadimage(&imgZombieEating[i], name);
	}

	//��ʼ����ʬ������֡ͼƬ����
	for (int i = 0; i < 11; i++) {
		sprintf_s(name, sizeof(name), "res/zm_dead/%d.png", i + 1);
		loadimage(&imgZombieDead[i], name);
	}

	//��ʼ��������ʬ��֡ͼƬ����
	for (int i = 0; i < 11; i++) {
		sprintf_s(name, sizeof(name), "res/zm_stand/%d.png", i + 1);
		loadimage(&imgZombieStand[i], name);
	}
}

void drawCards() {
	//ֲ�￨����俨��
	for (int i = 0; i < ZHI_WU_COUNT; i++) {
		int x = 268 - 112 + i * 65;
		int y = 6;
		putimage(x, y, &imgCards[i]);
	}
}

void drawPlant() {
	//һ֡һ֡�����ʵ��ֲ��ҡ��
	for (int i = 0; i < 5; i++) {
		for (int j = 0; j < 9; j++) {
			if (map[i][j].type > 0) {
				int zhiWuType = map[i][j].type - 1;
				int index = map[i][j].frameIndex;
				if (!index) {
					index++;
				}
				putimagePNG(map[i][j].x, map[i][j].y + 15, imgZhiWu[zhiWuType][index]);
			}
		}
	}

	//��Ⱦ�϶����̵�ֲ��
	if (curZhiWu) {
		bool changeFlag = 1;
		for (int i = 0; i < 5; i++) {
			for (int j = 0; j < 9; j++) {
				if (unmap[i][j].type > 0 && map[i][j].type == 0) {
					unmap[i][j].x = 259 - 112 + j * 81;
					unmap[i][j].y = 77 + 15 + i * 102;
					IMAGE* unimg = imgZhiWu[unmap[i][j].type - 1][0];
					putimagePNG(unmap[i][j].x, unmap[i][j].y, unimg);
					if (unmap[i][j].x == checkUnmap.x && unmap[i][j].y == checkUnmap.y) {
						changeFlag = 0;
					}
					checkUnmap = unmap[i][j];
				}
			}
		}
		if (changeFlag) {
			memset(unmap, 0, sizeof(unmap));
		}
		IMAGE* img = imgZhiWu[curZhiWu - 1][1];
		putimagePNG(curX - img->getwidth() / 2, curY - img->getheight() / 2, img);
	}
	else {
		memset(unmap, 0, sizeof(unmap));
	}
}

void drawSunshine() {
	int ballMax = sizeof(balls) / sizeof(balls[0]);
	for (int i = 0; i < ballMax; i++) {
		if(balls[i].used || balls[i].status == SUNSHINE_COLLECT) {
			IMAGE* img = &imgSunshineBall[balls[i].frameIndex];
			putimagePNG(balls[i].currentPresentation.x, balls[i].currentPresentation.y, img);
		}
	}

	//��ʾ����ֵ
	char scoreText[8];
	sprintf_s(scoreText, sizeof(scoreText), "%d", sunshine);
	int textX = 220 - 112, tmp = sunshine;
	while (tmp) {
		textX -= 5;
		tmp /= 10;
	}
	outtextxy(textX, 67, scoreText);
}

void drawZombie() {
	int zombieMax = sizeof(zombies) / sizeof(zombies[0]);
	for (int i = 0; i < zombieMax; i++) {
		if (zombies[i].used) {
			IMAGE* img = NULL;
			if (zombies[i].dead) {
				img = imgZombieDead;
			}
			else if (zombies[i].eating) {
				img = imgZombieEating;
			}
			else {
				img = imgZombie;
			}
			img += zombies[i].frameIndex;

			putimagePNG(zombies[i].x, zombies[i].y - 144, img);
		}
	}
}

void drawBullet() {
	int bulletMax = sizeof(bullets) / sizeof(bullets[0]);
	for (int i = 0; i < bulletMax; i++) {
		if (bullets[i].used) {
			if (bullets[i].blast) {
				IMAGE* img = &imgBulletBlast[bullets[i].frameIndex];
				putimagePNG(bullets[i].x, bullets[i].y, img);
			}
			else {
				putimagePNG(bullets[i].x, bullets[i].y, &imgBulletNormal);
			}
		}
	}
}

void updateWindow() {
	BeginBatchDraw(); //��ʼ����
	
	//��ʾ��Ϸ������ֲ�￨��
	putimage(-112, 15, &imgBackground);
	putimagePNG(180 - 112, 0, &imgBar5);

	drawCards();
	drawPlant();
	drawZombie();
	drawBullet();
	drawSunshine();

	EndBatchDraw(); //��������
}

void updatePlant() {
	static int count = 0;
	if (++count < 3) return;
	count = 0;

	//����ҡ��֡
	for (int i = 0; i < 5; i++) {
		for (int j = 0; j < 9; j++) {
			if (map[i][j].type > 0) {
				map[i][j].frameIndex++;
				int zhiWuType = map[i][j].type - 1;
				int index = map[i][j].frameIndex;
				if (imgZhiWu[zhiWuType][index] == NULL) {
					map[i][j].frameIndex = 0;
				}
			}
		}
	}
}

void createSunshine() {
	static int count;
	count++;
	static int fre = 110;
	if (count >= fre) {
		fre = 500 + rand() % 75;
		count = 0;

		//���������ȡһ����������
		int ballMax = sizeof(balls) / sizeof(balls[0]);
		int i = 0;
		while (i < ballMax && balls[i].used) i++;
		if (i >= ballMax) return;
		
		balls[i].used = 1;
		balls[i].frameIndex = 0;
		balls[i].timer = 0;
		balls[i].status = SUNSHINE_DOWN;
		balls[i].t = 0;
		balls[i].p1 = vector2(260 - 112 + rand() % (900 - 260), 60);
		balls[i].p4 = vector2(balls[i].p1.x, 200 + (rand() % 4) * 90);
		float off = 2.0;
		float distance = balls[i].p4.y - balls[i].p1.y;
		balls[i].speed = 1.5 / (distance / off);
	}

	//���տ���������
	int ballMax = sizeof(balls) / sizeof(balls[0]);
	for (int i = 0; i < 5; i++) {
		for (int j = 0; j < 9; j++) {
			if (map[i][j].type == XIANG_RI_KUI + 1) {
				map[i][j].timer++;

				if (map[i][j].timer > 500) {
					map[i][j].timer = 0;

					int k;
					for (k = 0; k < ballMax && balls[k].used; k++) {
						if (k >= ballMax) return;
					}
					balls[k].used = 1;
					balls[k].p1 = vector2(map[i][j].x, map[i][j].y);
					int w = (30 + rand() % 15) * (rand() % 2 ? 1 : -1);
					balls[k].p4 = vector2(map[i][j].x + w,
						map[i][j].y + imgZhiWu[XIANG_RI_KUI][0]->getwidth() - imgSunshineBall[0].getwidth());
					balls[k].p2 = vector2(balls[k].p1.x + w * 0.3, balls[k].p1.y - 100);
					balls[k].p3 = vector2(balls[k].p1.x + w * 0.7, balls[k].p1.y - 100);
					balls[k].status = SUNSHINE_PRODUCT;
					balls[k].speed = 0.05;
					balls[k].t = 0;
				}
			}
		}
	}
}

void updateSunshine() {
	int ballMax = sizeof(balls) / sizeof(balls[0]);

	for (int i = 0; i < ballMax; i++) {
		if (balls[i].used) {
			balls[i].frameIndex = (balls[i].frameIndex + 1) % 29;
			if (balls[i].status == SUNSHINE_DOWN) {
				struct sunshineBall* sun = &balls[i];
				sun->t += sun->speed;
				sun->currentPresentation = sun->p1 + sun->t * (sun->p4 - sun->p1);
				if (sun->t >= 1) {
					sun->status = SUNSHINE_GROUND;
					sun->timer = 0;
				}
			}
			else if (balls[i].status == SUNSHINE_GROUND) {
				balls[i].timer++;
				if (balls[i].timer > 200) {
					balls[i].timer = 0;
					balls[i].used = 0;
				}
			}
			else if (balls[i].status == SUNSHINE_PRODUCT) {
				struct sunshineBall* sun = &balls[i];
				sun->t += sun->speed;
				sun->currentPresentation = calcBezierPoint(sun->t, sun->p1, sun->p2, sun->p3, sun->p4);
				if (sun->t > 1) {
					sun->status = SUNSHINE_GROUND;
					sun->timer = 0;
				}
			}
		}
		else if (balls[i].status == SUNSHINE_COLLECT) {
			//��������
			mciSendString("play res/sunshine.mp3", 0, 0, 0);

			struct sunshineBall* sun = &balls[i];
			sun->t += sun->speed;
			sun->currentPresentation = sun->p1 + sun->t * (sun->p4 - sun->p1);
			if (sun->t > 1) {
				sun->used = 0;
				sunshine += 25;
				balls[i].status = -1;
			}
		}
	}
}

void createZombie() {
	if (zombieCount >= ZOMBIE_MAX) {
		return;
	}

	static int zombieFrequence = 300;
	static int count = 0;
	count++;
	if (count > zombieFrequence) {
		count = 0;
		zombieFrequence = 500 + rand() % 200;

		int i;
		int zombieMax = sizeof(zombies) / sizeof(zombies[0]);
		for (i = 0; i < zombieMax && zombies[i].used; i++);
		if (i < zombieMax) {
			memset(&zombies[i], 0, sizeof(zombies[i]));
			zombies[i].used = 1;
			zombies[i].x = WIN_WIDTH;
			zombies[i].row = rand() % 5;
			zombies[i].y = 77 + (1 + zombies[i].row) * 102;
			zombies[i].speed = 1;
			zombies[i].blood = 100;
			zombies[i].dead = 0;
			zombieCount++;
		}
	}
}

void updateZombie() {
	int zombieMax = sizeof(zombies) / sizeof(zombies[0]);

	static int count1 = 0;
	count1++;
	if (count1 > 2) {
		count1 = 0;
		//���½�ʬλ��
		for (int i = 0; i < zombieMax; i++) {
			if (zombies[i].used) {
				zombies[i].x -= zombies[i].speed;
				if (zombies[i].x < 56) {
					gameStatus = FAIL;
				}
			}
		}
	}

	static int count2 = 0;
	count2++;
	if (count2 > 2) {
		count2 = 0;
		for (int i = 0; i < zombieMax; i++) {
			if (zombies[i].used) {
				if (zombies[i].dead) {
					zombies[i].frameIndex++;
					if (zombies[i].frameIndex > 11) {	//�����ʬ����������������֡�������ڼ���֮��ɾ����ʬ
						zombies[i].used = 0;
						killCount++;
						if (killCount == ZOMBIE_MAX) {
							gameStatus = WIN;
						}
					}
				}
				else if (zombies[i].eating) {
					zombies[i].frameIndex = (zombies[i].frameIndex + 1) % 21;
				}
				else {
					zombies[i].frameIndex = (zombies[i].frameIndex + 1) % 22;
				}
			}
		}
	}
	
}

void shoot() {
	static int count1 = 0;
	if (++count1 < 3) return;
	count1 = 0;

	int lines[5] = { 0 };
	int zombieMax = sizeof(zombies) / sizeof(zombies[0]);
	int bulletMax = sizeof(bullets) / sizeof(bullets[0]);
	int dangerX = WIN_WIDTH - 30;
	for (int i = 0; i < zombieMax; i++) {
		if (zombies[i].used && zombies[i].x < dangerX) {
			lines[zombies[i].row] = 1;
		}
	}

	for (int i = 0; i < 5; i++) { 
		for (int j = 0; j < 9; j++) {
			if (map[i][j].type == WAN_DOU + 1 && lines[i]) {
				for (int k = 0; k < zombieMax; k++) {
					if (zombies[k].row == i && map[i][j].x < zombies[k].x) {
						map[i][j].shootTime++;
						if (map[i][j].shootTime > 20) {
							map[i][j].shootTime = 0;

							int k = 0;
							for (k = 0; k < bulletMax && bullets[k].used; k++);
							if (k < bulletMax) {
								bullets[k].used = 1;
								bullets[k].row = i;
								bullets[k].speed = 14;

								bullets[k].blast = 0;
								bullets[k].frameIndex = 0;
								int zhiwuX = 256 - 112 + j * 81;
								int zhiwuY = 77 + i * 102 + 27;
								bullets[k].x = zhiwuX + imgZhiWu[map[i][j].type - 1][0]->getwidth() - 10;
								bullets[k].y = zhiwuY;

							}
						}
						break;
					}
				}
				
			}
		}
	}
}

void updateBullets() {
	static int count = 0;
	if (++count < 2) return;
	count = 0;

	int bulletMax = sizeof(bullets) / sizeof(bullets[0]);
	for (int i = 0; i < bulletMax; i++) {
		if (bullets[i].used) {
			bullets[i].x += bullets[i].speed;
			if (bullets[i].x > WIN_WIDTH) {
				bullets[i].used = 0;
			}

			if (bullets[i].blast) {
				bullets[i].frameIndex++;
				if (bullets[i].frameIndex > 3) {
					bullets[i].used = 0;
				}
			}
		}
	}
}

void bullet2Zombie() {
	int bulletMax = sizeof(bullets) / sizeof(bullets[0]);
	int zombieMax = sizeof(zombies) / sizeof(zombies[0]);
	for (int i = 0; i < bulletMax; i++) {
		if (bullets[i].used == 0 || bullets[i].blast) continue;

		for (int j = 0; j < zombieMax; j++) {
			if (zombies[j].used == 0) continue;
			int x1 = zombies[j].x + 80;
			int x2 = zombies[j].x + 110;
			int x = bullets[i].x;
			if (bullets[i].row == zombies[j].row && x > x1 && x < x2) {
				zombies[j].blood -= 10;
				bullets[i].blast = 1;
				bullets[i].speed = 0;

				if (zombies[j].blood <= 0) {
					zombies[j].dead = 1;
					zombies[j].speed = 0;
					zombies[j].frameIndex = 0;
				}
			}
		}
	}
}

void zombie2Plant() {
	int zombieMax = sizeof(zombies) / sizeof(zombies[0]);
	for (int i = 0; i < zombieMax; i++) {

		int row = zombies[i].row;

		for (int j = 0; j < 9; j++) {
			if (map[row][j].type == 0) continue;
			int plantX = 256 - 112 + j * 81;
			int x1 = plantX + 10;
			int x2 = plantX + 60;
			int x3 = zombies[i].x + 80;
			if (x3 > x1 && x3 < x2) {
				zombies[i].eating = 1;
				zombies[i].speed = 0;
				for (int k = 0; k < zombieMax; k++) {
					if (zombies[k].eating) {
						map[row][j].deadTime++;
					}
					if (map[row][j].deadTime > 100) {
						map[row][j].deadTime = 0;
						map[row][j].type = 0;
						zombies[i].eating = 0;
						zombies[i].speed = 1;
					}
				}
			}
		}
	}
}

void collisionCheck() {
	bullet2Zombie();
	zombie2Plant();
}

void updateGame() {
	updatePlant();

	createSunshine(); //��������
	updateSunshine(); //���������״̬

	createZombie(); //������ʬ
	updateZombie(); //���½�ʬ��״̬

	shoot(); //�����㶹�ӵ�
	updateBullets(); //�����㶹�ӵ�
	collisionCheck(); //��ײ���

}

void collectSunshine(ExMessage* msg) {
	int ballMax = sizeof(balls) / sizeof(balls[0]);
	int width = imgSunshineBall[0].getwidth();
	int height = imgSunshineBall[0].getheight();

	for (int i = 0; i < 10; i++) {
		if (balls[i].used) {
			int x = balls[i].currentPresentation.x;
			int y = balls[i].currentPresentation.y;
			if (msg->x > x && msg->x < x + width && msg->y > y && msg->y < y + height) {
				balls[i].used = 0;
				balls[i].status = SUNSHINE_COLLECT;
				balls[i].p1 = balls[i].currentPresentation;
				balls[i].p4 = vector2(262 - 112 - 70, 0);
				balls[i].t = 0;
				float distance = dis(balls[i].p1 - balls[i].p4);
				float off = 8.0;
				balls[i].speed = 3.0 / (distance / off);


				//��������ֵ���ֵ
				if (sunshine >= 9999) {
					sunshine = 9999;
				}
				break;
			}
		}
	}
}

void userClick() {
	ExMessage msg;
	static int status = 0;

	if (peekmessage(&msg)) {
		int mmm = 87;
		//ʵ��ֲ���ѡȡ��ȡ��ѡȡ
		if (msg.message == WM_LBUTTONDOWN && firstDown == 0) {
			if (msg.x > 268 - 112 && msg.x < 268 - 112 + 65 * ZHI_WU_COUNT && msg.y < 96) {
				int index = (msg.x - 268 + 112) / 65;
				status = 1;
				curZhiWu = index + 1;
				firstDown = 1;
				curX = msg.x;
				curY = msg.y;
			}
			else {
				collectSunshine(&msg);
			}
		}
		else if (msg.message == WM_RBUTTONDOWN && firstDown == 1) {
			firstDown = 0;
			curZhiWu = 0;
			status = 0;
		}
		else if (msg.message == WM_MOUSEMOVE && status == 1) {
			curX = msg.x;
			curY = msg.y;

			if (msg.x > 256 - 112 && msg.x < 985 - 112 && msg.y > 77 + 15 && msg.y < 591 + 15) {
				int row = (msg.y - 77) / 102;
				int col = (msg.x - 256 + 112) / 81;
				unmap[row][col].type = curZhiWu;
			}
		}
		else if (msg.message == WM_LBUTTONDOWN && firstDown == 1) {
			if (msg.x > 256 - 112 && msg.x < 985 - 112 && msg.y > 77 + 15 && msg.y < 591 + 15) {
				int row = (msg.y - 77) / 102;
				int col = (msg.x - 256 + 112) / 81;

				if (map[row][col].type == 0) {
					map[row][col].type = curZhiWu;
					map[row][col].frameIndex = 0;
					map[row][col].shootTime = 0;

					map[row][col].x = 256 - 112 + col * 81;
					map[row][col].y = 77 + row * 102;
				}

				firstDown = 0;
				curZhiWu = 0;
				status = 0;
			}
		}
	}
}

void startUI() {
	IMAGE imgBackground, imgMenu[9];
	char name[64];

	//���ز˵������Ԫ��ͼƬ
	loadimage(&imgBackground, "res/menu.png");
	for (int i = 1; i <= 8; i++) {
		sprintf_s(name, sizeof(name), "res/menu%d.png", i);
		loadimage(imgMenu+i, name);
	}

	bool flag[5] = { 0 };
	while (1) {
		BeginBatchDraw();

		putimage(0, 0, &imgBackground);

		//ʵ�ֵ��������ͽ�����ת
		for (int i = 1; i <= 4; i++) {
			putimagePNG(474, -30 + i * 90, flag[i] ? imgMenu + 2 * i : imgMenu + 2 * i - 1);
		}

		ExMessage msg;
		if (peekmessage(&msg)) {
			if (msg.message == WM_LBUTTONDOWN) {
				for (int i = 0; i <= 4; i++) {
					if (msg.x > 474 && msg.x < 474 + 300 && msg.y > -30 + 90 * i && msg.y < -30 + 90 * (i + 1)) {
						flag[i] = 1;
					}
				}
			}
			else if(msg.message == WM_LBUTTONUP && flag) {
				for (int i = 0; i <= 4; i++) {
					if (msg.x > 474 && msg.x < 474 + 300 && msg.y > -30 + 90 * i && msg.y < -30 + 90 * (i + 1)) {
						return;
					}
					else flag[i] = 0;
				}
			}
		}

		EndBatchDraw();
	}
}

void viewScene() {
	int xMin = WIN_WIDTH - imgBackground.getwidth();
	vector2 points[9] = {
		{550, 80}, {530, 160}, {630, 170}, {530, 200}, {515, 270},
		{565,370}, {605, 340}, {705, 280}, {690, 340} };
	int index[9];
	for (int i = 0; i < 9; i++) {
		index[i] = rand() % 11;
	}
	int count = 0;
	for (int x = 0; x >= xMin; x -= 2) {
		BeginBatchDraw();
		putimage(x, 0, &imgBackground);

		count++;
		for (int k = 0; k < 9; k++) {
			putimagePNG(points[k].x - xMin + x, points[k].y, &imgZombieStand[index[k]]);
			if (count >= 10) {
				index[k] = (index[k] + 1) % 11;
			}
		}
		if (count >= 10)count = 0;
		EndBatchDraw();
		Sleep(5);

		//ͣ��һ������
		for (int i = 0; i < 100; i++) {
			BeginBatchDraw();

			putimage(xMin, 0, &imgBackground);
			for (int k = 0; k < 9; k++) {
				putimagePNG(points[k].x, points[k].y, &imgZombieStand[index[k]]);
				index[k] = (index[k] + 1) % 11;
			}
			EndBatchDraw();
			Sleep(30);
		}
		for (int x = xMin; x <= -112; x += 2) {
			BeginBatchDraw();
			putimage(x, 0, &imgBackground);
			count++;
			for (int k = 0; k < 9; k++) {
				putimagePNG(points[k].x - xMin + x, points[k].y, &imgZombieStand[index[k]]);
				if (count >= 10) {
					index[k] = (index[k] + 1) % 11;
				}
				if (count >= 10) count = 0;
			}
			EndBatchDraw();
			Sleep(5);
		}
		break;
	}
}

void barDown() {
	int height = imgBar5.getheight();
	for (int y = -height; y <= 0; y++) {
		BeginBatchDraw();
		putimage(-112, 15, &imgBackground);
		putimagePNG(250 - 112 - 70, y, &imgBar5);

		for (int i = 0; i < ZHI_WU_COUNT; i++) {
			int x = 338 - 112 - 70 + i * 65;

			putimage(x, 6 + y, &imgCards[i]);
		}
		EndBatchDraw();
		Sleep(10);
	}
}

bool checkOver() {
	int ret = false;
	if (gameStatus == WIN) {
		Sleep(2000);
		loadimage(0, "res/win.png");
		ret = true;
	}
	else if (gameStatus == FAIL) {
		Sleep(2000);
		loadimage(0, "res/fail2.png");
		ret = true;
	}
	return ret;
}

int main() {
	gameInit();
	startUI();
	//viewScene();
	//barDown();

	int timer = 0;
	while (1) {
		userClick();
		//�ӳ���������������ٶ�
		timer += getDelay();
		if (timer > 30) {
			timer = 0;
			updateWindow();
			updateGame();
			if (checkOver()) break;
		}
	}

	system("pause");
	return 0;
}