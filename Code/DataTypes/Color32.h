#pragma once
#include <algorithm>

// struct for a 32 bit color structure
// keeps all channels in a 8 bit unsigned char
// clamp all values to stay within the size limit
struct Color32
{
public:

	// predefined colors to use
	static Color32 WHITE; 
	static Color32 BLACK;
	static Color32 RED;
	static Color32 GREEN;
	static Color32 BLUE;

	unsigned char r;
	unsigned char g;
	unsigned char b;
	unsigned char a;

	Color32(){}

	Color32(unsigned char r, unsigned char g, unsigned char b, unsigned char a) :
		r(r),
		g(g),
		b(b),
		a(a){}

	Color32 operator + (const Color32& obj)
	{		
		return Color32(r + obj.r < 255 ? r + obj.r : 255,
					   g + obj.g < 255 ? g + obj.g : 255,
					   b + obj.b < 255 ? b + obj.b : 255,
					   a + obj.a < 255 ? a + obj.a : 255);
	}

	Color32 operator - (const Color32& obj)
	{
		return Color32(r - obj.r > 0 ? r - obj.r :0,
			           g - obj.g > 0 ? g - obj.g :0,
			           b - obj.b > 0 ? b - obj.b :0,
			           a - obj.a > 0 ? a - obj.a :0);
	}

	Color32 operator * (const Color32& obj)
	{
		return Color32(r * obj.r < 255 ? r * obj.r : 255,
			           g * obj.g < 255 ? g * obj.g : 255,
			           b * obj.b < 255 ? b * obj.b : 255,
			           a * obj.a < 255 ? a * obj.a : 255);
	}

	Color32 operator / (const Color32& obj)
	{
		return Color32(r / obj.r, g / obj.g, b / obj.b, a / obj.a);			           			           			            
	}

	bool operator == (const Color32& obj)
	{
		return r == obj.r && g == obj.g && b == obj.b && a == obj.a;
	}

	bool operator != (const Color32& obj)
	{
		return r != obj.r || g != obj.g || b != obj.b || a != obj.a;
	}

};