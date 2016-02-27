/*
* Copyright (c) 2006-2007 Erin Catto http://www.box2d.org
*
* This software is provided 'as-is', without any express or implied
* warranty.  In no event will the authors be held liable for any damages
* arising from the use of this software.
* Permission is granted to anyone to use this software for any purpose,
* including commercial applications, and to alter it and redistribute it
* freely, subject to the following restrictions:
* 1. The origin of this software must not be misrepresented; you must not
* claim that you wrote the original software. If you use this software
* in a product, an acknowledgment in the product documentation would be
* appreciated but is not required.
* 2. Altered source versions must be plainly marked as such, and must not be
* misrepresented as being the original software.
* 3. This notice may not be removed or altered from any source distribution.
*/

//‚±‚±‚©‚çƒRƒsƒy
//http://ncj-blog.blogspot.jp/2010/04/blog-post_20.html

#pragma once

#include <Siv3D.hpp>
#include "Debugdraw.h"

using namespace std;

#include "GlobalConstant.h"

void DebugDraw::DrawPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color)
{
	FontAsset(L"DebugDraw").draw(L"DrawPolygon", 0, 0, Palette::Red);

	int i = 1;

	for(; i < vertexCount; i++)
	{
		Line( vertices[i-1].x*PixelPerMeter, vertices[i-1].y*PixelPerMeter, vertices[i].x*PixelPerMeter, vertices[i].y*PixelPerMeter ).draw( Color(255*color.r,255*color.g,255*color.b) );
	}
 
	Line( vertices[0].x*PixelPerMeter, vertices[0].y*PixelPerMeter, vertices[i-1].x*PixelPerMeter, vertices[i-1].y*PixelPerMeter ).draw(Color(255*color.r,255*color.g,255*color.b));
}

void DebugDraw::DrawSolidPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color)
{
	FontAsset(L"DebugDraw").draw(L"DrawSolidPolygon",0,32,Palette::Red);
	Color c(255*color.r,255*color.g,255*color.b);
	Color c1(255*color.r,255*color.g,255*color.b,128);

	std::vector<Vec2> pts;
	for(int i=0; i<vertexCount; ++i)
	{
		pts.emplace_back( vertices[i].x*PixelPerMeter, vertices[i].y*PixelPerMeter );
	}

	Polygon(pts).draw(c1);
	Polygon(pts).drawWireframe(1.0,c);
}

void DebugDraw::DrawCircle(const b2Vec2& center, float32 radius, const b2Color& color)
{
	FontAsset(L"DebugDraw").draw(L"DrawCircle",0,64,Palette::Red);

	Circle(center.x*PixelPerMeter, center.y*PixelPerMeter, radius*PixelPerMeter).draw( Color(255*color.r, 255*color.g,255*color.b,128) );
	Circle(center.x*PixelPerMeter, center.y*PixelPerMeter, radius*PixelPerMeter-2).draw( Palette::Black );
}

void DebugDraw::DrawSolidCircle(const b2Vec2& center, float32 radius, const b2Vec2& axis, const b2Color& color)
{
	FontAsset(L"DebugDraw").draw(L"DrawSolidCircle",0,96,Palette::Red);
	Circle(center.x*PixelPerMeter, center.y*PixelPerMeter, radius*PixelPerMeter).draw( Color(255*color.r, 255*color.g,255*color.b,255) );
	Circle(center.x*PixelPerMeter, center.y*PixelPerMeter, radius*PixelPerMeter-2).draw( Palette::Black );
	Circle(center.x*PixelPerMeter, center.y*PixelPerMeter, radius*PixelPerMeter-2).draw( Color(255*color.r, 255*color.g,255*color.b,128) );

	b2Vec2 b2p = center + radius * axis;

	Point p0(center.x*PixelPerMeter, center.y*PixelPerMeter);
	Point p1(b2p.x*PixelPerMeter, b2p.y*PixelPerMeter);

	Line(p0.x,p0.y,p1.x,p1.y).draw(Palette::Darkgray);
}

void DebugDraw::DrawSegment(const b2Vec2& p1, const b2Vec2& p2, const b2Color& color)
{
	FontAsset(L"DebugDraw").draw(L"DrawSegment",0,128,Palette::Red);
	Line( p1.x*PixelPerMeter, p1.y*PixelPerMeter, p2.x*PixelPerMeter, p2.y*PixelPerMeter ).draw( Color(255*color.r,255*color.g,255*color.b) );
}

void DebugDraw::DrawTransform(const b2Transform& xf)
{
	FontAsset(L"DebugDraw").draw(L"DrawTransform",0,160,Palette::Red);
	b2Vec2 p1 = xf.p, p2;
	const float32 k_axisScale = 0.4f;

	p2 = p1 + k_axisScale * xf.q.GetXAxis();
	Line(p1.x*PixelPerMeter, p1.y*PixelPerMeter, p2.x*PixelPerMeter, p2.y*PixelPerMeter).draw(Palette::Red);

	p2 = p1 + k_axisScale * xf.q.GetYAxis();
	Line(p1.x*PixelPerMeter, p1.y*PixelPerMeter, p2.x*PixelPerMeter, p2.y*PixelPerMeter).draw(Palette::Green);
}

