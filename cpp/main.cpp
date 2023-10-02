#include <iostream>
#include <iomanip>
#include <cmath>
#include <vector>
#include <string>

using namespace std;

class RGB
{
public:
    float r; // red
    float g; // green
    float b; // blue

    RGB()
    {
        Set(0.0, 0.0, 0.0);
    }
    RGB(const float &R, const float &G, const float &B)
    {
        Set(R, G, B);
    }

    RGB(const RGB &rgb)
    {
        this->r = rgb.r;
        this->g = rgb.g;
        this->b = rgb.b;
    }

    void Set(const float &R, const float &G, const float &B)
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
        Set(0.0, 0.0, 0.0);
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
    RGB operator*(const float &scalar)
    {
        return RGB(r * scalar, g * scalar, b * scalar);
    }

    // Overloading rgb*rgb operator
    RGB operator*(const RGB &rgb)
    {
        return RGB(r * rgb.r, g * rgb.g, b * rgb.b);
    }

    // Overloading rgb*rgb operator
    RGB operator+(const RGB &rgb)
    {
        return RGB(r + rgb.r, g + rgb.g, b + rgb.b);
    }

    bool operator==(const RGB &rgb)
    {
        return (r == rgb.r && g == rgb.g && b == rgb.b);
    }

    bool operator!=(const RGB &rgb)
    {
        return (r != rgb.r && g != rgb.g && b != rgb.b);
    }

    bool isEmpty()
    {
        return (r == 0.0 && g == 0.0 && b == 0.0);
    }

    // Overloading /= operator
    RGB &operator/=(const float &scalar)
    {
        r /= scalar;
        g /= scalar;
        b /= scalar;
        return (*this); // return RGB(r / scalar, g / scalar, b / scalar);
    }

    RGB getNorm()
    {
        float Max = max();
        return RGB((r / Max), (g / Max), (b / Max));
    }
};

// Tools ------------------------------------------------------------------------------------

// sRGB <-> lRGB (https://mina86.com/2019/srgb-xyz-conversion)
float toLinear(float value)
{
    if (value <= 0.04045)
    {
        value = value / 12.92; // * 255
    }
    else
    {
        value = pow((value + 0.055) / 1.055, 2.4); //* 255
    }
    return value;
};

float fromLinear(float value)
{
    if (value <= 0.0031308)
    {
        value = value * 12.92; // * 255
    }
    else
    {
        value = 1.055 * pow(value, 1 / 2.4) - 0.055; //* 255
    }
    return value;
};

void RGBtoLinear(RGB &rgb)
{
    rgb.r = toLinear(rgb.r);
    rgb.g = toLinear(rgb.g);
    rgb.b = toLinear(rgb.b);
}

void RGBfromLinear(RGB &rgb)
{
    rgb.r = fromLinear(rgb.r);
    rgb.g = fromLinear(rgb.g);
    rgb.b = fromLinear(rgb.b);
}

// Constants RGB -> Y (https://en.wikipedia.org/wiki/Relative_luminance)
const float K_R = 0.2126;
const float K_G = 0.7152;
const float K_B = 0.0722;

// Change rgb by Luminance
unsigned int rgb_max_i(float *arr, unsigned int no_index = -1)
{
    float max = 0;
    unsigned int index = 0;

    for (int i = 0; i < 3; i++)
    {
        if (i == no_index)
            continue;
        if (arr[i] > max)
        {
            max = arr[i];
            index = i;
        }
    }
    return index;
};

float getY(const RGB &rgb)
{
    return K_R * toLinear(rgb.r) + K_G * toLinear(rgb.g) + K_B * toLinear(rgb.b);
}

void toGray(RGB &rgb)
{
    float Y = fromLinear(getY(rgb));
    rgb = RGB(Y, Y, Y);
}

