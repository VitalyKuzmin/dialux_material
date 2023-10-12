#include <math.h>
#include <iostream>
using namespace std;

#define _epsilon 0.000001f
#define _epsilond 0.00000000001f
#define _tol 0.000001f
#define _told 0.00000000001f

#define _pi 3.141592653589793238462643383279f
#define _2pi 6.283185307179586476925286766559f
#define _hpi 1.57079632679489661923132169163f

#define _rad(deg) ((deg) / 180.0f * _pi)
#define _deg(rad) (180.0f * (rad) / _pi)

#define _r45 0.78539816339744830961566084581f
#define _r90 1.57079632679489661923132169163f

#define _r180 3.14159265358979323846264338327f
#define _r270 4.71238898038468985769396507491f
#define _r360 6.28318530717958647692528676655f

#define _max_float 10000000.0f
#define _max_int 10000000
#define _max_string 260

#define gre_min(a, b) (((a) < (b)) ? (a) : (b))
#define gre_max(a, b) (((a) > (b)) ? (a) : (b))
#define gre_abs(a) ((a) < 0 ? -(a) : a)
#define gre_sqr(x) ((x) * (x))

typedef unsigned int u32;

//    vec2
struct vec2
{
    float x;
    float y;

    vec2(float x = 0.0f, float y = 0.0f) { set(x, y); }
    void set(float x, float y)
    {
        this->x = x;
        this->y = y;
    }
    vec2 operator+(const vec2 &pt) const { return vec2(x + pt.x, y + pt.y); }
    vec2 operator-(const vec2 &pt) const { return vec2(x - pt.x, y - pt.y); }
    float &operator[](int i) { return ((float *)(this))[i]; }
    operator float *() const { return (float *)(this); }
    float length() const { return (float)sqrt(x * x + y * y); }

    void normalize()
    {
        float m = length();

        if (m > 0)
        {
            float im = 1.0f / m;
            x *= im;
            y *= im;
        }
    }
};

// vec3
struct vec3
{
    float x, y, z;

    vec3(float x = 0.0f, float y = 0.0f, float z = 0.0f) { set(x, y, z); }
    vec3(const float *p)
    {
        this->x = p[0];
        this->y = p[1];
        this->z = p[2];
    }
    vec3(const vec3 &v)
    {
        this->x = v.x;
        this->y = v.y;
        this->z = v.z;
    }
    vec3 &operator=(const vec3 &v)
    {
        this->x = v.x;
        this->y = v.y;
        this->z = v.z;
        return (*this);
    }
    ~vec3() {}

    void set(float x = 0.0f, float y = 0.0f, float z = 0.0f)
    {
        this->x = x;
        this->y = y;
        this->z = z;
    }
    void offset(float x = 0.0f, float y = 0.0f, float z = 0.0f)
    {
        this->x += x;
        this->y += y;
        this->z += z;
    }

    vec3 operator-() const { return vec3(-x, -y, -z); }
    vec3 operator+(const vec3 &v) const { return vec3(this->x + v.x, this->y + v.y, this->z + v.z); }
    vec3 operator-(const vec3 &v) const { return vec3(this->x - v.x, this->y - v.y, this->z - v.z); }
    vec3 operator*(const vec3 &v) const { return vec3(this->x * v.x, this->y * v.y, this->z * v.z); }
    vec3 operator/(const vec3 &v) const { return vec3((v.x != 0) ? this->x / v.x : this->x, (v.y != 0) ? this->y / v.y : this->y, (v.z != 0) ? this->z / v.z : this->z); }

    vec3 &operator+=(const vec3 &v)
    {
        this->x += v.x;
        this->y += v.y;
        this->z += v.z;
        return (*this);
    }
    vec3 &operator-=(const vec3 &v)
    {
        this->x -= v.x;
        this->y -= v.y;
        this->z -= v.z;
        return (*this);
    }
    vec3 &operator*=(const vec3 &v)
    {
        this->x *= v.x;
        this->y *= v.y;
        this->z *= v.z;
        return (*this);
    }
    vec3 &operator/=(const vec3 &v)
    {
        if (v.x != 0)
            this->x /= v.x;
        if (v.y != 0)
            this->y /= v.y;
        if (v.z != 0)
            this->z /= v.z;
        return (*this);
    }