void DebugDraw::DrawPoint(const b2Vec2& p, float32 size, const b2Color& color)
{
	FontAsset(L"DebugDraw").draw(L"DrawPoint",0,192,Palette::Red);
	int Size = size*PixelPerMeter;
	Rect((p.x-size/2)*PixelPerMeter, (p.y-size/2)*PixelPerMeter, Size, Size).draw(Color(255*color.r,255*color.g,255*color.b));
}

void DebugDraw::DrawString(int x, int y, const char *string, ...)
{
	FontAsset(L"DebugDraw").draw(L"DrawString",0,224,Palette::Red);
}

void DebugDraw::DrawAABB(b2AABB* aabb, const b2Color& c)
{
	FontAsset(L"DebugDraw").draw(L"DrawAABB",0,256,Palette::Red);
	Color clr(255*c.r, 255*c.g, 255*c.b);
	Point p[2] = {
		Point( aabb->lowerBound.x*PixelPerMeter, aabb->lowerBound.y*PixelPerMeter ),
		Point( aabb->upperBound.x*PixelPerMeter, aabb->upperBound.y*PixelPerMeter )
	};
	Rect( p[0], p[1].x-p[0].x, p[1].y-p[0].y ).drawFrame(1,0,clr);
}

void DebugDrawWireFrame::DrawPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color)
{
	int i = 1;

	for(; i < vertexCount; i++)
	{
		Line( vertices[i-1].x*PixelPerMeter, vertices[i-1].y*PixelPerMeter, vertices[i].x*PixelPerMeter, vertices[i].y*PixelPerMeter ).draw( Color(255*color.r,255*color.g,255*color.b) );
	}
 
	Line( vertices[0].x*PixelPerMeter, vertices[0].y*PixelPerMeter, vertices[i-1].x*PixelPerMeter, vertices[i-1].y*PixelPerMeter ).draw(Color(255*color.r,255*color.g,255*color.b));
}

void DebugDrawWireFrame::DrawSolidPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color)
{
	Color c = ColorF(color.r, color.g, color.b);
	Color c1(255*color.r,255*color.g,255*color.b,128);

	std::vector<Vec2> pts(vertexCount+1);

	for(int i=0; i<vertexCount; ++i)
	{
		pts[i].x = vertices[i].x*PixelPerMeter;
		pts[i].y = vertices[i].y*PixelPerMeter;
	}

	pts[vertexCount] = pts[0];

	LineString(pts).draw(c);
}

void DebugDrawWireFrame::DrawCircle(const b2Vec2& center, float32 radius, const b2Color& color)
{
	Circle(center.x*PixelPerMeter, center.y*PixelPerMeter, radius*PixelPerMeter).drawFrame( 1,1,Color(255*color.r, 255*color.g,255*color.b,128) );
}

void DebugDrawWireFrame::DrawSolidCircle(const b2Vec2& center, float32 radius, const b2Vec2& axis, const b2Color& color)
{
	Circle(center.x*PixelPerMeter, center.y*PixelPerMeter, radius*PixelPerMeter).drawFrame( 1,1,Color(255*color.r, 255*color.g,255*color.b,255) );

	b2Vec2 b2p = center + radius * axis;

	Point p0(center.x*PixelPerMeter, center.y*PixelPerMeter);
	Point p1(b2p.x*PixelPerMeter, b2p.y*PixelPerMeter);

	Line(p0.x,p0.y,p1.x,p1.y).draw(Palette::Darkgray);	
}

void DebugDrawWireFrame::DrawSegment(const b2Vec2& p1, const b2Vec2& p2, const b2Color& color)
{
	Line( p1.x*PixelPerMeter, p1.y*PixelPerMeter, p2.x*PixelPerMeter, p2.y*PixelPerMeter ).draw( Color(255*color.r,255*color.g,255*color.b) );
}

void DebugDrawWireFrame::DrawTransform(const b2Transform& xf)
{
	b2Vec2 p1 = xf.p, p2;
	const float32 k_axisScale = 0.4f;

	p2 = p1 + k_axisScale * xf.q.GetXAxis();
	Line(p1.x*PixelPerMeter, p1.y*PixelPerMeter, p2.x*PixelPerMeter, p2.y*PixelPerMeter).draw(Palette::Red);

	p2 = p1 + k_axisScale * xf.q.GetYAxis();
	Line(p1.x*PixelPerMeter, p1.y*PixelPerMeter, p2.x*PixelPerMeter, p2.y*PixelPerMeter).draw(Palette::Green);
}

void DebugDrawWireFrame::DrawPoint(const b2Vec2& p, float32 size, const b2Color& color)
{
	int Size = size*PixelPerMeter;
	Rect((p.x-size/2)*PixelPerMeter, (p.y-size/2)*PixelPerMeter, Size, Size).draw(Color(255*color.r,255*color.g,255*color.b));
}

void DebugDrawWireFrame::DrawString(int x, int y, const char *string, ...)
{
	//–¢ŽÀ‘•
}

void DebugDrawWireFrame::DrawAABB(b2AABB* aabb, const b2Color& c)
{
	Color clr(255*c.r, 255*c.g, 255*c.b);
	Point p[2] = {
		Point( aabb->lowerBound.x*PixelPerMeter, aabb->lowerBound.y*PixelPerMeter ),
		Point( aabb->upperBound.x*PixelPerMeter, aabb->upperBound.y*PixelPerMeter )
	};
	Rect( p[0], p[1].x-p[0].x, p[1].y-p[0].y ).drawFrame(1,0,clr);
}
