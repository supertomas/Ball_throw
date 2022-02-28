#include<Siv3D.hpp>

#include"Load_Function.h"

const FilePath tomlPath = FileSystem::FullPath(U"example/config/config.toml");

struct EmojiData
{
	String name;
	Emoji emoji;
};

Array<EmojiData> LoadItems(const FilePath& tomlPath)
{
	Array<EmojiData> items;
	const TOMLReader toml(tomlPath);
	if (!toml)
	{
		return items;
	}
	size_t i = 0;

	for (const auto& object : toml[U"Items"].tableArrayView())
	{
		EmojiData item;
		item.name = U"Enemy{}"_fmt(i);
		item.emoji = Emoji(object[U"path"].getString());
		items << item;
		++i;
	}
	return items;
}

struct NumberEffect : IEffect
{
	Vec2 m_start;

	int32 m_number;

	Font m_font;

	int32 m_count;

	NumberEffect(Vec2 start, const Font& font, int32 count)
		: m_start(start)
		, m_font(font)
		, m_count(count)
	{}

	bool update(double t) override
	{
		const HSV color(180 - m_number * 1.8, 1.0 - (t * 2.0));

		//m_font(m_number).drawAt(m_start.movedBy(0, t * -120), color);
		if (m_count == 1)
		{
			m_font(U"Great !!").drawAt(m_start.movedBy(0, t * -120), color);
		}
		else
		{
			m_font(m_count, U"コンボ").drawAt(m_start.movedBy(0, t * -120), color);
		}
		// 0.5 秒以上経過で消滅
		return t < 0.5;
	}
};

class Enemy
{
public:
	Point pos, size;
	int32 posx = 0;
	int32 posy = 0;
	int32 RandomNumber;
	int32 direction;
	Enemy(Point _pos, Point _size, int32 Randomnumber, int32 _direction) :
		pos(_pos),
		size(_size),
		RandomNumber(Randomnumber),
		direction(_direction)
	{

	}
	int32 getX(int32 direction)
	{
		if (direction == 0)
		{
			return pos.x;
		}
	}
	void update()
	{

		if (direction == 1)
		{
			if (pos.x > -100)
			{
				pos.x -= 5;
			}
		}
		else if (direction == 0)
		{
			if (pos.x < Window::ClientWidth() + 100)
			{
				pos.x += 5;
			}
		}
	}

	Rect getBody()
	{
		return Rect(pos.x - size.x / 2, pos.y - size.y + 20, size.x, size.y);
	}

	void draw() const
	{
		const String name = U"Enemy{}"_fmt(RandomNumber);
		TextureAsset(name).drawAt(pos.x, pos.y);
	}
private:

};

class Ball
{
private:

	Vec2 pos;

	int32 r;

	int32 error;

	double g;

	String name;

	bool grab;

	bool active;

	int32 pre_ground;

	Stopwatch timer;

	int32 combo;

public:
	double v_x, v_y;

	explicit Ball(const FilePath& toml)
		: pos(GetPos(toml))
		, r(GetRadius(toml))
		, error(GetError(toml))
		, g(GetGravity(toml))
		, name(U"ball")
		, grab(false)
		, active(false)
		, pre_ground(pos.y)
		, v_x(0.0)
		, v_y(0.0)
		, combo(0) {}

	bool isActive() const
	{
		return active;
	}

	void addCombo()
	{
		++combo;
	}

	int32 getCombo() const
	{
		return combo;
	}

	const Vec2& getPos() const
	{
		return pos;
	}

	Circle getRegion() const
	{
		return Circle(pos.x, pos.y, error);
	}

	Circle getBody() const
	{
		return Circle(pos.x, pos.y, r);
	}

	void update()
	{

		if (getBody().leftClicked())
		{
			grab = true;
		}

		//速度の初期値を設定
		else if (grab && MouseL.up()) // マウスの左ボタンが離されたら
		{
			active = true;
			grab = false;
			v_x = (pos.x - Cursor::Pos().x) / 8.0;
			v_y = ((pos.y - Cursor::Pos().y) / 8.0);
		}

		//斜方投射実装
		if (active && pos.y <= pre_ground)
		{
			v_y += g;
			pos.y += v_y;
			pos.x += v_x;
		}
		else if (pos.y > pre_ground + 1)
		{
			active = false;
			timer.restart();
			combo = 0;
			pos.y = pre_ground;
		}
		else
		{
			active = false;
		}

		if (timer.ms() > 500)
		{
			pos.x = 200;
			timer.reset();
		}
		
		if (grab)
		{
			Line(Cursor::Pos().x, Cursor::Pos().y, pos.x + (pos.x - Cursor::Pos().x), pos.y + (pos.y - Cursor::Pos().y)).drawArrow(10, Vec2(20, 40), Palette::Orange);
		}
	}

	void draw() const
	{
		if (getRegion().mouseOver())
		{
			getBody()(TextureAsset(name)).draw(Palette::Skyblue);
		}
		else
		{
			getBody()(TextureAsset(name)).draw();
		}
	}
};

void ReloadSetting(const DirectoryWatcher& watcher, Ball& ball)
{
	for (const auto& change : watcher.retrieveChanges())
	{
		if ((change.first == tomlPath)
			&& (change.second == FileAction::Modified)) // TOML ファイルが更新されたら
		{
			Print << U"reload";
			ball = Ball(tomlPath);
		}
	}
}

void Main()
{
	// 背景を水色にする
	Scene::SetBackground(ColorF(0.8, 0.9, 1.0));
	Window::SetStyle(WindowStyle::Sizable);

	const Font font(30);
	const Font effectFont(40, Typeface::Heavy);
	TextureAsset::Register(U"ball", U"Images/ボール.png");
	for (const auto& item : LoadItems(tomlPath))
	{
		TextureAsset::Register(item.name, item.emoji);
	}

	DirectoryWatcher watcher(FileSystem::ParentPath(tomlPath));
	Array<Enemy> enemies;

	Ball ball(tomlPath);

	Stopwatch stopwatch(true);

	int32 score = 0;

	Effect effect;

	while (System::Update())
	{
		//
		// update
		//

		ball.update();

		if (stopwatch > 0.8s)
		{
			enemies.push_back(Enemy({ 400 ,Random(0,350) }, { 90, 60 }, Random(0, 14), Random(0, 1)));
			stopwatch.restart();
		}

		for (auto& enemy : enemies)
		{
			enemy.update();
		}

		for (auto it = enemies.begin(); it != enemies.end();)
		{
			if (ball.isActive() && it->getBody().intersects(ball.getBody()))
			{
				score += (100 + ball.getCombo() * 20);
				ball.addCombo();
				const Vec2 effectpos = ball.getPos().movedBy(0, 100);
				effect.add<NumberEffect>(effectpos, effectFont, ball.getCombo());
				it = enemies.erase(it);
			}
			else
			{
				++it;
			}
		}

		// enemyのサイズが10以上になったら始めのenemyを消す
		enemies.removed_if([](const Enemy& e)
			{
				return e.pos.x < -100;
			});

		ReloadSetting(watcher, ball);

		//
		// draw
		//
		for (const auto& enemy : enemies)
		{
			enemy.draw();
		}

		ball.draw();

		effect.update();

		// スコアの表示
		font(U"スコア : ", score, U"🐣").drawAt({ 670,50 }, Palette::Black);
	}
}