    // bool operator==(const vec3& v) {  return (math::is_equal(this->x, v.x) && math::is_equal(this->y, v.y) && math::is_equal(this->z, v.z)) ? true : false;  }

    vec3 operator*(float u) const { return vec3(x * u, y * u, z * u); }
    friend vec3 operator*(const float u, const vec3 &vec);

    vec3 operator+(float u) const { return vec3(x + u, y + u, z + u); }
    vec3 operator-(float u) const { return vec3(x - u, y - u, z - u); }
    vec3 &operator+=(float u)
    {
        x += u;
        y += u;
        z += u;
        return (*this);
    }
    vec3 &operator-=(float u)
    {
        x -= u;
        y -= u;
        z -= u;
        return (*this);
    }
    vec3 &operator*=(float u)
    {
        x *= u;
        y *= u;
        z *= u;
        return (*this);
    }
    vec3 &operator/=(float u)
    {
        x /= u;
        y /= u;
        z /= u;
        return (*this);
    }

    operator float *() const { return (float *)(this); }
    float length() const { return sqrtf(x * x + y * y + z * z); }
    void reset()
    {
        x = 0.0f;
        y = 0.0f;
        z = 0.0f;
    }
    void normalize()
    {
        float m = length();

        if (m > 0)
        {
            float im = 1.0f / m;
            x *= im;
            y *= im;
            z *= im;
        }
    }

    float angle(const vec3 &vec) const
    {
        // return angle between [0, 180]
        float l1 = this->length();
        float l2 = vec.length();
        float d = this->dot(vec);
        return acosf(d / (l1 * l2)) / _pi * 180.0f;
    }

    vec3 norm() const
    {
        vec3 nrm(*this);
        nrm.normalize();
        return nrm;
    }

    void adapt(float eps = 0.000001f)
    {
        if (x > -eps && x < eps)
            x = 0.0f;
        if (y > -eps && y < eps)
            y = 0.0f;
        if (z > -eps && z < eps)
            z = 0.0f;
    }

    void setupMin(const vec3 &v)
    {
        if (x > v.x)
            x = v.x;
        if (y > v.y)
            y = v.y;
        if (z > v.z)
            z = v.z;
    }

    void setupMax(const vec3 &v)
    {
        if (x < v.x)
            x = v.x;
        if (y < v.y)
            y = v.y;
        if (z < v.z)
            z = v.z;
    }

    bool is_zero() const { return (x == 0.0f && y == 0.0f && z == 0.0f); }
    float distance(const vec3 &v) const
    {
        float dx = x - v.x;
        float dy = y - v.y;
        float dz = z - v.z;
        return sqrtf(dx * dx + dy * dy + dz * dz);
    }
    vec3 inv() const { return vec3(-x, -y, -z); }

    float dot(const vec3 &v) const { return (x * v.x + y * v.y + z * v.z); }
    vec3 cross(const vec3 &v) const { return vec3(y * v.z - z * v.y, z * v.x - x * v.z, x * v.y - y * v.x); }
    bool equal(const vec3 &v, float epsilon) const { return fabs(x - v.x) < epsilon && fabs(y - v.y) < epsilon && fabs(z - v.z) < epsilon; }
};

//    vertex3f (89)
#pragma pack(push, 1)
struct vertex3f
{
    vec3 p;
    vec3 n;
    color4f c;
    vec2 t;

    color4f cg;
    color4f cr;

    color3f vs; // прямая составляющая освещенности
    color3f vd; // диффузная состовляющая освещенности

    float v;
    float nv;
    float m;
    unsigned char s;
    vertex3f() : v(0.0f), nv(0.0f), s(0), m(-1.0f) {}
};
#pragma pack(pop)

