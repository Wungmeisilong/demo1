/* *************** HeadFiles *************** */
#include <easyx.h>
#include <cmath>
#include <ctime>
#include <list>


/* *************** 全局变量 *************** */
long ret_useless = 0;				// 捕获 _getwch() 返回值以消除警告
const int GW = 640;					// 屏幕宽度
const int GH = 480;					// 屏幕高度
const double g = 9.8;				// 重力加速度
const double PI = 3.1415926;
const int len_max = 80;				// 光线线的最大长度
const int h_max = GH - len_max;		// 光线线可以达到的最大高度
const double v_max = sqrt(2 * g * h_max / 10.0);	// 光线线的最大初始速度
const int n_max = 5;				// 屏幕上最多烟花数量



/* *************** LightLine *************** */
class LightLine
{
public:
	LightLine(int, double);
	// 绘制光线的函数
	void Draw() const;
	// 移动光线的函数
	void Move();
	// 判断光线是否停止的函数
	bool Stopped() const { return v == 0; }
	// 判断光线是否超过光线的函数
	bool OverLine() const { return py < h_max * n_max / (n_max + 1); }	// Necessary condition for the next fireworks to rise
	// 获取光线的x坐标的函数
	int GetX() const { return px; }
	// 获取光线的y坐标的函数
	int GetY() const { return py; }
	//添加播放音乐的函数
	void PlayMusic() const { PlaySound(TEXT("light.wav"), NULL, SND_FILENAME | SND_ASYNC); }

private:
	int px;				// Position_x
	int py;				// Position_y
	int len;			// Length
	double v;			// Velocity (The -y axis is positiva)
	clock_t ct = 0;		// Recording time
};
LightLine::LightLine(int x = rand() % (GW - 80) + 40, double vv = (rand() % 20 + 76.0) / 100.0 * v_max) :px(x), py(h_max)
{
	v = vv;								// The initial velocity determines the height can be reached
	len = int(v / v_max * len_max);		// v : v_max = len : len_max
}
void LightLine::Draw() const
{
	for (int j = py; j < py + len; ++j)
	{
		float hsv_v = 0.8f * (len - (j - py)) / len + 0.2f;		// Gradient color
		setfillcolor(HSVtoRGB(0, 1.0f, hsv_v));
		solidcircle(px, j, 1);
	}
}
void LightLine::Move()
{
	if (v == 0)
		return;
	if (ct == 0)
	{
		ct = clock();
		Draw();
		return;
	}
	clock_t t = clock() - ct;
	ct = clock();
	double v_cur = v - g * t / 1000.0;		// The -y axis is positiva for the velocity
	if (v_cur > 0)
	{
		py += int(10 * (v_cur * v_cur - v * v) / 2 / g);
		v = v_cur;
	}
	else
	{
		py -= int(10 * v * v / 2 / g);
		v = 0;
	}
	len = int(v / v_max * len_max);
	Draw();
}


