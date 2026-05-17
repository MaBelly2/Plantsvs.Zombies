#include "Scene.h"
#include <cstdio>
#include <ctime>
#include <mmsystem.h>
#pragma comment(lib, "Winmm.lib")

Scene* Scene::create()
{
	Scene* scene = new Scene;
	if (scene->init())
	{
		return scene;
	}
	delete scene;
	return nullptr;
}

Scene::~Scene()
{
	// 1. 清理 UI 卡片
	for (auto card : m_cards) {
		delete card;
	}
	m_cards.clear();

	// 2. 清理草坪上的手推车
	for (int i = 0; i < 5; ++i) {
		if (m_lawnmowers[i] != nullptr) {
			delete m_lawnmowers[i];
			m_lawnmowers[i] = nullptr;
		}
	}

	// 3. 清理网格上残留的植物
	for (int i = 0; i < 5; ++i) {
		for (int j = 0; j < 9; ++j) {
			if (m_PlantTable[i][j] != nullptr) {
				delete m_PlantTable[i][j];
				m_PlantTable[i][j] = nullptr;
			}
		}
	}

	// 4. 清理残余的僵尸、子弹和阳光
	for (auto zombie : m_zombies) {
		delete zombie;
	}
	m_zombies.clear();

	for (auto bullet : m_bullets) {
		delete bullet;
	}
	m_bullets.clear();

	for (auto sun : m_suns) {
		delete sun;
	}
	m_suns.clear();
}

bool Scene::init()
{
	// 播放背景音乐
	mciSendString("open assets/music/bgm.mp3 alias bgm", NULL, 0, NULL);
	mciSendString("play bgm repeat", NULL, 0, NULL);

	loadimage(&m_BackgroundImg, "assets/DaytimeBackground.jpg", 1280, 720);

	// ====== 加载铲子图片 ======
	loadimage(&m_ShovelSlotImg, "assets/PlantShovel.png", 60, 60);
	loadimage(&m_ShovelImg, "assets/PlantShovel.png", 60, 60);

	// 初始化植物网格
	for (int i = 0; i < 5; ++i) {
		for (int j = 0; j < 9; ++j) {
			m_PlantTable[i][j] = nullptr;
		}
	}

	m_sun = 50;	// 开局默认给 50 阳光

	// ====== 【融合点】初始化全新 UI 卡片 ======
	// 注意：确保图片已转为 .png 格式。
	Card* peaCard = new Card();
	peaCard->init(PEASHOOTER, 120, 2, 65, 85, "assets/Card/Peashooter.png");
	m_cards.push_back(peaCard);

	Card* sunCard = new Card();
	sunCard->init(SUNFLOWER, 185, 2, 65, 85, "assets/Card/Sunflower.png");
	m_cards.push_back(sunCard);

	Card* snowCard = new Card();
	snowCard->init(SNOWPEA, 250, 2, 65, 85, "assets/Card/SnowPea.png");
	m_cards.push_back(snowCard);

	Card* nutCard = new Card();
	nutCard->init(WALLNUT, 315, 2, 65, 85, "assets/Card/Wallnut.png");
	m_cards.push_back(nutCard);

	// ====== 【融合点】推车的初始化 ======
	for (int i = 0; i < 5; ++i) {
		// 根据你的测量：行高 118，起始 Y 100
		int mowerY = 100 + i * 118;
		m_lawnmowers[i] = Lawnmower::create(Vec2(25, mowerY), 80, 80);
	}

	m_isHoldingPlant = false;	// 初始状态手里没有植物
	m_isHoldingShovel = false;	// 初始没拿铲子

	// ====== 胜负状态初始化 ======
	m_status = PLAYING;
	m_zombiesGenerated = 0;
	m_maxZombies = 10;

	return true;
}

