#include "Scene.h"
#include"Zombie.h"
#include <cstdio>
#include <mmsystem.h>
#pragma comment(lib, "Winmm.lib")

Scene* Scene::create()
{
	Scene* scene = new Scene;
	if (scene->init())  //如果初始化成功
	{
		return scene;
	}
	delete scene;  //错了 删掉这个场景
	return nullptr;
}

//把 init 当作初始化函数在使用  变成一个构造函数在使用
bool Scene::init()
{
	// 播放背景音乐 (alias bgm 是给这段音频起个别名，repeat 表示循环播放)
	mciSendString("open assets/music/bgm.mp3 alias bgm", NULL, 0, NULL);
	mciSendString("play bgm repeat", NULL, 0, NULL);

	loadimage(&m_BackgroundImg, "assets/DaytimeScene.jpg");

	// ====== 加载铲子图片 ======
	// 【A：找好铲子的图片素材，放到 assets 文件夹里，并根据实际调整大小及路径】
	// 假设槽位大小是 60x60，手里的铲子大小也是 60x60
	loadimage(&m_ShovelSlotImg, "assets/PlantShovel.png", 60, 60);
	loadimage(&m_ShovelImg, "assets/PlantShovel.png", 60, 60);

	// 初始化植物
	for (int i = 0; i < 5; ++i){
		for (int j = 0; j < 9; ++j){
			m_PlantTable[i][j] = nullptr;
		}
	}

	m_sun = 50;					// 开局默认给 50 阳光

	// 推车的初始化
	for (int i = 0; i < 5; ++i) {
		// 根据格子Y坐标计算推车位置 (假设房子在 x=150 的位置)
		// 根据之前刷僵尸的公式：80 + row * 100
		int mowerY = 80 + i * 100;

		// 【D：根据背景图微调 X 坐标，让推车刚好停在草坪最左侧外】
		m_lawnmowers[i] = Lawnmower::create(Vec2(150, mowerY), 80, 80);
	}
	
	// 初始化卡片 CD 为 0
	//【B：记得添加】
	m_cardCDTimers[PEASHOOTER] = 0.0f;
	m_cardCDTimers[SUNFLOWER] = 0.0f;

	m_isHoldingPlant = false;	// 初始状态手里没有植物
	m_isHoldingShovel = false;	// 初始没拿铲子

	// ====== 胜负状态初始化 ======
	m_status = PLAYING;
	m_zombiesGenerated = 0;
	//【B：根据实际情况修改数量，自行决定 可与D商量 根据波数而定】
	m_maxZombies = 10; // 设定本关出 10 只僵尸就能赢

	return true;
}