void changeLuminance(RGB &rgb, const float &Ynew)
{
    float color[3];

    // sRGB to linear RGB --------------------------------
    color[0] = toLinear(rgb.r);
    color[1] = toLinear(rgb.g);
    color[2] = toLinear(rgb.b);

    const float K[3] = {K_R, K_G, K_B};

    // for no division to null errors --------------------------------
    for (int j = 0; j < 3; j++)
    {
        if (color[j] == 0.0)
            color[j] = 0.000000001;
    }

    // Change Luminance ---------------------------------------------------
    float sum = K[0] * color[0] + K[1] * color[1] + K[2] * color[2];
    float coeff = Ynew / sum;
    unsigned int index = rgb_max_i(color);
    color[index] *= coeff;
    if (color[index] > 1)
    {
        color[index] = 1;
        sum = 0;
        for (int j = 0; j < 3; j++)
        {
            if (j == index)
                continue;
            sum += color[j] * K[j];
        }
        coeff = (Ynew - K[index]) / sum;
        unsigned int index2 = rgb_max_i(color, index);
        color[index2] = color[index2] * coeff;
        if (color[index2] > 1)
        {
            color[index2] = 1;
            unsigned int index3;
            for (int j = 0; j < 3; j++)
            {
                if (j == index || j == index2)
                    continue;
                sum += color[j] * K[j];
                index3 = j;
            }
            coeff = (Ynew - K[index] - K[index2]) / sum;

            color[index3] *= coeff;

            if (color[index3] > 1)
                color[index3] = 1;
        }
        else
        {
            for (int j = 0; j < 3; j++)
            {
                if (j == index || j == index2)
                    continue;
                color[j] *= coeff;
            }
        }
    }
    else
    {
        for (int j = 0; j < 3; j++)
        {
            if (j == index)
                continue;
            color[j] *= coeff;
        }
    }

    // linear RGB to sRGB --------------------------------
    rgb.r = fromLinear(color[0]);
    rgb.g = fromLinear(color[0]);
    rgb.b = fromLinear(color[1]);
};

void changeLuminance2(float *color, float Ynew)
{
    const float K[3] = {K_R, K_G, K_B};
    // Change Luminance ---------------------------------------------------
    float sum = K[0] * color[0] + K[1] * color[1] + K[2] * color[2];
    float coeff; //= Ynew / sum;
    unsigned int index = rgb_max_i(color);
    // color[index] *= coeff;
    if (color[index] > 1)
    {
        color[index] = 1;
        sum = 0;
        for (int j = 0; j < 3; j++)
        {
            if (j == index)
                continue;
            sum += color[j] * K[j];
        }
        coeff = (Ynew - K[index]) / sum;
        unsigned int index2 = rgb_max_i(color, index);
        color[index2] = color[index2] * coeff;
        if (color[index2] > 1)
        {
            color[index2] = 1;
            unsigned int index3;
            for (int j = 0; j < 3; j++)
            {
                if (j == index || j == index2)
                    continue;
                sum += color[j] * K[j];
                index3 = j;
            }
            coeff = (Ynew - K[index] - K[index2]) / sum;

            color[index3] *= coeff;

            if (color[index3] > 1)
                color[index3] = 1;
        }
        else
        {
            for (int j = 0; j < 3; j++)
            {
                if (j == index || j == index2)
                    continue;
                color[j] *= coeff;
            }
        }
    }
    else
    {
        for (int j = 0; j < 3; j++)
        {
            if (j == index)
                continue;
            color[j] *= coeff;
        }
    }
}

// Получить цвет пикселя из прямой и отраженной составляющей луча падающий на определенный материал
RGB GetColor(RGB Direct, RGB Photon, float Ymax, RGB diffuse, RGB specular, RGB transmission)
{
    // Нормируем результаты расчета
    Direct /= Ymax;
    Photon /= Ymax;

    // sRGB -> linearRGB
    RGBtoLinear(diffuse);
    RGBtoLinear(specular);
    RGBtoLinear(transmission);

    // Производим расчет
    RGB Yrgb = Direct * specular + Photon * diffuse;
    RGB Yrgb_trans = (Direct + Photon) * transmission; // прозрачная часть если нужна

    // linearRGB -> sRGB
    RGBfromLinear(Yrgb);
    RGBfromLinear(Yrgb_trans); // прозрачная часть если нужна

    return Yrgb;
}

void NormaliseY(float &Ysum, float &Y1, float &Y2)
{
    if (Ysum > 1)
    {
        // Y1/Y2 = K
        // Y1+Y2 = 1
        // --------------
        // Y2 = 1 - K*Y2
        // Y1 = 1 - Y2
        float K = Y1 / Y2;
        Y2 = 1 - K * Y2;
        Y1 = 1 - Y2;
        Ysum = 1;
    }
}