void Scene::drawTick()
{
	// 1. 绘制背景
	putimage_alpha(0, 0, &m_BackgroundImg);

	// 2. 绘制顶部的所有卡片（卡片内部自动处理冷却遮罩层）
	for (auto card : m_cards) {
		card->drawTick();
	}

	// 3. 【融合点】渲染固定位置的铲子图标
	putimage_alpha(1190, 10, &m_ShovelSlotImg);

	// 4. 绘制植物
	for (int i = 0; i < 5; ++i) {
		for (int j = 0; j < 9; ++j) {
			if (m_PlantTable[i][j]) m_PlantTable[i][j]->drawTick();
		}
	}

	// 5. 绘制推车
	for (int i = 0; i < 5; ++i) {
		if (m_lawnmowers[i]) m_lawnmowers[i]->drawTick();
	}

	// 6. 绘制僵尸
	for (auto zombie : m_zombies) zombie->drawTick();

	// 7. 绘制子弹
	for (auto bullet : m_bullets) bullet->drawTick();

	// 8. 绘制实体阳光
	for (auto sun : m_suns) sun->drawTick();

	// 9. 【融合点】渲染 UI：阳光数量
	settextcolor(YELLOW);
	settextstyle(24, 0, "微软雅黑", 0, 0, FW_BOLD, false, false, false);
	char sunText[32];
	sprintf_s(sunText, "%d", m_sun);
	outtextxy(53, 65, sunText);

	// 10. 如果你拿着铲子，让铲子跟着鼠标动
	if (m_isHoldingShovel) {
		POINT pt;
		GetCursorPos(&pt);
		ScreenToClient(GetHWnd(), &pt);
		putimage_alpha(pt.x - 30, pt.y - 30, &m_ShovelImg);
	}

	// 11. 结算画面的文字渲染
	if (m_status == GAME_OVER) {
		setfillcolor(DARKGRAY);
		solidrectangle(0, 0, 1280, 720);
		settextcolor(RED);
		settextstyle(100, 0, "微软雅黑", 0, 0, FW_BOLD, false, false, false);
		const char* text = "僵尸吃掉了你的脑子！";
		outtextxy((1280 - textwidth(text)) / 2, (720 - textheight(text)) / 2, text);
	}
	else if (m_status == WIN) {
		settextcolor(YELLOW);
		settextstyle(120, 0, "微软雅黑", 0, 0, FW_BOLD, false, false, false);
		const char* text = "通 关 ！";
		outtextxy((1280 - textwidth(text)) / 2, (720 - textheight(text)) / 2, text);
	}
}