void Scene::drawTick()
{
	//1. 绘制背景
	putimage(0, 0, &m_BackgroundImg);

	//2. 绘制植物
	for (int i = 0; i < 5; ++i) {
		for (int j = 0; j < 9; ++j) {
			if (m_PlantTable[i][j]) m_PlantTable[i][j]->drawTick();
		}
	}

	//3. 绘制推车
	for (int i = 0; i < 5; ++i) {
		if (m_lawnmowers[i]) m_lawnmowers[i]->drawTick();
	}

	//4. 绘制僵尸
	for (auto zombie : m_zombies) zombie->drawTick();	//基于范围的循环  auto 自动推导类型 zombie是容器中元素的引用/拷贝
	
	//5. 绘制子弹
	for (auto bullet : m_bullets) bullet->drawTick();

	// 渲染 UI：阳光数量
	// 设置文字样式、颜色并输出到左上角
	// 【D：根据实际情况调试位置】
	settextcolor(BLACK);
	settextstyle(30, 0, "微软雅黑");

	char sunText[32];
	sprintf_s(sunText, "阳光: %d", m_sun);	// 将整数格式化为字符串
	outtextxy(20, 20, sunText);				// 在屏幕 (20, 20) 的位置输出文字

	// ====== 3. 绘制卡片冷却遮罩 ======
	// 【A：提供卡片的具体坐标和宽高】
	// 假设豌豆射手卡片坐标是 (x: 100, y: 10)，宽高是 (w: 50, h: 70)
	int peaCardX = 100, peaCardY = 10, cardW = 50, cardH = 70;

	if (m_cardCDTimers[PEASHOOTER] > 0) {
		// 计算冷却比例 (0.0 到 1.0 之间)
		float maxCD = Plant::getCardCooldown(PEASHOOTER);
		float ratio = m_cardCDTimers[PEASHOOTER] / maxCD;

		// 计算遮罩层的高度 (比例越大，遮罩越高)
		int coverHeight = (int)(cardH * ratio);

		// 设置绘制无边框的灰色半透明矩形（EasyX 标准库不支持真半透明，用深灰色模拟）
		setfillcolor(DARKGRAY);
		// 画一个矩形遮住卡片，从底部慢慢向下收缩效果
		solidrectangle(peaCardX, peaCardY, peaCardX + cardW, peaCardY + coverHeight);
	}

	// 向日葵卡片的遮罩同理，换成向日葵卡片的坐标去画即可
	//【A：补充，或者优化代码实现】

	// 3. 渲染固定位置的铲子图标
	putimage(500, 10, &m_ShovelSlotImg);

	// 4. 如果你拿着铲子，让铲子跟着鼠标动
	if (m_isHoldingShovel) {
		POINT pt;
		GetCursorPos(&pt);				// 获取屏幕鼠标坐标
		ScreenToClient(GetHWnd(), &pt); // 转换为 EasyX 窗口内部坐标

		// 在鼠标位置画一个铲子 (假设 m_ShovelImg 是铲子的图片)
		putimage(pt.x - 30, pt.y - 30, &m_ShovelImg); 
	}

	// 2. 结算画面的文字渲染
	if (m_status == GAME_OVER) {
		// 画一个半透明的黑色遮罩
		setfillcolor(DARKGRAY);
		solidrectangle(0, 0, 1280, 720);

		settextcolor(RED); // 失败用红色
		settextstyle(100, 0, "微软雅黑", 0, 0, FW_BOLD, false, false, false);

		// 让文字尽量居中
		const char* text = "僵尸吃掉了你的脑子！";
		int textW = textwidth(text);
		int textH = textheight(text);
		outtextxy((1280 - textW) / 2, (720 - textH) / 2, text);
	}
	else if (m_status == WIN) {
		settextcolor(YELLOW); // 胜利用金黄色
		settextstyle(120, 0, "微软雅黑", 0, 0, FW_BOLD, false, false, false);

		const char* text = "通 关 ！";
		int textW = textwidth(text);
		int textH = textheight(text);
		outtextxy((1280 - textW) / 2, (720 - textH) / 2, text);
	}
}