// // Получить материал из  Phong материала
RGB OpenGLToMaterial(RGB Ka, RGB Kd, RGB Ks, RGB Ke, float Ns, float Ni, float d)
{
    // Ka -  Ambient color       [[0,1],[0,1],[0,1]]
    // Kd -  Diffuse color       [[0,1],[0,1],[0,1]]
    // Ks -  Specular color      [[0,1],[0,1],[0,1]]
    // Ke -  Emission color      [[0,1],[0,1],[0,1]]

    // Ns -  Shininess           [0,128]
    // Ni -  Refractive index    [1,2]

    // d -   dissolve (1/opacity)  [0,1]

    float type = 0; // 0 - Metallic;  1 - Painted; 2 - Transparent;

    if (d < 1.0) // && illum > 3
    {
        type = 2;
    }
    else if (Kd == Ks)
    {
        type = 1;
    }

    RGB diffuse;
    RGB specular;
    RGB ambient;
    RGB emission;

    float Trans;
    float Shin;
    float N;

    emission = RGB(0, 0, 0);
    Shin = Ns;

    RGB color = Kd;
    if (color.isEmpty())
    {
        color = Ks;
    }
    ambient = RGB(color);

    if (type == 0) // Type::Metallic
    {
        diffuse = RGB(color);
        specular = RGB(color);

        float Ys = getY(Ks);
        float Yd = getY(Kd);
        float Y = Ys + Yd;

        NormaliseY(Y, Ys, Yd);
        changeLuminance(specular, Ys);
        changeLuminance(diffuse, Yd);

        Trans = 0;
        N = 1;
        // Shin = 40;
    }
    else if (type == 1) // Type::Painted
    {
        diffuse = RGB(color);
        specular = RGB(0, 0, 0); // Grayscale

        float Ys = getY(Ks);
        float Yd = getY(Kd);
        float Y = Ys + Yd;

        NormaliseY(Y, Ys, Yd);
        changeLuminance(specular, Ys);
        changeLuminance(diffuse, Yd);

        Trans = 0;
        N = 1;
        // Shin = 80;
    }
    else if (type == 2) // Type::Transparent
    {

        diffuse = RGB(0, 0, 0); // zero
        specular = RGB(color);
        Trans = 1 / d;
        float Refl = getY(Ks) + getY(Kd);
        float Y = Refl + Trans;

        NormaliseY(Y, Refl, Trans);
        changeLuminance(specular, Refl);
        changeLuminance(ambient, Y);
        N = Ns;
        // Shin = 40;
    }
}

// Получить материал из  Phong материала
RGB OpenGLToDialuxMaterial(RGB Ka, RGB Kd, RGB Ks, RGB Ke, float Ns, float Ni, float d)
{
    // Ka -  Ambient color       [[0,1],[0,1],[0,1]]
    // Kd -  Diffuse color       [[0,1],[0,1],[0,1]]
    // Ks -  Specular color      [[0,1],[0,1],[0,1]]
    // Ke -  Emission color      [[0,1],[0,1],[0,1]]

    // Ns -  Shininess           [0,128]
    // Ni -  Refractive index    [1,2]

    // d -   dissolve (1/opacity)  [0,1]

    float type = 1; // 0 - Metallic;  1 - Painted; 2 - Transparent;

    if (d < 1.0)
    {
        type = 2;
    }
    else if (!Ks.isEmpty() && Kd == Ks)
    {
        type = 0;
    }

    RGB diffuse;
    RGB specular;
    RGB ambient;
    RGB emission; // not use

    float Refl, Kspec_refl, Trans, N, Shin;

    // Trans = 0;
    // N = 1;
    // Shin = Ns;

    RGB color = Kd;
    if (color.isEmpty())
    {
        color = Ks;
    }

    if (type == 0) // Type::Metallic
    {

        float Ys = getY(Ks) + getY(Ka);
        float Yd = getY(Kd);
        float Y = Ys + Yd;
        NormaliseY(Y, Ys, Yd);

        changeLuminance(color, Y);

        Refl = Y;
        Kspec_refl = Ys / Y;
        Shin = 40;
    }
    else if (type == 1) // Type::Painted
    {

        float Ys = getY(Ks);
        float Yd = getY(Kd) + getY(Ka);
        float Y = Ys + Yd;
        NormaliseY(Y, Ys, Yd);

        changeLuminance(color, Y);

        Refl = Y;
        Kspec_refl = Ys / Y;
        Shin = 80;
    }
    else if (type == 2) // Type::Transparent
    {

        Trans = 1 / d;
        Refl = getY(Ks) + getY(Kd);
        float Y = Refl + Trans;
        NormaliseY(Y, Refl, Trans);

        changeLuminance(color, Y);

        N = Ns;
        Shin = 40;
    }
}

