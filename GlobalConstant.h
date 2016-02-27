#pragma once

static const int PixelPerMeter = 32;
static const int MaxVelocity = 4;

inline b2Vec2 tob2Vec2(const Vec2& v)
{
	return b2Vec2(static_cast<float32>(v.x/PixelPerMeter),static_cast<float32>(v.y/PixelPerMeter));
}

inline Vec2 toVec2(const b2Vec2& v)
{
	return Vec2(v.x*PixelPerMeter,v.y*PixelPerMeter);
}

inline double toPixelDouble(double v)
{
	return v * PixelPerMeter;
}