typedef vertex3f vertex;

struct color3f
{
public:
    float r, g, b;

    // color3f() { set(0.0, 0.0, 0.0); }

    color3f(float r = 0.0f, float g = 0.0f, float b = 0.0f) { set(r, g, b); }

    color3f(const color3f &rgb)
    {
        this->r = rgb.r;
        this->g = rgb.g;
        this->b = rgb.b;
    }

    void set(const float &R, const float &G, const float &B)
    {
        r = R;
        g = G;
        b = B;
    }

    void show()
    {
        cout << "(" << r << ", " << g << ", " << b << ")";
    }

    void show255()
    {
        cout << "(" << round(r * 255) << ", " << round(g * 255) << ", " << round(b * 255) << ")";
    }

    void clear()
    {
        set(0.0, 0.0, 0.0);
    }

    float sum()
    {
        return r + g + b;
    }
    float max()
    {
        float max = r;
        max = (g > max) ? g : max;
        max = (b > max) ? b : max;
        return max;
    }

    // Overloading * operator
    color3f operator*(const float &scalar)
    {
        return color3f(r * scalar, g * scalar, b * scalar);
    }

    // Overloading rgb*rgb operator
    color3f operator*(const color3f &rgb)
    {
        return color3f(r * rgb.r, g * rgb.g, b * rgb.b);
    }

    // Overloading rgb+rgb operator
    color3f operator+(const color3f &rgb)
    {
        return color3f(r + rgb.r, g + rgb.g, b + rgb.b);
    }

    bool operator==(const color3f &rgb)
    {
        return (r == rgb.r && g == rgb.g && b == rgb.b);
    }

    bool operator!=(const color3f &rgb)
    {
        return (r != rgb.r && g != rgb.g && b != rgb.b);
    }

    bool isEmpty()
    {
        return (r == 0.0 && g == 0.0 && b == 0.0);
    }

    // Overloading /= operator
    color3f &operator/=(const float &scalar)
    {
        r /= scalar;
        g /= scalar;
        b /= scalar;
        return (*this); // return color3f(r / scalar, g / scalar, b / scalar);
    }

    color3f getNorm()
    {
        float Max = max();
        return color3f((r / Max), (g / Max), (b / Max));
    }
};

//////////////////////////////////////////////////////////////////////////
// color4
struct color4f
{
    float r;
    float g;
    float b;
    float a;

    color4f(float r = 0.0f, float g = 0.0f, float b = 0.0f, float a = 1.0f) { set(r, g, b, a); }
    color4f(const color3f &other) { set(other.r, other.g, other.b, 1.0f); }
    void set(float r, float g, float b, float a = 1.0f)
    {
        this->r = r;
        this->g = g;
        this->b = b;
        this->a = a;
    }
    operator float *() const { return (float *)(this); }
    color4f operator*(float u) const { return color4f(r * u, g * u, b * u); }

    void operator*=(float u)
    {
        r *= u;
        g *= u;
        b *= u;
    }

    color4f &operator=(const color3f &other)
    {
        r = other.r;
        g = other.g;
        b = other.b;
        a = 1.0f;
        return (*this);
    }
    operator color3f() const { return color3f(r, g, b); }

    void clamp()
    {
        r = (r < 0) ? 0 : r > 1 ? 1
                                : r;
        g = (g < 0) ? 0 : g > 1 ? 1
                                : g;
        b = (b < 0) ? 0 : b > 1 ? 1
                                : b;
        a = (a < 0) ? 0 : a > 1 ? 1
                                : a;
    }
};

//////////////////////////////////////////////////////////////////////////
// vec4
struct vec4
{
    float x, y, z, w;

    vec4(float x = 0.0f, float y = 0.0f, float z = 0.0f, float w = 1.0f) { set(x, y, z, w); }
    vec4(const vec4 &v)
    {
        x = v.x;
        y = v.y;
        z = v.z;
        w = v.w;
    }
    ~vec4() {}