// Classes ---------------------------------------------------------------------

class Material
{
public:
    string Name;

    RGB diff; // Diffuse color      [0,1]
    RGB spec; // Specular color     [0,1]
    RGB amb;  // Ambient color      [0,1]
    float Tr; // Transparency       [0,1]

    unsigned int Shin; // Shininess                [0,128]
    float N;           // Refractive index         [1,2]

    Material()
    {
        Tr = 0;
        Shin = 0;
        N = 1;
    }

    Material(const Material &mat, string name)
    {
        this->diff = RGB(mat.diff);
        this->spec = RGB(mat.spec);
        this->amb = RGB(mat.amb);

        this->Tr = mat.Tr;
        this->Shin = mat.Shin;
        this->N = mat.N;
        this->Name = name;
    }

    // Show ------------------------------------------------------------------------------------------------
    void show()
    {

        // cout << "Render: ";
        if (!Name.empty())
            cout << Name << ":\n";

        cout << "diff:";
        diff.show();
        cout << ", ";

        cout << "spec:";
        spec.show();
        cout << ", ";

        cout << "abs:";
        amb.show();
        cout << ", ";

        cout << "Tr:" << Tr;
        cout << ", ";
        cout << "Shin:" << Shin;
        cout << ", ";
        cout << "n:" << N;
    }

    //  Show web -------------------------------------------------------------------------------

    string YtoStr(string name, RGB &Yrgb)
    {
        return "%22" + name + "%22:" + "[" + to_string(Yrgb.r) + "," + to_string(Yrgb.g) + "," + to_string(Yrgb.b) + "]";
    }

    string ValueToStr(string name, float val)
    {
        return "%22" + name + "%22:" + to_string(val);
    }

    string ValueToStr(string name, unsigned int &val)
    {
        return "%22" + name + "%22:" + to_string(val);
    }

    void show_web()
    {
        string url = "start https://vitalykuzmin.github.io/dialux_material/render.html?params={"; // http://l-i-n.ru/apps/Phong/index.html
        url += YtoStr("D", diff) + ",";
        url += YtoStr("S", spec) + ",";
        url += YtoStr("A", amb) + ",";
        url += ValueToStr("Tr", Tr) + ",";
        url += ValueToStr("Sh", Shin) + ",";
        url += ValueToStr("N", N);
        url += "}";
        system(url.c_str());
    }
};

class MaterialMaster
{
public:
    enum Type
    {
        Metallic,
        Painted,
        Transparent
    };

    // Properties
    RGB mColor;        // sRGB [0,1] (https://en.wikipedia.org/wiki/SRGB)
    float mRefl;       // Reflection factor        [0,0.9]
    float mKspec_refl; // Reflective coating       [0,1]
    float mTrans;      // Degree of transmission   [0,1]
    float mN;          // Refractive index         [1,2]
    Type mType;        // Material type

    float mKoeff; // Koefficient for transition between material types;

    Material mMaterial; // Render material

    // Constructors
    MaterialMaster()
    {
        mColor = RGB(0, 0, 0);
        mRefl = 0.0;
        mKspec_refl = 0.0;
        mTrans = 0.0;
        mN = 1.0;
        mType = Type::Metallic;
        mKoeff = -1;
    }

    // Init
    void init(float R, float G, float B, Type type, const float &r, const float &k)
    {
        mColor.Set(R / 255, G / 255, B / 255);
        SetProps(type, r, k);
    }

    void init(float R, float G, float B, Type type, const float &r, const float &t, const float &n)
    {
        mColor.Set(R / 255, G / 255, B / 255);
        SetProps(type, r, t, n);
    }

    Material createMaterial(string name)
    {
        return Material(mMaterial, name);
    }

    // Set
    void SetProps(Type type, const float &r, const float &k)
    {
        mType = type;
        if (type == Type::Painted || type == Type::Metallic)
        {
            mRefl = r;
            mKspec_refl = k;
        }
        else
        {
            throw invalid_argument("Invalid material type for this args");
        }
        update(2);
    }