/* *************** ParticleSwarm *************** */
class ParticleSwarm
{
	struct Particle
	{
		int x;
		int y;
		int z = 0;		// Z axis vertical screen inword
		double vy;		// The y axis is positiva for the velocity
		Particle(int xx, int yy, double vv) :x(xx), y(yy), vy(vv) {}
	};
public:
	ParticleSwarm(int, int, float);
	void Draw() const;
	void Move();
	bool Finish() const { return vec.size() <= 1; }

private:
	double vx;
	double vz = 0;
	float hsv_h;					// Color parameter
	clock_t ct = 0;
	std::list<Particle> vec;		// For saving particles
};
ParticleSwarm::ParticleSwarm(int x, int y, float colorh = float(rand() % 256))
{
	// Cylindrical coordinate to xyz (parameters: len, radian_xz, radian_yx)
	hsv_h = colorh + rand() % 20;
	hsv_h = hsv_h > 255 ? hsv_h - 256 : hsv_h;
	double vm = v_max / 2 * (rand() % 5 + 15.0) / 20.0;
	double radian_xz = (rand() % 360) * PI / 180;
	double radian_yx = (rand() % 90) * PI / 180 + PI / 2;
	vx = vm * cos(radian_yx) * cos(radian_xz);
	vz = vm * cos(radian_yx) * sin(radian_xz);
	double vy = vm * sin(radian_yx);
	int len = rand() % 30 + 50;
	while (len)
	{
		// Use len as time parameter
		// You can change 200 to 50 to see the effect
		int xx = x + int(10 * vx * len / 200.0);
		int zz = int(10 * vz * len / 200.0);
		double cvy = vy - g * len / 200.0;
		int yy = y + int(10 * (cvy * cvy - vy * vy) / 2 / g);
		vec.push_back(Particle(xx, yy, cvy));
		--len;
	}
}
void ParticleSwarm::Draw() const
{
	int n = 0;
	auto size = vec.size();
	for (auto& x : vec)
	{
		if (x.x >= 0 && x.x < GW && x.y >= 0 && x.y < GH)
		{
			float cv = 0.2f + 0.8f * (size - n) / size - x.z / 40 * 0.1f;
			auto color = HSVtoRGB(hsv_h, 1.0f, cv > 0 ? cv : 0);
			if (x.z < 0)		// Z axis vertical screen inword
			{
				setfillcolor(color);
				solidcircle(x.x, x.y, abs(x.z) / 80 > 1 ? 2 : 1);
			}
			else
				putpixel(x.x, x.y, color);
		}
		++n;
	}
}
void ParticleSwarm::Move()
{
	if (ct == 0)
	{
		ct = clock();
		Draw();
		return;
	}
	for (int i = 0; i < 3 && vec.size() > 1; i++)
		vec.pop_back();		// Delete particles for shortening length
	clock_t t = clock() - ct;
	ct = clock();
	for (auto& x : vec)
	{
		double vy_cur = x.vy - g * t / 1000.0;
		x.x += int(10 * vx * t / 1000.0);
		x.y += int(10 * (vy_cur * vy_cur - x.vy * x.vy) / 2 / g);
		x.z += int(10 * vz * t / 1000.0);
		x.vy = vy_cur;
	}
	Draw();
}


/* *************** Fireworks *************** */
class Fireworks
{
public:
	Fireworks(int, int);
	void Move();
	bool Empty() const { return vec.empty(); }

private:
	std::list<ParticleSwarm> vec;
};
Fireworks::Fireworks(int x, int y)
{
	bool colorful = rand() % 100 < 20 ? true : false;
	float h = float(rand() % 256);
	int n = rand() % 5 + 45;
	for (int i = 0; i < n; i++)
	{
		if (colorful)
			vec.push_back(ParticleSwarm(x, y));
		else
			vec.push_back(ParticleSwarm(x, y, h));
	}
}
void Fireworks::Move()
{
	std::list<decltype(vec.begin())> toDel;
	for (auto it = vec.begin(); it != vec.end(); ++it)
	{
		if (it->Finish())
		{
			toDel.push_back(it);
			continue;
		}
		it->Move();
	}
	for (auto& x : toDel)
		vec.erase(x);
}


/* *************** main *************** */
int main()
{
	initgraph(GW, GH);
	setrop2(R2_MERGEPEN);
	srand((unsigned)time(nullptr));

	// Refresh once in 50ms
	clock_t ct = clock();
	// LightLine list
	std::list<LightLine> vec;
	vec.push_back(LightLine());
	// Fireworks list
	std::list<Fireworks> vec2;

	BeginBatchDraw();
	while (!(GetAsyncKeyState(VK_ESCAPE) & 0x8000))
	{
		if (clock() - ct > 50)
		{
			cleardevice();
			ct = clock();

			// LightLine list
			std::list<decltype(vec.begin())> toDel;
			if (vec.size() == 0)
				vec.push_back(LightLine());
			else if (vec.size() < n_max && rand() % 100 < 10 && (--vec.end())->OverLine())
				vec.push_back(LightLine());
			for (auto it = vec.begin(); it != vec.end(); ++it)
			{
				if (it->Stopped())
				{
					vec2.push_back(Fireworks(it->GetX(), it->GetY()));
					toDel.push_back(it);
					continue;
				}
				it->Move();
			}
			for (auto& it : toDel)
				vec.erase(it);
			// Fireworks list
			std::list<decltype(vec2.begin())> toDel2;
			for (auto it = vec2.begin(); it != vec2.end(); ++it)
			{
				if (it->Empty())
				{
					toDel2.push_back(it);
					continue;
				}
				it->Move();
			}
			for (auto& it : toDel2)
				vec2.erase(it);

			FlushBatchDraw();
		}
		Sleep(1);
	}
	EndBatchDraw();

	closegraph();
	return 0;
}