    void set(float x = 0.0f, float y = 0.0f, float z = 0.0f, float w = 1.0f)
    {
        this->x = x;
        this->y = y;
        this->z = z;
        this->w = w;
    }
    operator vec3() const { return vec3(x, y, z); }
    vec4 &operator=(const vec4 &v)
    {
        this->x = v.x;
        this->y = v.y;
        this->z = v.z;
        this->w = v.w;
        return (*this);
    }

    vec4 operator+(const vec4 &v) const { return vec4(this->x + v.x, this->y + v.y, this->z + v.z, this->w + v.w); }
    vec4 operator-(const vec4 &v) const { return vec4(this->x - v.x, this->y - v.y, this->z - v.z, this->w - v.w); }
    vec4 operator*(const vec4 &v) const { return vec4(this->x * v.x, this->y * v.y, this->z * v.z, this->w * v.w); }
    vec4 operator/(const vec4 &v) const { return vec4((v.x != 0) ? this->x / v.x : this->x, (v.y != 0) ? this->y / v.y : this->y, (v.z != 0) ? this->z / v.z : this->z, (v.w != 0) ? this->w / v.w : this->w); }

    vec4 &operator+=(const vec4 &v)
    {
        this->x += v.x;
        this->y += v.y;
        this->z += v.z;
        this->w += v.w;
        return (*this);
    }
    vec4 &operator-=(const vec4 &v)
    {
        this->x -= v.x;
        this->y -= v.y;
        this->z -= v.z;
        this->w -= v.w;
        return (*this);
    }
    vec4 &operator*=(const vec4 &v)
    {
        this->x *= v.x;
        this->y *= v.y;
        this->z *= v.z;
        this->w *= v.w;
        return (*this);
    }
    vec4 &operator/=(const vec4 &v)
    {
        if (v.x != 0)
            this->x /= v.x;
        if (v.y != 0)
            this->y /= v.y;
        if (v.z != 0)
            this->z /= v.z;
        if (v.w != 0)
            this->w /= v.w;
        return (*this);
    }

    vec4 operator*(float u) { return vec4(x * u, y * u, z * u, w * u); }
    vec4 &operator+=(float u)
    {
        x += u;
        y += u;
        z += u;
        w += u;
        return (*this);
    }
    vec4 &operator-=(float u)
    {
        x -= u;
        y -= u;
        z -= u;
        w -= u;
        return (*this);
    }
    vec4 &operator*=(float u)
    {
        x *= u;
        y *= u;
        z *= u;
        w *= u;
        return (*this);
    }
    vec4 &operator/=(float u)
    {
        x /= u;
        y /= u;
        z /= u;
        w /= u;
        return (*this);
    }
    bool operator==(const vec4 &v) { return x == v.x && y == v.y && z == v.z && w == v.w; }
    operator float *() const { return (float *)(this); }
    float mag() const { return sqrtf(x * x + y * y + z * z + w * w); }

    void normalize()
    {
        float m = mag();

        if (m > 0)
        {
            float im = 1.0f / m;
            x *= im;
            y *= im;
            z *= im;
            w *= im;
        }
    }

    bool is_zero() { return (x == 0.0f && y == 0.0f && z == 0.0f && w == 0.0f); }
    float distance(vec4 &v)
    {
        float dx = x - v.x;
        float dy = y - v.y;
        float dz = z - v.z;
        float dw = w - v.w;
        return sqrtf(dx * dx + dy * dy + dz * dz + dw * dw);
    }
};

struct material
{

    color4f color;    // sRGB [0,1] (https://en.wikipedia.org/wiki/SRGB)
    u32 type;         // 0 - Metallic;  1 - Painted; 2 - Transparent;
    float Refl;       // Reflection factor        [0,0.9]
    float Kspec_refl; // Reflective coating       [0,1]
    float Trans;      // Degree of transmission   [0,1]
    float N;          // Refractive index         [1,2]
    float Shin;       // Shininness               [1,128]
};

class Geometry
{

    material m_material;

public:
    const material &getMaterial() const { return m_material; }
};