    void SetProps(Type type, const float &r, const float &t, const float &N)
    {
        mType = type;
        if (type == Type::Transparent)
        {
            mRefl = r;
            mTrans = t;
            mN = N;
        }
        else
        {
            throw invalid_argument("Invalid material type for this args");
        }
        update(2);
    }

    void setType(MaterialMaster::Type newType)
    {
        if (mType == newType)
            return;
        mType = newType;
        update(1);
    }

    void setColor(const float &R, const float &G, const float &B)
    {
        mColor.Set(R / 255, G / 255, B / 255);
        mKoeff = -1;
        update(1);
    }

    void setRefl(float r)
    {
        if (mRefl == r)
            return;

        if (r < 0)
            r = 0.0;

        if (r > 0.9)
            r = 0.9;

        mRefl = r;
        if (mType == Type::Painted)
        {
            mKoeff = mRefl * mKspec_refl;
            if (mRefl == 0)
                mKspec_refl = 0;
        }
        else if (mType == Type::Transparent)
        {
            mKoeff = 1.0;
            if (mRefl > (1 - mTrans))
                mTrans = 1 - mRefl;
        }

        update(2);
    }

    void setKspec_refl(float k)
    {
        if (mKspec_refl == k)
            return;

        if (mType == Type::Transparent)
            return;

        if (k < 0)
            k = 0.0;

        if (k > 1.0)
            k = 1.0;

        mKspec_refl = k;

        if (mType == Type::Metallic)
        {
            mKoeff = mKspec_refl;
            update();
        }
        else if (mType == Type::Painted)
        {
            mKoeff = mKspec_refl * mRefl;
            update(2);
        }
    }

    bool setTrans(float t)
    {
        if (mTrans == t)
            return false;

        if (mType != Type::Transparent)
            return false;

        if (t < 0)
            t = 0.0;

        if (t > 1.0)
            t = 1.0;

        mTrans = t;

        mKoeff = 1.0;

        if (mTrans > (1 - mRefl))
        {
            mRefl = 1 - mTrans;
        }

        update(2);
    }

    void setN(float n)
    {
        if (mType != Type::Transparent)
            return;

        if (n < 1.0)
            n = 1.0;

        if (n > 2.0)
            n = 2.0;

        mN = n;

        update();
    }

    void setMaterial(const RGB &d, const RGB &s, const RGB &a, const float &tr, const float &shin, const float &n)
    {
        mMaterial.diff = d;
        mMaterial.spec = s;
        mMaterial.amb = a;
        mMaterial.Tr = tr;
        mMaterial.Shin = shin;
        mMaterial.N = n;
    }

    // GetY ----------------------------------------------------------------
    float getYfromRgb()
    {
        RGB Yrgb;
        // sRGB to Y
        Yrgb.r = toLinear(mColor.r) * K_R;
        Yrgb.g = toLinear(mColor.g) * K_G;
        Yrgb.b = toLinear(mColor.b) * K_B;

        float Y = 0.9 * Yrgb.sum(); // 10% идет на поглощение
        return Y;
    }

    float getYfromProps()
    {
        float Y;
        float K;
        switch (mType)
        {
        case Type::Metallic:
            Y = mRefl;
            break;
        case Type::Painted:
            K = mKspec_refl == 1 ? 0 : mKspec_refl * mRefl;
            Y = (mRefl - K) / (1 - K);
            break;
        case Type::Transparent:
            if (mRefl == 0)
            {
                Y = mTrans;
            }
            else
            {
                K = mTrans / mRefl;
                Y = mRefl * (1 + K);
            }
            break;
        }
        return Y;
    }

    // Update
    float updateRGB()
    {
        float Y = getYfromProps();
        changeLuminance(mColor, Y);
    }

    float updateProps()
    {
        float Y = getYfromRgb();
        float K = mKoeff;
        switch (mType)
        {
        case Type::Metallic:
            if (K != -1)
                mKspec_refl = mKoeff;
            mRefl = Y;
            break;
        case Type::Painted:

            if (K == -1)
                K = mKspec_refl * mRefl;

            mRefl = K + (1 - K) * Y;
            mKspec_refl = mRefl ? K / mRefl : 0;

            if (mKoeff != -1 && mRefl > 0.9)
                mRefl = 0.9;

            break;
        case Type::Transparent:

            if (mRefl == 0)
            {
                mTrans = Y;
            }
            else
            {
                if (K == -1)
                    K = mTrans / mRefl;
                mRefl = Y / (1 + K);
                mTrans = K * mRefl;
            }
            break;
        }
    }

