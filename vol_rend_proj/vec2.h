#ifndef VEC2_H
#define VEC2_H

#include <math.h>

class Vec2
{
    public:
        Vec2(float x=0, float y=0) :
            X(x), Y(y)
        {}

        float operator[](int i) const { return ((float*)&X)[i]; }
        float &operator[](int i) { return ((float*)&X)[i]; }
        friend Vec2 operator-(const Vec2 &v1, const Vec2 &v2); 
        friend Vec2 operator+(const Vec2 &v1, const Vec2 &v2); 
        friend float operator*( const Vec2 & v1, const Vec2& v2);
        friend Vec2 operator*( float x, const Vec2& v2);
        
        //friend ostream &operator<<(ostream &out, const Vec2& v);
                
        float norm() const;
        void normalize();
        void Set(float x,float y) { X=x; Y=y; }
        
    public:
        float X,Y;
};

inline Vec2 operator- (const Vec2 &v1, const Vec2 &v2 )
{
    return Vec2(v1.X - v2.X, v1.Y - v2.Y );
}

inline Vec2 operator+ (const Vec2 &v1, const Vec2 &v2 )
{
    return Vec2(v1.X + v2.X, v1.Y + v2.Y );
}
/*
inline ostream &operator<<(ostream &out, const Vec2 &v)
{
    out << " [ " << v.X << " " << v.Y << " ] ";
    return out;
}
*/
inline float Vec2::norm() const 
{
    return sqrt(X*X + Y*Y);
}

inline void Vec2::normalize()
{
    float mag = norm();
    X /= mag;
    Y /= mag;
}

inline float operator*( const Vec2 &v1, const Vec2 &v2)
{
    return v1.X*v2.X + v1.Y*v2.Y;
}

inline Vec2 operator*( float x, const Vec2 &v2)
{
    return Vec2( x*v2.X, x*v2.Y );
}

#endif