void Scene::eventTick(float delta)
{
	if (m_status != PLAYING) {
		return;
	}

	float sec = delta / 1000.0f;

	// 1. 更新卡片冷却计时器
	for (auto card : m_cards) {
		card->eventTick(delta);
	}

	// 2. 僵尸自动生成系统
	if (m_zombiesGenerated < m_maxZombies) {
		m_zombieTimer += sec;
		if (m_zombieTimer >= m_zombieInterval) {
			m_zombieTimer = 0;
			int row = rand() % 5;

			// 【融合点】根据你的计算微调高度
			int spawnY = 100 + row * 118 - 20;

			Zombie* z = Zombie::create(NORMAL_ZOMBIE, Vec2(1280, spawnY), 80, 100);
			if (z) {
				m_zombies.push_back(z);
			}
			m_zombiesGenerated++;

			if (m_zombieInterval > 1.5f) m_zombieInterval -= 0.1f;
		}
	}

	// 3. 推车生成系统
	for (int i = 0; i < 5; ++i) {
		if (m_lawnmowers[i]) m_lawnmowers[i]->eventTick(delta);
	}

	// 4. 天空掉落阳光
	m_skySunTimer += sec;
	if (m_skySunTimer >= m_skySunInterval) {
		m_skySunTimer = 0;
		int spawnX = rand() % (1280 - 80);
		int targetY = 200 + rand() % 300;
		m_suns.push_back(Sun::create(Vec2(spawnX, -80), targetY));
	}

	// 更新阳光逻辑并清理过期的
	for (auto it = m_suns.begin(); it != m_suns.end(); ) {
		(*it)->eventTick(delta);
		if ((*it)->isCollected() || (*it)->isExpired()) {
			delete* it;
			it = m_suns.erase(it);
		}
		else {
			++it;
		}
	}

	// ====== 【重大修复】统一的扁平化鼠标事件处理，杜绝死循环 ======
	ExMessage msg;
	while (peekmessage(&msg, EX_MOUSE)) {
		if (msg.message == WM_RBUTTONDOWN) {
			m_isHoldingPlant = false;
			m_isHoldingShovel = false;
		}
		else if (msg.message == WM_LBUTTONDOWN) {
			bool clickedSun = false;

			// ① 判定是否点中了地上的阳光
			for (auto sun : m_suns) {
				if (msg.x >= sun->getX() && msg.x <= sun->getX() + sun->getWidth() &&
					msg.y >= sun->getY() && msg.y <= sun->getY() + sun->getHeight()) {
					sun->collect();
					m_sun += 25;
					clickedSun = true;
					break;
				}
			}

			if (!clickedSun) {
				bool clickedCard = false;

				// ② 判定是否点击了顶部的某张植物卡片
				for (auto card : m_cards) {
					if (card->isClicked(msg.x, msg.y)) {
						clickedCard = true;
						if (card->isReady(m_sun)) {
							m_isHoldingPlant = true;
							m_holdingType = card->getType();
							m_isHoldingShovel = false;
						}
						break;
					}
				}

				if (!clickedCard) {
					// ③ 【融合点】判定是否点中了铲子槽位 (1180~1260, 0~70)
					if (msg.x >= 1180 && msg.x <= 1260 && msg.y >= 0 && msg.y <= 70) {
						m_isHoldingShovel = true;
						m_isHoldingPlant = false;
					}
					// ④ 点击了草坪区域
					else {
						int row, col;
						if (getGridIndex(msg.x, msg.y, row, col)) {
							// 种植物
							if (m_isHoldingPlant && m_PlantTable[row][col] == nullptr) {
								int cost = Plant::getCost(m_holdingType);
								m_sun -= cost;

								// 【融合点】使用你测算出来的绝对坐标！为了让 80x100 的植物在 116x118 的格子里居中，我加了一点点偏移补偿 (+18, +9)
								int plantX = 105 + col * 116 + 18;
								int plantY = 100 + row * 118 + 9;

								m_PlantTable[row][col] = Plant::create(m_holdingType, Vec2(plantX, plantY), 80, 100);

								for (auto card : m_cards) {
									if (card->getType() == m_holdingType) {
										card->resetCD();
										break;
									}
								}
								m_isHoldingPlant = false;
							}
							// 挖植物
							else if (m_isHoldingShovel && m_PlantTable[row][col] != nullptr) {
								delete m_PlantTable[row][col];
								m_PlantTable[row][col] = nullptr;
								m_isHoldingShovel = false;
							}
						}
						else {
							m_isHoldingPlant = false;
							m_isHoldingShovel = false;
						}
					}
				}
			}
		}
	}

	// 5. 实体逻辑更新：植物生产与子弹射击
	for (int i = 0; i < 5; ++i) {
		for (int j = 0; j < 9; ++j) {
			Plant* p = m_PlantTable[i][j];
			if (p) {
				p->eventTick(delta);
				if (p->shouldFire()) {
					Bullet* b = Bullet::create(NORMAL_BULLET, Vec2(p->getX() + 40, p->getY() + 20), 30, 30);
					m_bullets.push_back(b);
					p->resetFireFlag();
				}
				if (p->getType() == SUNFLOWER && p->shouldSpawnSun()) {
					m_suns.push_back(Sun::create(Vec2(p->getX() + 20, p->getY()), p->getY() + 40));
					p->resetSunFlag();
				}
			}
		}
	}

	// 更新子弹
	for (auto bullet : m_bullets) {
		bullet->eventTick(delta);
	}

	// 更新僵尸
	for (auto zombie : m_zombies) {
		zombie->eventTick(delta);
	}

	// 碰撞检测与死亡结算
	checkCollision(delta);
	cleanUp();

	// 胜负判定检测
	for (auto zombie : m_zombies) {
		// 僵尸突破了屋子左侧防线
		if (!zombie->isDead() && zombie->getX() < -50) {
			m_status = GAME_OVER;
			return;
		}
	}
	if (m_zombiesGenerated >= m_maxZombies && m_zombies.empty()) {
		m_status = WIN;
	}
}

