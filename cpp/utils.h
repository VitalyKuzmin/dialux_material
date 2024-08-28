#include <math.h>
#include <iostream>
using namespace std;

typedef unsigned int u32;

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

    color3f operator*(const float &scalar)
    {
        return color3f(r * scalar, g * scalar, b * scalar);
    }

    color3f operator*(const color3f &rgb)
    {
        return color3f(r * rgb.r, g * rgb.g, b * rgb.b);
    }

    color3f operator+(const color3f &rgb)
    {
        return color3f(r + rgb.r, g + rgb.g, b + rgb.b);
    }

    color3f operator-(const color3f &rgb)
    {
        return color3f(r - rgb.r, g - rgb.g, b - rgb.b);
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

    color3f &operator/=(const color3f &rgb)
    {
        r /= rgb.r;
        g /= rgb.g;
        b /= rgb.b;
        return (*this);
    }

    color3f &operator*=(const color3f &rgb)
    {
        r *= rgb.r;
        g *= rgb.g;
        b *= rgb.b;
        return (*this);
    }

    color3f &operator-=(const color3f &rgb)
    {
        r -= rgb.r;
        g -= rgb.g;
        b -= rgb.b;
        return (*this);
    }

    color3f &operator+=(const color3f &rgb)
    {
        r += rgb.r;
        g += rgb.g;
        b += rgb.b;
        return (*this);
    }

    color3f &operator/=(const float &scalar)
    {
        r /= scalar;
        g /= scalar;
        b /= scalar;
        return (*this);
    }

    color3f &operator*=(const float &scalar)
    {
        r *= scalar;
        g *= scalar;
        b *= scalar;
        return (*this);
    }

    color3f &operator-=(const float &scalar)
    {
        r -= scalar;
        g -= scalar;
        b -= scalar;
        return (*this);
    }

    color3f &operator+=(const float &scalar)
    {
        r += scalar;
        g += scalar;
        b += scalar;
        return (*this);
    }

    color3f getNorm()
    {
        float Max = max();
        return color3f((r / Max), (g / Max), (b / Max));
    }
};

typedef color3f color4f;

struct vertex3f
{

    color4f c;
    color4f cg;
    color4f cr;

    color3f vl; // прямая составляющая освещенности
    color3f vd; // диффузная состовляющая освещенности
    color3f vs; // суммараная освещенность

    void show()
    {
        cout << "cg";
        cg.show255();
        cout << "\n";

        cout << "c";
        c.show255();
        cout << "\n";
    }

    void show_point()
    {
        cout << "Point---------------\n";
        cout << "vl";
        vl.show();
        cout << "\n";

        cout << "vd";
        vd.show();
        cout << "\n";
        cout << "\n";
    }

    vertex3f() {};

    vertex3f(const vertex3f &v)
    {
        vl = v.vl;
        vd = v.vd;
        vs = vd + vl;
        c = v.c;
        cg = v.cg;
        cr = v.cr;
    }
};

typedef vertex3f vertex;