void Scene::eventTick(float delta)
{
	// 如果游戏已经结束（赢了或输了），直接 return，冻结所有逻辑！
	if (m_status != PLAYING) {
		return;
	}

	float sec = delta / 1000.0f;

	// ====== 1. 更新卡片冷却计时器 ======
	for (auto& pair : m_cardCDTimers) {
		if (pair.second > 0) {
			pair.second -= sec; // 随着时间流逝减少 CD
		}
	}

	// ====== 僵尸自动生成系统 ======
	if (m_zombiesGenerated < m_maxZombies) {
		m_zombieTimer += sec;
		if (m_zombieTimer >= m_zombieInterval) {
			m_zombieTimer = 0; // 计时器清零

			// 随机选择一行 (0-4)
			int row = rand() % 5;
			// 计算该行对应的 Y 坐标（需根据你的格子坐标微调）
			//【D：】
			int spawnY = 80 + row * 100;

			// 在屏幕最右侧外一点点（1280）生成僵尸
			//【D：】
			Zombie* z = Zombie::create(NORMAL_ZOMBIE, Vec2(1280, spawnY), 80, 100);
			if (z) {
				m_zombies.push_back(z);
			}

			m_zombiesGenerated++; // 生成计数+1

			// 随着时间推移缩短刷怪间隔，增加难度
			//【D：实现一个 m_zombieInterval 的递减函数，控制难度】
			if (m_zombieInterval > 1.5f) m_zombieInterval -= 0.1f;	//简单示例，可以更复杂一点
		}
	}

	// ====== 推车生成系统 ======
	for (int i = 0; i < 5; ++i) {
		if (m_lawnmowers[i]) m_lawnmowers[i]->eventTick(delta);
	}

	// ====== 阳光自动生成系统 ======
	// 1. 处理天空掉落阳光
	m_skySunTimer += sec;
	if (m_skySunTimer >= m_skySunInterval) {
		m_skySunTimer = 0;
		//【D：根据实际情况调整】
		int spawnX = rand() % (1280 - 80); // 随机横坐标
		int targetY = 200 + rand() % 300;  // 随机掉落高度
		m_suns.push_back(Sun::create(Vec2(spawnX, -80), targetY));
	}

	// 2. 更新阳光逻辑并清理过期的
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

	// 3. 处理鼠标点击收集阳光
	ExMessage msg;
	while (peekmessage(&msg, EX_MOUSE)) {
		if (msg.message == WM_LBUTTONDOWN) {
			bool clickedSun = false;
			for (auto sun : m_suns) {
				// 简单的范围检测
				if (msg.x >= sun->getX() && msg.x <= sun->getX() + sun->getWidth() &&
					msg.y >= sun->getY() && msg.y <= sun->getY() + sun->getHeight()) {
					sun->collect();
					m_sun += 25; // 收集一个加25阳光
					clickedSun = true;
					break;
				}
			}
			if (!clickedSun) {
				// 如果没点到阳光，再走种植物逻辑...
				//====== 2. 处理鼠标交互 ======
				while (peekmessage(&msg, EX_MOUSE))
				{
					// ====== 右键取消操作 ======
					if (msg.message == WM_RBUTTONDOWN) {
						m_isHoldingPlant = false;
						m_isHoldingShovel = false;
					}

					// ====== 左键点击操作 ======
					if (msg.message == WM_LBUTTONDOWN)
					{
						// 交互逻辑 A：判断是否点击了顶部的植物卡片
						//【D：根据实际背景测量并修改】
						if (/*鼠标点中了豌豆射手卡片*/false) {
							if (m_sun >= Plant::getCost(PEASHOOTER) && m_cardCDTimers[PEASHOOTER] <= 0) {
								m_isHoldingPlant = true;
								m_isHoldingPlant = PEASHOOTER;
							}
						}
						else if (/*鼠标点中了向日葵卡片*/false) {
							if (m_sun >= Plant::getCost(SUNFLOWER) && m_cardCDTimers[SUNFLOWER] <= 0) {
								m_isHoldingPlant = true;
								m_isHoldingPlant = SUNFLOWER;
							}
						}
					}

					// 交互逻辑 B：如果手里拿着植物，并且点击了草坪
					//【D：根据实际背景测量并修改】
					else if (m_isHoldingPlant)
					{
						int row, col;
						// 将鼠标坐标转换为网格索引
						if (getGridIndex(msg.x, msg.y, row, col))
						{
							//检查该格子是否为空
							if (m_PlantTable[row][col] == nullptr)
							{
								// 1. 扣除阳光（组长可以将不同植物的造价写在一个配置表或函数里）
								int cost = Plant::getCost(m_holdingType);
								m_sun -= cost;

								// 2. 计算植物实际的像素坐标 (这里以格子左上角为例)
								// 【D：调试偏移量 使植物居中】
								int plantX = 250 + col * 80;
								int plantY = 80 + row * 100;

								// 3. 种下植物！
								m_PlantTable[row][col] = Plant::create(m_holdingType, Vec2(plantX, plantY), 80, 100);
								m_cardCDTimers[m_holdingType] = Plant::getCardCooldown(m_holdingType);		// 种下后，对应植物卡片进入冷却！
								
								// 4. 重置状态
								m_isHoldingPlant = false;
							}
						}
					}

					// 交互逻辑 C：判断是否点中了顶部的【铲子槽位】
					// 假设 把铲子画在了 (x: 500, y: 10) 宽 60 高 60 的位置
					// 【A：根据实际情况调试，填写】
					else if (/* 鼠标点中铲子槽的判定：msg.x, msg.y 在范围内 */ false) {
						m_isHoldingShovel = true;
						m_isHoldingPlant = false; // 如果手里正拿着植物，强制放下
					}

					// 交互逻辑 D：如果手里【拿着铲子】，并且点击了草坪
					else if (m_isHoldingShovel) {
						int row, col;
						if (getGridIndex(msg.x, msg.y, row, col)) {
							// 检查这格到底有没有植物
							if (m_PlantTable[row][col] != nullptr) {
								// 【核心操作】
								// 1. 释放该植物占用的内存，防止内存泄漏！
								delete m_PlantTable[row][col];
								// 2. 把表格里的指针清空，否则会变成野指针，僵尸一碰就崩溃！
								m_PlantTable[row][col] = nullptr;
							}
							// 不管有没有挖到东西，点完草坪就把铲子收起来（原版逻辑）
							m_isHoldingShovel = false;
						}
					}

					else {
						// 如果点到了草坪外面，说明玩家想取消种植
						m_isHoldingPlant = false;
					}
				}
			}
		}
	}

	//====== 3. 实体逻辑更新 ======
	//1. 更新植物
	for (int i = 0; i < 5; ++i) {
		for (int j = 0; j < 9; ++j) {
			Plant* p = m_PlantTable[i][j];
			if (p) {
				p->eventTick(delta);
				// 检查植物是否准备好射击
				if (p->shouldFire()) {
					// 创建子弹，起始坐标设为植物中心偏右
					//【D：根据实际情况调整参数】
					Bullet* b = Bullet::create(NORMAL_BULLET, Vec2(p->getX() + 40, p->getY() + 20), 30, 30);
					m_bullets.push_back(b);	//把薪子弹放到列表末尾
					p->resetFireFlag();		// 射完了，把旗子放下来
				}
				if (p->getType() == SUNFLOWER && p->shouldSpawnSun()) {
					// 在向日葵旁边弹出一个阳光
					// 目标高度设为植物脚下附近
					m_suns.push_back(Sun::create(Vec2(p->getX() + 20, p->getY()), p->getY() + 40));
					p->resetSunFlag();
				}
			}
		}
	}

	//2. 更新子弹
	for (auto bullet : m_bullets) {
		bullet->eventTick(delta);
	}

	//3. 更新僵尸位置
	for (auto zombie : m_zombies) {
		zombie->eventTick(delta);
	}

	//4. 碰撞检测与死亡结算
	checkCollision(delta);
	cleanUp();

	// ====== 3. 【新增】胜负判定检测 ======
	// 检测失败条件：有没有僵尸走进了左侧屋子？（假设 x < -50 算进屋）
	for (auto zombie : m_zombies) {
		if (!zombie->isDead() && zombie->getX() < -50) {
			m_status = GAME_OVER;
			return; // 一旦失败，立刻结束判定
		}
	}

	// 检测胜利条件：名额用完，且场上僵尸死光了（被 cleanUp 清理掉了）
	if (m_zombiesGenerated >= m_maxZombies && m_zombies.empty()) {
		m_status = WIN;
	}
}