void Scene::checkCollision(float delta)
{
	// 1. 子弹打僵尸
	for (auto bullet : m_bullets) {
		if (!bullet->isActive()) continue;

		for (auto zombie : m_zombies) {
			if (zombie->isDead()) continue;

			bool isOverlap = !(
				bullet->getX() + bullet->getWidth() < zombie->getX() ||
				zombie->getX() + zombie->getWidth() < bullet->getX() ||
				bullet->getY() + bullet->getHeight() < zombie->getY() ||
				zombie->getY() + zombie->getHeight() < bullet->getY()
				);

			if (isOverlap) {
				zombie->setHp(zombie->getHp() - bullet->getAttack());
				bullet->setActive(false);
				break;
			}
		}
	}

	// 2. 僵尸吃植物
	for (auto zombie : m_zombies) {
		if (zombie->isDead()) continue;

		bool isTouchingPlant = false;
		for (int i = 0; i < 5; ++i) {
			for (int j = 0; j < 9; ++j) {
				Plant* plant = m_PlantTable[i][j];
				if (plant) {
					bool isOverlap = !(
						plant->getX() + plant->getWidth() < zombie->getX() ||
						zombie->getX() + zombie->getWidth() < plant->getX() ||
						plant->getY() + plant->getHeight() < zombie->getY() ||
						zombie->getY() + zombie->getHeight() < plant->getY()
						);

					if (isOverlap) {
						isTouchingPlant = true;
						zombie->setEating(true);

						int damage = zombie->getDamage(delta);
						if (damage > 0) {
							plant->setHp(plant->getHp() - damage);
							if (plant->getHp() <= 0) {
								delete plant;
								m_PlantTable[i][j] = nullptr;
							}
						}
						break;
					}
				}
			}
		}
		if (!isTouchingPlant) {
			zombie->setEating(false);
		}
	}

	// 3. 撑杆僵尸跳跃侦测
	for (auto zombie : m_zombies) {
		if (zombie->getType() == POLE_VAULTING_ZOMBIE) {
			int zombieRow, zombieCol;
			zombie->getGridPosition(zombieRow, zombieCol);
			int frontCol = zombieCol - 1;
			if (frontCol >= 0 && m_PlantTable[zombieRow][frontCol] != nullptr) {
				zombie->setPlantAhead(true);
			}
		}
	}

	// 4. 推车碾压僵尸
	for (int i = 0; i < 5; ++i) {
		Lawnmower* lm = m_lawnmowers[i];
		if (!lm) continue;

		for (auto zombie : m_zombies) {
			if (zombie->isDead()) continue;

			bool isOverlap = !(
				lm->getPos().x + lm->getWidth() < zombie->getPos().x ||
				zombie->getPos().x + zombie->getWidth() < lm->getPos().x ||
				lm->getPos().y + lm->getHeight() < zombie->getPos().y ||
				zombie->getPos().y + zombie->getHeight() < lm->getPos().y
				);

			if (isOverlap) {
				if (!lm->isMoving()) {
					lm->trigger();
				}
				zombie->setHp(0);
			}
		}
	}
}

void Scene::cleanUp()
{
	for (auto it = m_zombies.begin(); it != m_zombies.end();) {
		if ((*it)->isRemovable()) {
			delete* it;
			it = m_zombies.erase(it);
		}
		else {
			++it;
		}
	}

	for (auto it = m_bullets.begin(); it != m_bullets.end();) {
		if (!(*it)->isActive() || (*it)->isOutOfScreen()) {
			delete* it;
			it = m_bullets.erase(it);
		}
		else {
			++it;
		}
	}

	for (int i = 0; i < 5; ++i) {
		if (m_lawnmowers[i] && m_lawnmowers[i]->isRemovable()) {
			delete m_lawnmowers[i];
			m_lawnmowers[i] = nullptr;
		}
	}
}

bool Scene::getGridIndex(int x, int y, int& row, int& col)
{
	// 【融合点】采用你的精确网格测量数据
	const int GRID_START_X = 105;
	const int GRID_START_Y = 100;
	const int CELL_WIDTH = 116;
	const int CELL_HEIGHT = 118;

	if (x < GRID_START_X || x > GRID_START_X + 9 * CELL_WIDTH ||
		y < GRID_START_Y || y > GRID_START_Y + 5 * CELL_HEIGHT) {
		return false;
	}

	col = (x - GRID_START_X) / CELL_WIDTH;
	row = (y - GRID_START_Y) / CELL_HEIGHT;

	return true;
}