    void update(unsigned int flag = 0)
    {
        if (flag == 1)
            updateProps();
        else if (flag == 2)
            updateRGB();

        render();
    }

    // Render --------------------------------------------------------
    void render()
    {
        RGB diff(mColor);
        RGB spec(mColor);
        RGB amb(mColor);

        float opacity;
        float Shin;
        float N;

        if (mType == Type::Metallic)
        {
            float Ys = mRefl * mKspec_refl;
            float Yd = mRefl * (1 - mKspec_refl);
            changeLuminance(diff, Yd);
            changeLuminance(diff, Ys);
            amb = spec;

            opacity = 0;
            Shin = 40; // примерно
            N = 1;
        }
        else if (mType == Type::Painted)
        {

            float Ys = mRefl * mKspec_refl;
            float Yd = mRefl * (1 - Ys);
            changeLuminance(diff, Yd);
            spec = RGB(0, 0, 0);
            changeLuminance(spec, Ys); // grayscale
            amb = spec;

            opacity = 0;
            Shin = 80; // примерно
            N = 1;
        }
        else if (mType == Type::Transparent)
        {
            float Y = mRefl + mTrans;
            diff.Set(0, 0, 0);
            changeLuminance(spec, mRefl);
            changeLuminance(amb, Y);

            opacity = mTrans / Y;
            Shin = 40; // примерно
            N = mN;
        }

        setMaterial(diff, spec, amb, opacity, Shin, N);
    };

    // Show
    void showRGB()
    {
        cout << "sRGB: ";
        mColor.show255();
    }

    void showProps()
    {
        cout << "Type: ";
        switch (mType)
        {
        case Type::Metallic:
            cout << "Metallic\n";
            cout << "Reflection factor: " << round(mRefl * 100) << "%\n";
            cout << "Reflective coating: " << round(mKspec_refl * 100) << "%";
            break;
        case Type::Painted:
            cout << "Painted\n";
            cout << "Reflection factor: " << round(mRefl * 100) << "%\n";
            cout << "Reflective coating: " << round(mKspec_refl * 100) << "%";
            break;
        case Type::Transparent:
            cout << "Transparent\n";
            cout << "Reflection factor: " << round(mRefl * 100) << "%\n";
            cout << "Degree of transmission: " << round(mTrans * 100) << "%\n";
            cout << "Refractive index: " << mN;
            break;
        }
    }

    void showMaterial()
    {
        mMaterial.show();
    }

    void show()
    {
        cout << "-------------------------------------------------------";
        cout << "\n";
        showRGB();
        cout << "\n";
        showProps();
        cout << "\n";
        showMaterial();
        cout << "\n\n";
    }

    // Show web
    void show_web()
    {
        mMaterial.show_web();
    }
};

class Materials
{
public:
    std::vector<Material> mMaterials;

    Materials() {}

    void add(Material mat)
    {
        mMaterials.push_back(mat);
    }

    void show()
    {
        for (int i = 0; i < mMaterials.size(); i++)
        {
            cout << i << ". ";
            mMaterials[i].show();
            cout << "\n";
        }
    }
};

// Main ----------------------------------------------------------------
int main()
{
    cout << std::fixed << std::setprecision(2);

    MaterialMaster master;
    master.init(137, 178, 88, MaterialMaster::Type::Metallic, 0.21, 0.13);
    master.show();

    Materials materials;
    // 0
    materials.add(master.createMaterial("Init material"));

    // 1
    master.setKspec_refl(0.2);
    materials.add(master.createMaterial("Change Kspec_refl to 0.2"));

    // 2
    master.setType(MaterialMaster::Type::Transparent);
    materials.add(master.createMaterial("Change type to Transparent"));

    // 3
    master.setTrans(0.5);
    materials.add(master.createMaterial("Change Trans to 0.5"));

    // 4
    master.setN(1.3);
    materials.add(master.createMaterial("Change N to 1.3"));

    // 5
    master.setColor(50, 50, 50);
    materials.add(master.createMaterial("Change color to (50,50,50)"));

    // 6
    master.setRefl(0.9);
    materials.add(master.createMaterial("Change Refl to 0.9"));

    // 7
    master.setType(MaterialMaster::Type::Painted);
    materials.add(master.createMaterial("Change type to Painted"));

    materials.show();
    master.show_web();
}