void Scene::checkCollision(float delta)
{
	// ====== 1. 子弹打僵尸 ======
	//遍历所有存活的子弹
	for (auto bullet : m_bullets) {
		if (!bullet->isActive())continue;	//如果子弹已经失效，跳过

		//遍历所有存活僵尸
		for (auto zombie : m_zombies) {
			if (zombie->isDead())continue;	//如果僵尸已经死了，跳过

			//矩形碰撞检测算法
			//原理：判断两个矩形不重叠的四个条件，如果都不满足，就一定重叠
			bool isOverlap = !(
				bullet->getX() + bullet->getWidth() < zombie->getX() ||		//子弹在僵尸左边
				zombie->getX() + zombie->getWidth() < bullet->getX() ||		//僵尸在子弹左边
				bullet->getY() + bullet->getHeight() < zombie->getY() ||	//子弹在僵尸上边
				zombie->getY() + zombie->getHeight() < bullet->getY()		//僵尸在子弹上边
				);

			if (isOverlap) {
				//发生碰撞
				zombie->setHp(zombie->getHp() - bullet->getAttack());		//僵尸扣血
				bullet->setActive(false);									//子弹标记为失效
				break;	//子弹已打中，无需再与后续僵尸判断
			}
		}
	}

	// ====== 2. 僵尸吃植物 ======
	for (auto zombie : m_zombies) {
		if (zombie->isDead()) continue;

		bool isTouchingPlant = false; // 标记这只僵尸当前有没有碰到植物

		// 遍历草坪上的所有植物
		for (int i = 0; i < 5; ++i) {
			for (int j = 0; j < 9; ++j) {
				Plant* plant = m_PlantTable[i][j];
				if (plant) {
					// AABB 矩形碰撞检测
					bool isOverlap = !(
						plant->getX() + plant->getWidth() < zombie->getX() ||
						zombie->getX() + zombie->getWidth() < plant->getX() ||
						plant->getY() + plant->getHeight() < zombie->getY() ||
						zombie->getY() + zombie->getHeight() < plant->getY()
						);

					if (isOverlap) {
						isTouchingPlant = true;
						zombie->setEating(true); // 僵尸停止移动

						// 尝试获取僵尸的伤害（内部会自动判断攻击冷却是否结束）
						int damage = zombie->getDamage(delta);
						if (damage > 0) {
							plant->setHp(plant->getHp() - damage);
							printf("僵尸啃了植物一口，植物剩余血量: %d\n", plant->getHp());

							// 判断植物是否被吃掉
							if (plant->getHp() <= 0) {
								delete plant;               // 释放植物内存
								m_PlantTable[i][j] = nullptr; // 表格置空
								// 注意：植物死后，下一帧 isOverlap 就会变 false，僵尸自动恢复行走
							}
						}
						break; // 一只僵尸一次只能啃一株植物
					}
				}
			}
		}

		// 如果遍历完所有植物，发现都没碰着，说明面前没障碍了，继续走
		if (!isTouchingPlant) {
			zombie->setEating(false);
		}
	}
	for (auto zombie : m_zombies) {
		//检查撑杆僵尸前一格是否有植物
		if (zombie->getType() == POLE_VAULTING_ZOMBIE) {
			int zombieRow, zombieCol;
			zombie->getGridPosition(zombieRow, zombieCol);
			int frontCol = zombieCol - 1;
			if (frontCol >= 0 && m_PlantTable[zombieRow][frontCol] != nullptr) {
				zombie->setPlantAhead(true);
			}
		}
	}

	// ====== 3. 推车与僵尸的碰撞 ======
	for (int i = 0; i < 5; ++i) {
		Lawnmower* lm = m_lawnmowers[i];
		if (!lm) continue; // 这行的推车如果已经用掉了，就跳过

		for (auto zombie : m_zombies) {
			if (zombie->isDead()) continue;

			// AABB 矩形碰撞检测
			bool isOverlap = !(
				lm->getPos().x + lm->getWidth() < zombie->getPos().x ||
				zombie->getPos().x + zombie->getWidth() < lm->getPos().x ||
				lm->getPos().y + lm->getHeight() < zombie->getPos().y ||
				zombie->getPos().y + zombie->getHeight() < lm->getPos().y
				);

			if (isOverlap) {
				// 如果推车还没动，触发它！
				if (!lm->isMoving()) {
					lm->trigger();
				}

				// 只要发生了碰撞（不管是刚触发还是推车正在冲锋），僵尸直接秒杀！
				zombie->setHp(0);
				// 如果想让秒杀的僵尸直接灰飞烟灭，也可以在这里直接修改僵尸的状态为DIE
			}
		}
	}
}
void Scene::cleanUp()
{
	// 1. 清理死掉的僵尸
	for (auto it = m_zombies.begin(); it != m_zombies.end();) {	//写法和上面不一样：避免迭代器失效  (前面是自动递增)
		if ((*it)->isRemovable()) {
			delete* it;					//释放内存
			it = m_zombies.erase(it);	//从 vector 中移除，并获取下一个有效的迭代器
		}
		else {
			++it;
		}
	}

	// 2. 清理打中目标 或 飞出屏幕的子弹
	for (auto it = m_bullets.begin(); it != m_bullets.end();) {
		if (!(*it)->isActive() || (*it)->isOutOfScreen()) {
			delete* it;
			it = m_bullets.erase(it);
		}
		else {
			++it;
		}
	}

	// 3. 清理跑出屏幕的推车
	for (int i = 0; i < 5; ++i) {
		if (m_lawnmowers[i] && m_lawnmowers[i]->isRemovable()) {
			delete m_lawnmowers[i];
			m_lawnmowers[i] = nullptr;	// 指针置空，代表这一行的推车已经被消耗掉了
		}
	}
}

bool Scene::getGridIndex(int x, int y, int& row, int& col)
{
	// 【D：根据实际背景测量并修改】 草坪区域的左上角起点坐标
	const int GRID_START_X = 250;
	const int GRID_START_Y = 80;

	// 【D：根据实际背景测量并修改】 每个格子的宽度和高度
	const int CELL_WIDTH = 80;
	const int CELL_HEIGHT = 100;

	// 检查鼠标是否在草坪区域内
	if (x<GRID_START_X || x>GRID_START_X + 9 * CELL_WIDTH ||
		y<GRID_START_Y || y>GRID_START_Y + 5 * CELL_HEIGHT) {
		return false;	//不在草坪上
	}

	// 计算对应的二维数组下标
	col = (x - GRID_START_X) / CELL_WIDTH;
	row = (y - GRID_START_Y) / CELL_HEIGHT;

	return true;
}
