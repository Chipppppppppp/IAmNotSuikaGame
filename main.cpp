# include <Siv3D.hpp>
#include <emscripten.h>

EM_JS(
	char*, getItem, (const char* key),
	{
		return allocate(intArrayFromString(localStorage.getItem(UTF8ToString(key)) ?? ""), ALLOC_NORMAL);
	}
);
EM_JS(
	void, setItem, (const char* key, const char* value),
	{
		localStorage.setItem(UTF8ToString(key), UTF8ToString(value));
	}
);

const std::array<Color, 10> colors{ Palette::Red, Palette::Orange, Palette::Yellow, Palette::Deepskyblue, Palette::Blue, Palette::Greenyellow, Palette::Green, Palette::Pink, Palette::Deeppink, Palette::Purple };

double size(uint64 x) {
	return 20 * Pow(1.25, x);
}

struct RingEffect : IEffect
{
	Vec2 m_pos;

	ColorF m_color;

	double sz;

	explicit RingEffect(const Vec2& pos, uint64 idx)
		: m_pos{ pos }
		, m_color{ ColorF{ colors[idx % 10], 0.75 }}
		, sz{ size(idx) } {}

	bool update(double t) override
	{
		// イージング
		const double e = EaseOutExpo(t);

		Circle{ m_pos, (e * sz * 1.5) }.drawFrame((20.0 * (1.0 - e)), m_color);

		return (t < 1.0);
	}
};

