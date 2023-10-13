#include <math.h>
#include <iostream>
using namespace std;

// Constants RGB -> Y (https://en.wikipedia.org/wiki/Relative_luminance)
const float K_R = 0.2126;
const float K_G = 0.7152;
const float K_B = 0.0722;

#define gre_min(a, b) (((a) < (b)) ? (a) : (b))
#define gre_max(a, b) (((a) > (b)) ? (a) : (b))
#define gre_abs(a) ((a) < 0 ? -(a) : a)
#define gre_sqr(x) ((x) * (x))

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

    void show()
    {
        cout << "c";
        c.show();
        cout << "\n";

        cout << "cg";
        cg.show();
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

    vertex3f(){};

    void set(color3f VL, color3f VD)
    {
        vl = VL;
        vd = VD;
    }

    vertex3f(const vertex3f &v)
    {
        vl = v.vl;
        vd = v.vd;
        c = v.c;
        cg = v.cg;
        cr = v.cr;
    }
};

typedef vertex3f vertex;

struct material
{
    color4f color; // sRGB [0,1] (https://en.wikipedia.org/wiki/SRGB)

    u32 type;         // 0 - Metallic;  1 - Painted; 2 - Transparent;
    float Refl;       // Reflection factor        [0,0.9]
    float Kspec_refl; // Reflective coating       [0,1]
    float Trans;      // Degree of transmission   [0,1]
    float N;          // Refractive index         [1,2]
    float Shin;       // Shininness               [1,128]

    // Calc Params
    color3f cl; // linear specular Y RGB[0,1]
    color3f cd; // linear diffuse Y RGB[0,1]
    color3f cs; // linear sum Y RGB[0,1]

    material(){

    };

    material(const material &mat)
    {
        set(mat.color, mat.type, mat.Refl, mat.Kspec_refl, mat.Trans, mat.N, mat.Shin);
    };

    material(const color3f &c, const unsigned int &t, const float &refl,
             const float &kspec_refl, const float &trans, const float &n, const float &shin)
    {
        set(c, t, refl, kspec_refl, trans, n, shin);
    }

    void set(const color3f &c, const unsigned int &t, const float &refl,
             const float &kspec_refl, const float &trans, const float &n, const float &shin)
    {
        color = color3f(c);
        type = t;
        Refl = refl;
        Kspec_refl = kspec_refl;
        Trans = trans;
        N = n;
        Shin = shin;
    }

    void prepareColors();

    void show()
    {

        cout << "Material--------------\n";
        cout << "color: ";
        color.show();
        cout << "\n";

        cout << "type: ";
        cout << type;
        cout << "\n";

        cout << "Refl: ";
        cout << Refl;
        cout << "\n";

        cout << "Kspec_refl: ";
        cout << Kspec_refl;
        cout << "\n";

        cout << "Trans: ";
        cout << Trans;
        cout << "\n";

        cout << "cl: ";
        cl.show();
        cout << "\n";

        cout << "cd: ";
        cd.show();
        cout << "\n";

        cout << "cs: ";
        cs.show();
        cout << "\n";
        cout << "\n";
    }
};
