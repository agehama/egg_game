#pragma once
#include <Siv3D.hpp>
#include "Debugdraw.h"

#include "GlobalConstant.h"

#ifdef _DEBUG
#pragma comment(lib,"x64/Box2D/Box2D_d.lib")
#else
#pragma comment(lib,"x64/Box2D/Box2D.lib")
#endif

struct Box2DConfig
{
	float32 timeStep;	//Box2Dが計算するタイムステップを設定
	int32 velocityIterations;	//速度に関して計算処理を反復する回数
	int32 positionIterations;	//位置に関して計算処理を反復する回数
	Box2DConfig(){ timeStep=0; velocityIterations=0; positionIterations=0;}
	Box2DConfig(float32 t, int32 v, int32 p)
	{
		timeStep = t;
		velocityIterations = v;
		positionIterations = p;
	}
};

struct PhysicalMaterial
{
	PhysicalMaterial(float density = 0.5f, float friction = 0.5f, float restitution = 0.5f)
		:m_density(density),m_friction(friction),m_restitution(restitution){}
	float m_density;
	float m_friction;
	float m_restitution;
};

bool isGameOver = false;

class Box2DManager
{
public:
	Box2DManager(const Texture& tex,float g=10.0f)
		:m_texture(tex),m_gravity(0.0f,g),m_world(m_gravity),m_config(1.0f/60.0f,10,10),m_count(0)
	{
		m_world.SetAllowSleeping(false);
		b2dDraw* debugDraw = new b2dDraw();
		uint32 flags = 0;
		flags += b2Draw::e_shapeBit;
		flags += b2Draw::e_jointBit;
		debugDraw->SetFlags(flags);
		m_world.SetDebugDraw(debugDraw);
	}
	Box2DManager(float g = 10.0f)
		:m_gravity(0.0f, g), m_world(m_gravity), m_config(1.0f / 60.0f, 10, 10), m_count(0)
	{
		m_world.SetAllowSleeping(false);
		b2dDraw* debugDraw = new b2dDraw();
		uint32 flags = 0;
		flags += b2Draw::e_shapeBit;
		flags += b2Draw::e_jointBit;
		debugDraw->SetFlags(flags);
		m_world.SetDebugDraw(debugDraw);
	}
	virtual ~Box2DManager(){}
	void init(){}
	void setGravity(const Vec2& v)
	{
		m_world.SetGravity(tob2Vec2(v));
	}
	void update()
	{
		m_world.Step(m_config.timeStep,m_config.velocityIterations,m_config.positionIterations);
		m_world.ClearForces();
	}
	void debugDraw()
	{
		m_world.DrawDebugData();
	}
	b2Body* add(const RectF& rect,bool isDynamic=true,const PhysicalMaterial& material=PhysicalMaterial(),int angle=0)
	{
		const b2Vec2 center = tob2Vec2(rect.center);
		const b2Vec2 size = tob2Vec2(rect.size);
		const b2BodyType type = isDynamic ? b2BodyType::b2_dynamicBody : b2BodyType::b2_staticBody;

		b2PolygonShape rectshape;
		rectshape.SetAsBox(size.x*0.5f,size.y*0.5f);

		return push(center,static_cast<float32>(Radians(1.0*angle)),rectshape,type,material);
	}
	b2Body* add(const Triangle& tri,bool isDynamic=true,const PhysicalMaterial& material=PhysicalMaterial(),int angle=0)
	{
		const b2Vec2 center=tob2Vec2((tri.p0+tri.p1+tri.p2)/3.0);
		const b2BodyType type = isDynamic ? b2BodyType::b2_dynamicBody : b2BodyType::b2_staticBody;

		b2PolygonShape trishape;
		b2Vec2 vts[3] = {tob2Vec2(tri.p0)-center,tob2Vec2(tri.p1)-center,tob2Vec2(tri.p2)-center};
		trishape.Set(vts,3);

		return push(center,static_cast<float32>(Radians(1.0*angle)),trishape,type,material);
	}
	b2Body* add(const Polygon& poly,const Vec2& center,bool isDynamic=true,const PhysicalMaterial& material=PhysicalMaterial(),int angle=0)
	{
		const b2Vec2 b2center = tob2Vec2(center);
		const b2BodyType type = isDynamic ? b2BodyType::b2_dynamicBody : b2BodyType::b2_staticBody;
		
		b2PolygonShape polyshape;
		b2Vec2* vts = new b2Vec2[poly.num_vertices];
		for (size_t i = 0; i<poly.num_vertices; ++i)
		{
			vts[i] = tob2Vec2(poly.vertex(i)) - b2center;
		}
		polyshape.Set(vts, poly.num_vertices);

		delete [] vts;
		return push(b2center,static_cast<float32>(Radians(1.0*angle)),polyshape,type,material);
	}
	b2Body* add(const Circle& circle,bool isDynamic=true,const PhysicalMaterial& material=PhysicalMaterial())
	{
		const b2Vec2 center = tob2Vec2(circle.center);
		const float32 radius = static_cast<float32>(circle.r/PixelPerMeter);
		const b2BodyType type = isDynamic ? b2BodyType::b2_dynamicBody : b2BodyType::b2_staticBody;
		
		b2CircleShape circleshape;
		circleshape.m_radius = radius;

		return push(center,0.f,circleshape,type,material);
	}
	void addToBody(b2Body* pBody, const Polygon& poly, const Vec2& center, bool isDynamic = true, const PhysicalMaterial& material = PhysicalMaterial(), int angle = 0)
	{
		const b2Vec2 b2center = tob2Vec2(center);
		const b2BodyType type = isDynamic ? b2BodyType::b2_dynamicBody : b2BodyType::b2_staticBody;

		b2PolygonShape polyshape;
		b2Vec2* vts = new b2Vec2[poly.num_vertices];
		for (size_t i = 0; i<poly.num_vertices; ++i)
		{
			vts[i] = tob2Vec2(poly.vertex(i)) - b2center;
		}
		polyshape.Set(vts, poly.num_vertices);

		delete[] vts;

		pBody->CreateFixture(&polyshape, material.m_density);
	}
	void addDistanceJoint(b2Body* pBody1, b2Body* pBody2, double Hz, double length, bool collideConnected = false)
	{
		b2DistanceJointDef jointdef;
		jointdef.Initialize(pBody1, pBody2, pBody1->GetWorldCenter(), pBody2->GetWorldCenter());
		jointdef.collideConnected = collideConnected;
		jointdef.frequencyHz = static_cast<float32>(Hz);
		jointdef.dampingRatio = 0.0;
		jointdef.length = length;
		m_world.CreateJoint(&jointdef);
	}
	void addDistanceJoint(b2Body* pBody1, b2Body* pBody2,const b2Vec2& pos1, const b2Vec2& pos2, double Hz, double length, bool collideConnected = false)
	{
		b2DistanceJointDef jointdef;
		jointdef.Initialize(pBody1, pBody2, pBody1->GetWorldCenter()+pos1, pBody2->GetWorldCenter()+pos2);
		jointdef.collideConnected = collideConnected;
		jointdef.frequencyHz = static_cast<float32>(Hz);
		jointdef.dampingRatio = 0.0;
		jointdef.length = length;
		m_world.CreateJoint(&jointdef);
	}
	void addDistanceJoint(const b2DistanceJointDef& def)
	{
		m_world.CreateJoint(&def);
	}
	void addVelocityBy(b2Body* pBpdy, const Vec2& v)
	{
		b2Vec2 d = tob2Vec2(v);
		b2Vec2 mov = d + pBpdy->GetLinearVelocity();
		mov.x = Clamp(mov.x,-1.f*MaxVelocity,1.f*MaxVelocity);
		pBpdy->SetLinearVelocity(mov);
	}
	void forceBy(b2Body* pBpdy, const Vec2& v)
	{
		const b2Vec2 mov = tob2Vec2(v);
		pBpdy->ApplyForce(mov, pBpdy->GetPosition(), true);
	}
	void addVec2(b2Body* pBpdy, const Vec2& v)
	{
		const b2Vec2 mov = tob2Vec2(v);
		pBpdy->SetTransform(pBpdy->GetPosition()+mov,pBpdy->GetAngle());
	}
	void setVec2(b2Body* pBpdy, const Vec2& v)
	{
		const b2Vec2 mov = tob2Vec2(v);
		pBpdy->SetTransform(mov,pBpdy->GetAngle());
	}
	void addAngle(b2Body* pBpdy, float angle)
	{
		pBpdy->SetTransform(pBpdy->GetPosition(),pBpdy->GetAngle()+Radians(angle));
	}
	void setAngle(b2Body* pBpdy, float angle)
	{
		pBpdy->SetTransform(pBpdy->GetPosition(),Radians(angle));
	}
	Vec2 getPosition(b2Body* pBpdy)
	{
		const b2Vec2& pos = pBpdy->GetPosition();
		return Vec2(pos.x*PixelPerMeter,pos.y*PixelPerMeter);
	}

	void destroyJoint(b2Joint* joint)
	{
		m_world.DestroyJoint(joint);
	}

	const Box2DConfig& getConfig()const
	{
		return m_config;
	}

	void destroyBody(b2Body* pBody)
	{
		m_world.DestroyBody(pBody);
	}

protected:

	b2Body* push(const b2Vec2& pos,float32 rad,const b2Shape& shape,b2BodyType type,const PhysicalMaterial& material)
	{
		b2BodyDef bodydef;
		bodydef.type = type;
		bodydef.position.Set(pos.x,pos.y);
		bodydef.angle = rad;
		b2Body* pb = m_world.CreateBody(&bodydef);

		b2FixtureDef fixturedef;
		fixturedef.shape = &shape;
		fixturedef.density = material.m_density;
		fixturedef.friction = material.m_friction;
		fixturedef.restitution = material.m_restitution;
		pb->CreateFixture(&fixturedef);
		
		auto pf = pb->CreateFixture(&fixturedef);

		return pb;
	}
	b2Vec2 m_gravity;
	b2World m_world;
	Box2DConfig m_config;
	Optional<Texture> m_texture;
	
	int m_count;
};