void Main()
{

	Window::SetTitle(U"I Am Not Suika Game");

	// ウィンドウを 800*1200 にリサイズ
	Window::Resize(800, 1200);

	// 1位 ~ 3位のスコアを読み込む
	Array<int64> scores{ -1, -1, -1 };
	char* score0 = getItem("score0");
	if (*score0) {
		scores[0] = atoi(score0);
	}
	free(score0);
	char* score1 = getItem("score1");
	if (*score1) {
		scores[1] = atoi(score1);
	}
	free(score1);
	char* score2 = getItem("score2");
	if (*score2) {
		scores[2] = atoi(score2);
	}
	free(score2);

	// フォント
	const Font font{ 60 };
	const Font font_small{ 30 };
	const Font fontSDF{ FontMethod::SDF, 100, Typeface::Bold };

	// 2D 物理演算のシミュレーションステップ（秒）
	constexpr double stepSec = (1.0 / 200.0);

	// 2D 物理演算のシミュレーション蓄積時間（秒）
	double accumulatorSec = 0.0;

	// 重力加速度 (cm/s^2)
	constexpr double gravity = 980 * 0.75;

	// 2D 物理演算のワールド
	P2World world{ gravity };

	// [_] 地面
	const P2Body ground = world.createLine(P2Static, Vec2{ 0, 400 }, Line{ -200, 0, 200, 0 });
	const P2Body leftWall = world.createLine(P2Static, Vec2{ -200, 0 }, Line{ 0, 0, 0, 400 });
	const P2Body rightWall = world.createLine(P2Static, Vec2{ 200, 0 }, Line{ 0, 0, 0, 400 });

	// 物体
	HashTable<P2BodyID, std::pair<P2Body, uint64>> bodies;

	Effect effect;

	// 2D カメラ
	Camera2D camera{ Vec2{ 0, 0 }, 1.0, CameraControl::None_ };

	// 終了したか
	bool ended = false;
	bool canPut = true;
	P2BodyID prevId = (P2BodyID)-1;
	uint64 nextIdx = Random<uint64>(4);
	uint64 nextNextIdx = Random<uint64>(4);
	int64 score = 0;
	double cursorX = 0, cursorY = -100;

	auto drawAll = [&]() {
		{
			// 2D カメラから Transformer2D を作成
			const auto t = camera.createTransformer();

			// すべてのボディを描画
			for (const auto& [id, body] : bodies)
			{
				body.first.draw(colors[body.second % 10]);
			}

			// ネクストを描画
			if (canPut) {
				double sz = size(nextIdx);
				Circle{ cursorX, cursorY - sz, sz }.draw(colors[nextIdx]);
			}

			// 地面を描画
			ground.draw(Palette::Skyblue);
			leftWall.draw(Palette::Skyblue);
			rightWall.draw(Palette::Skyblue);

			effect.update();
		}

		Circle{ 700, 920, size(nextNextIdx) }.draw(colors[nextNextIdx]);
		font_small(U"Next").drawAt(Vec2{ 700, 1000 }, Palette::Skyblue);

		font(U"記録").draw(Vec2{ 40, 10 }, Palette::Skyblue);
		font_small(scores[0] == -1 ? U"1位: -" : U"1位: {}"_fmt(scores[0])).draw(Vec2{ 40, 100 }, Palette::Skyblue);
		font_small(scores[1] == -1 ? U"2位: -" : U"2位: {}"_fmt(scores[1])).draw(Vec2{ 40, 150 }, Palette::Skyblue);
		font_small(scores[2] == -1 ? U"3位: -" : U"3位: {}"_fmt(scores[2])).draw(Vec2{ 40, 200 }, Palette::Skyblue);

		// スコアを描画
		font(U"Score: {}"_fmt(score)).draw(Vec2{ 40, 1060 }, Palette::Skyblue);
	};

	while (System::Update())
	{
		drawAll();
		if (SimpleGUI::ButtonAt(U"リトライ", Vec2{ 700, 1120 }))
		{
			ended = false;
			canPut = true;
			prevId = (P2BodyID)-1;
			nextIdx = Random<uint64>(4);
			nextNextIdx = Random<uint64>(4);
			score = 0;
			cursorX = 0, cursorY = -100;
			bodies.clear();
		}

		if (ended)
		{
			if (SimpleGUI::ButtonAt(U"ツイート", Vec2{ 700, 1060 }))
			{
				System::LaunchBrowser(U"https://twitter.com/intent/tweet?text=I%20Am%20Not%20Suika%20Game%20%E3%81%A7%20{}%20%E7%82%B9%E3%82%92%E7%8D%B2%E5%BE%97%E3%81%97%E3%81%BE%E3%81%97%E3%81%9F%EF%BC%81%0Ahttps%3A%2F%2Fchipppppppppp.github.io%2FIAmNotSuikaGame%2F%0A%23IAmNotSuikaGame%20%23%E7%AD%91%E9%A7%92%E6%96%87%E5%8C%96%E7%A5%AD2023"_fmt(score));
			}
			continue;
		}

		for (accumulatorSec += Scene::DeltaTime(); stepSec <= accumulatorSec; accumulatorSec -= stepSec)
		{
			// 2D 物理演算のワールドを更新
			world.update(stepSec);

			auto collisions = world.getCollisions();

			if (!canPut)
			{
				for (const auto& [pair, collision] : collisions)
				{
					if (pair.a == prevId || pair.b == prevId)
					{
						canPut = true;
						break;
					}
				}
				if (canPut)
				{
					nextIdx = nextNextIdx;
					nextNextIdx = Random<uint64>(4);
					double szNxt = size(nextIdx);
					cursorX = Min(199 - szNxt, Max(-199 + szNxt, cursorX + Random<double>(-1, 1)));
				}
			}

			for (const auto& [pair, collision] : collisions)
			{
				auto ia = bodies.find(pair.a), ib = bodies.find(pair.b);
				if (ia != bodies.end() && ib != bodies.end() && ia->second.second == ib->second.second)
				{
					uint64 idx = ia->second.second + 1;
					score += Pow(2.0, idx - 1);

					Vec2 a = ia->second.first.getPos(), b = ib->second.first.getPos();
					double x = (a.x + b.x) / 2;
					double y = (a.y + b.y) / 2;

					effect.add<RingEffect>(a, idx - 1);
					effect.add<RingEffect>(b, idx - 1);

					double sz = size(idx);

					P2Body body = world.createCircle(P2Dynamic, Vec2{ x, y }, Circle{ sz });

					bodies.emplace(body.id(), std::pair{ body, idx });
					bodies.erase(pair.a);
					bodies.erase(pair.b);
				}
			}
		}

		bool flag = true;
		if (canPut) {
			for (const auto& [id, body] : bodies) if (body.first.getPos().y >= 500)
			{
				flag = false;
				break;
			}
		}

		if (!flag)
		{
			scores.push_back(score);
			scores.rsort();
			scores.pop_back();
			setItem("score0", std::to_string(scores[0]).c_str());
			setItem("score1", std::to_string(scores[1]).c_str());
			setItem("score2", std::to_string(scores[2]).c_str());
			ended = true;
			continue;
		}

		if (canPut) {
			// 2D カメラから Transformer2D を作成
			const auto t = camera.createTransformer();

			auto [x, y] = Cursor::PosF();

			if ((KeyLeft.pressed() || MouseL.pressed() && (cursorY + 100 <= y && y <= 400 && x < -200 || y < cursorY + 100 && x < cursorX)) && cursorX - 4 > -200 + size(nextIdx))
			{
				cursorX -= 4;
			}
			if ((KeyRight.pressed() || MouseL.pressed() && (cursorY + 100 <= y && y <= 400 && 200 < x || y < cursorY + 100 && cursorX < x)) && cursorX + 4 < 200 - size(nextIdx))
			{
				cursorX += 4;
			}

			if (canPut && (KeyEnter.pressed() || MouseL.pressed() && -200 <= Cursor::PosF().x && Cursor::PosF().x <= 200 && cursorY + 100 <= Cursor::PosF().y && Cursor::PosF().y <= 400))
			{
				double sz = size(nextIdx);

				P2Body body = world.createCircle(P2Dynamic, Vec2{ cursorX, cursorY - sz }, Circle{ sz });

				bodies.emplace(body.id(), std::pair{ body, nextIdx });

				prevId = body.id();
				canPut = false;
			}
		}

		double minY = 1e10;
		for (auto [id, body] : bodies) minY = Min(minY, body.first.getPos().y - size(body.second));
		cursorY = Min(minY - 100, -100.0);
	}
}
