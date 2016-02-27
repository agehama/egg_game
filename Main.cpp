#include <Siv3D.hpp>
#include "Box2DManager.h"

extern bool isGameOver;

void drawPolygon(b2Body* pBody)
{
	const b2Vec2 pos = pBody->GetPosition();
	const Vec2 drawpos = toVec2(pos);
	const b2Transform& xf = pBody->GetTransform();

	for (b2Fixture* pFixture = pBody->GetFixtureList(); pFixture != NULL; pFixture = pFixture->GetNext())
	{
		b2Shape* shape = pFixture->GetShape();
		b2PolygonShape* s = dynamic_cast<b2PolygonShape*>(shape);

		std::vector<Vec2> points;
		for (int32 index = 0; index < s->GetVertexCount(); ++index)
		{
			points.push_back(toVec2(b2Mul(xf, s->GetVertex(index))));
		}
		Polygon(points.data(), points.size()).draw(Palette::Gray);
	}
}

void Main()
{
	Window::Resize(1280, 720);

	FontAsset::Register(L"DebugDraw", 16);
	
	Box2DManager manager(10.0f);

	const Point WindowSize = Window::Size();

	
	b2Body* pGoal;
	const float cupWidth = 200;
	const Vec2 goalCenter(Window::Center().x, 500);
	{
		std::vector<Vec2> poly1;
		std::vector<Vec2> poly2;
		std::vector<Vec2> poly3;

		const float cupDepth = 300;
		const float cupThickness = 30;
		const float rimSharpness = 50;

		poly1.push_back(goalCenter + Vec2(-cupWidth*0.5, 0));
		poly1.push_back(goalCenter + Vec2(-cupWidth*0.5 + cupThickness, rimSharpness));
		poly1.push_back(goalCenter + Vec2(-cupWidth*0.5 + cupThickness, cupDepth));
		poly1.push_back(goalCenter + Vec2(-cupWidth*0.5, cupDepth));

		poly2.push_back(goalCenter + Vec2(-cupWidth*0.5, cupDepth - cupThickness));
		poly2.push_back(goalCenter + Vec2(cupWidth*0.5, cupDepth - cupThickness));
		poly2.push_back(goalCenter + Vec2(cupWidth*0.5, cupDepth));
		poly2.push_back(goalCenter + Vec2(-cupWidth*0.5, cupDepth));

		poly3.push_back(goalCenter + Vec2(cupWidth*0.5 - cupThickness, rimSharpness));
		poly3.push_back(goalCenter + Vec2(cupWidth*0.5, 0));
		poly3.push_back(goalCenter + Vec2(cupWidth*0.5, cupDepth));
		poly3.push_back(goalCenter + Vec2(cupWidth*0.5 - cupThickness, cupDepth));

		pGoal = manager.add(Polygon(poly1), goalCenter, false, PhysicalMaterial(0.5, 0.0, 0.5));
		manager.addToBody(pGoal, Polygon(poly2), goalCenter, false, PhysicalMaterial(0.5, 0.0, 0.5));
		manager.addToBody(pGoal, Polygon(poly3), goalCenter, false, PhysicalMaterial(0.5, 0.0, 0.5));
	}
	
	const Vec2 center(300, 300);
	const float particleRadius = 6.f;
	const float softBodyRadius = 55.f;
	int particleSize = 20;
	const float unitAngle = TwoPi / particleSize;
	const float distance = (Vec2(cos(unitAngle), sin(unitAngle))*softBodyRadius - Vec2(cos(unitAngle * 2), sin(unitAngle * 2))*softBodyRadius).length();

	struct EggParticles
	{
		b2Body* pCenter;
		std::vector<b2Body*> eggYolk;
		std::vector<b2Body*> eggWhite;
	};
	std::vector<EggParticles> eggs;

	const auto makeEgg = [&manager, &eggs](const Vec2& center)
	{
		const float particleRadius = 6.f;
		const float softBodyRadius = 55.f;
		int particleSize = 20;
		const float unitAngle = TwoPi / particleSize;
		const float distance = (Vec2(cos(unitAngle), sin(unitAngle))*softBodyRadius - Vec2(cos(unitAngle * 2), sin(unitAngle * 2))*softBodyRadius).length();

		EggParticles eggData;

		eggData.pCenter = manager.add(Circle(center, particleRadius), true, PhysicalMaterial());
		eggData.eggYolk.push_back(eggData.pCenter);
		for (int i = 0; i < particleSize; ++i)
		{
			const float angle = unitAngle * i;
			const Vec2 currentCenter = center + Vec2(cos(angle), sin(angle))*softBodyRadius;
			b2Body* pCurrent = manager.add(Circle(currentCenter, particleRadius), true, PhysicalMaterial());
			eggData.eggYolk.push_back(pCurrent);

			double freq = 10.0;
			manager.addDistanceJoint(eggData.pCenter, eggData.eggYolk.back(), freq, softBodyRadius / PixelPerMeter, true);

			if (i != 0)
			{
				manager.addDistanceJoint(*(eggData.eggYolk.end() - 2), *(eggData.eggYolk.end() - 1), freq, distance / PixelPerMeter, true);
			}
			if (i + 1 == particleSize)
			{
				manager.addDistanceJoint(eggData.eggYolk.back(), eggData.eggYolk.front(), freq, distance / PixelPerMeter, true);
			}
		}

		/*
		卵白
		*/
		const int eggWhiteNum = 100;
		for (int i = 0; i < 100; ++i)
		{
			const float angle = Random()*TwoPi;
			const float currentPosFromCenter = Random(softBodyRadius, softBodyRadius * 3);
			const Vec2 currentCenter = center + Vec2(cos(angle), sin(angle))*currentPosFromCenter;
			b2Body* pCurrent = manager.add(Circle(currentCenter, particleRadius * 2), true, PhysicalMaterial(0.1, 0.5, 0.2));
			eggData.eggWhite.push_back(pCurrent);

			double freq = 0.5;
			manager.addDistanceJoint(pCurrent, eggData.pCenter, freq, softBodyRadius / PixelPerMeter, false);
		}

		eggs.push_back(eggData);
	};

	Optional<Vec2> clicking;
	Line currentGravity;

	int count = 0;
	bool requested = false;
	
	Texture metaball(L"ball.png");

	const float textureScale = 0.5;
	RenderTexture particlesTexture((Window::Size() * textureScale).asPoint(), TextureFormat::R8G8_Unorm);

	Stopwatch stopWatch;
	Stopwatch stopWatchSec;
	
	std::vector<Vec2> poly1;
	std::vector<Vec2> poly2;
	std::vector<Vec2> poly3;
	b2Body* pPlayer = nullptr;
	b2Body* pMouse = nullptr;
	{
		const float cupWidth = 200;
		const float cupDepth = 300;
		const float cupThickness = 30;
		const float rimSharpness = 0;
		const Vec2 cupPos(500, 400);
		poly1.push_back(cupPos + Vec2(-cupWidth*0.5, 0));
		poly1.push_back(cupPos + Vec2(-cupWidth*0.5 + cupThickness, rimSharpness));
		poly1.push_back(cupPos + Vec2(-cupWidth*0.5 + cupThickness, cupDepth));
		poly1.push_back(cupPos + Vec2(-cupWidth*0.5, cupDepth));

		poly2.push_back(cupPos + Vec2(-cupWidth*0.5, cupDepth - cupThickness));
		poly2.push_back(cupPos + Vec2(cupWidth*0.5, cupDepth - cupThickness));
		poly2.push_back(cupPos + Vec2(cupWidth*0.5, cupDepth));
		poly2.push_back(cupPos + Vec2(-cupWidth*0.5, cupDepth));

		poly3.push_back(cupPos + Vec2(cupWidth*0.5 - cupThickness, rimSharpness));
		poly3.push_back(cupPos + Vec2(cupWidth*0.5, 0));
		poly3.push_back(cupPos + Vec2(cupWidth*0.5, cupDepth));
		poly3.push_back(cupPos + Vec2(cupWidth*0.5- cupThickness, cupDepth));

		pPlayer = manager.add(Polygon(poly2), cupPos, true, PhysicalMaterial(0.5, 0.0, 0.0));
		manager.addToBody(pPlayer, Polygon(poly1), cupPos, true, PhysicalMaterial(0.5, 0.0, 0.0));
		manager.addToBody(pPlayer, Polygon(poly3), cupPos, true, PhysicalMaterial(0.5, 0.0, 0.0));

		pMouse = manager.add(Circle(cupPos + Vec2(0, -200), 1.0), false, PhysicalMaterial(0, 0.0, 0.0));
		manager.addDistanceJoint(pPlayer, pMouse, tob2Vec2(Vec2(-cupWidth*0.5, -cupDepth*0.7)), tob2Vec2(Vec2(0, 0)), 50.0, 200.0 / PixelPerMeter);
		manager.addDistanceJoint(pPlayer, pMouse, tob2Vec2(Vec2(+cupWidth*0.5, -cupDepth*0.7)), tob2Vec2(Vec2(0, 0)), 50.0, 200.0 / PixelPerMeter);

	}

	const auto isGoal = [&cupWidth, &goalCenter](const Vec2& position) 
	{
		return abs(position.x - goalCenter.x) < cupWidth*0.5;
	};

	Graphics::SetBackground(Palette::Brown);
	stopWatch.start();

	Font font(90,Typeface::Black);
	Font font2(120, Typeface::Black);
	Font font3(32, Typeface::Black);
	int point = 0;
	PixelShader ps(L"eggs.hlsl");

	while (System::Update())
	{
		if (System::FrameCount() % 120 == 0)
		{
			const float xPos = Random(Window::Width()*0.2, Window::Width()*0.8);
			makeEgg(Vec2(xPos, -200.0));
			manager.update();
		}
		
		if (!eggs.empty())
		{
			int currentFrameEgg = System::FrameCount() % eggs.size();
			EggParticles& egg = eggs[currentFrameEgg];
			
			if (!egg.eggWhite.empty())
			{
				for (auto it = egg.eggWhite.end() - 1; ; --it)
				{
					const auto pos = toVec2((*it)->GetPosition());
					if (Window::Height() < pos.y)
					{
						if (isGoal(pos))
						{
							point -= 10;
						}
						manager.destroyBody(*it);
						it = egg.eggWhite.erase(it);
					}

					if (egg.eggWhite.empty() || it == egg.eggWhite.begin())
					{
						break;
					}
				}
			}
			
			if (!egg.eggYolk.empty())
			{
				for (auto it = egg.eggYolk.end() - 1; ; --it)
				{
					const auto pos = toVec2((*it)->GetPosition());
					if (Window::Height() < pos.y)
					{
						if (isGoal(pos))
						{
							point += 100;
						}
						manager.destroyBody(*it);
						it = egg.eggYolk.erase(it);
					}

					if (egg.eggYolk.empty() || it == egg.eggYolk.begin())
					{
						break;
					}
				}
			}

			if (egg.eggWhite.empty() && egg.eggYolk.empty())
			{
				eggs.erase(eggs.begin() + currentFrameEgg);
			}
		}


		int v = (stopWatch.s() % 12) / 3;
		Vec2 g(0, 1000.0);

		manager.setGravity(Vec2(0, 1000.0));
		currentGravity = Line(Window::Center(), Window::Center() + g * 0.1 );
		double rate = 1.0 - 1.0*(stopWatch.ms() % 1000) / 1000;
		currentGravity.drawArrow(10.0, Vec2(20.0, 20.0), Color(Palette::Orange).setAlpha(256 * rate));

		manager.update();

		SamplerState state;
		state.addressU = TextureAddressMode::Clamp;
		state.addressV = TextureAddressMode::Clamp;
		state.addressW = TextureAddressMode::Clamp;
		Graphics2D::SetSamplerState(state);

		particlesTexture.clear(ColorF(0.0));
		Graphics2D::SetTransform(Mat3x2::Identity().scale(textureScale));
		Graphics2D::SetRenderTarget(particlesTexture);
		Graphics2D::SetBlendState(BlendState::Additive);

		float minJointForceSq = 100000.0;
		float maxJointForceSq = 0.0;
		
		for(const auto& egg : eggs)
		{
			for(auto pBody : egg.eggWhite)
			{
				const float invDt = 1.0 / manager.getConfig().timeStep;
				for (b2JointEdge* pJoint = pBody->GetJointList(); pJoint != NULL; pJoint = pJoint->next)
				{
					const b2Vec2 v = pJoint->joint->GetReactionForce(invDt);
					const float forceSq = v.LengthSquared();
					if (forceSq < minJointForceSq)
					{
						minJointForceSq = forceSq;
					}

					if (maxJointForceSq < forceSq)
					{
						maxJointForceSq = forceSq;
					}

					if (15.0 < forceSq)
					{
						manager.destroyJoint(pJoint->joint);
					}
				}

				const b2Vec2 pos = pBody->GetPosition();
				const Vec2 drawpos = toVec2(pos);
				const b2Transform& xf = pBody->GetTransform();

				for (b2Fixture* pFixture = pBody->GetFixtureList(); pFixture != NULL; pFixture = pFixture->GetNext())
				{
					b2Shape* shape = pFixture->GetShape();
					b2CircleShape* s = dynamic_cast<b2CircleShape*>(shape);

					metaball.scale(0.5).drawAt(drawpos + toVec2(s->m_p), Color(255, 0, 0));
				}
			}

			for (auto pBody : egg.eggYolk)
			{
				const b2Vec2 pos = pBody->GetPosition();
				const Vec2 drawpos = toVec2(pos);
				const b2Transform& xf = pBody->GetTransform();

				for (b2Fixture* pFixture = pBody->GetFixtureList(); pFixture != NULL; pFixture = pFixture->GetNext())
				{
					b2Shape* shape = pFixture->GetShape();
					b2CircleShape* s = dynamic_cast<b2CircleShape*>(shape);
					metaball.scale(0.5).drawAt(drawpos + toVec2(s->m_p), Color(0, 255, 0));
				}
			}
		}

		Graphics2D::SetBlendState(BlendState::Default);

		Graphics2D::SetTransform(Mat3x2::Identity().scale(1.0));
		Graphics2D::SetRenderTarget(Graphics::GetSwapChainTexture());

		Window::ClientRect().draw({ Color(0,255,255),Color(0,255,255),Color(255,255,255) ,Color(255,255,255) });
		font.drawCenter(L"落ちてくる卵の黄身を\n容器に入れる仕事", 100, Palette::Red);
		font3.draw(L"給料：", { 0,470 }, Palette::Red);
		font2.draw(Format(point, L"円"), { 0,500 }, Palette::Red);
		Graphics2D::BeginPS(ps);
		
		particlesTexture.scale(1.0 / textureScale).draw();

		Graphics2D::EndPS();
		
		pMouse->SetTransform(tob2Vec2(Mouse::Pos()), 0.0);
		pPlayer->SetAngularVelocity(-pPlayer->GetAngle()*10.0);
		for (b2JointEdge* pJoint = pPlayer->GetJointList(); pJoint != NULL; pJoint = pJoint->next)
		{
			Line(toVec2(pJoint->joint->GetAnchorA()), toVec2(pJoint->joint->GetAnchorB())).draw(Palette::Gray);
		}
		drawPolygon(pPlayer);
		drawPolygon(pGoal);
	}
}