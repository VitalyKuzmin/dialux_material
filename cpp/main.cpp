// #include <iostream>
#include <iomanip>
#include <cmath>
#include <vector>
#include <string>

#include "tools.h"

using namespace std;

class DialuxMaterial
{

public:
    // Properties
    color3f mColor;     // sRGB [0,1] (https://en.wikipedia.org/wiki/SRGB)
    float mRefl;        // Reflection factor        [0,0.9]
    float mKspec_refl;  // Reflective coating       [0,1]
    float mTrans;       // Degree of transmission   [0,1]
    float mN;           // Refractive index         [1,2]
    float mShin;        // Shininness               [1,128]
    unsigned int mType; // Material type            [0 - Metallic;  1 - Painted; 2 - Transparent]
    DialuxMaterial(const color3f &color, const unsigned int &type, const float &refl,
                   const float &kspec_refl, const float &trans, const float &n, const float &shin)
    {
        mColor = color3f(color);
        mType = type;
        mRefl = refl;
        mKspec_refl = kspec_refl;
        mTrans = trans;
        mN = n;
        mShin = shin;
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

void RGBtoLinear(color3f &rgb)
{
    rgb.r = toLinear(rgb.r);
    rgb.g = toLinear(rgb.g);
    rgb.b = toLinear(rgb.b);
}

void RGBfromLinear(color3f &rgb)
{
    rgb.r = fromLinear(rgb.r);
    rgb.g = fromLinear(rgb.g);
    rgb.b = fromLinear(rgb.b);
}

void RGBfromLinear(color4f &rgb)
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

float getY(const color3f &rgb)
{
    return K_R * toLinear(rgb.r) + K_G * toLinear(rgb.g) + K_B * toLinear(rgb.b);
}

void toGray(color3f &rgb)
{
    float Y = fromLinear(getY(rgb));
    rgb = color3f(Y, Y, Y);
}

void changeY(float *color, const float &Ynew)
{
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
}

void changeLuminance(color3f &rgb, const float &Ynew)
{
    float color[3];

    // sRGB to linear RGB --------------------------------
    color[0] = toLinear(rgb.r);
    color[1] = toLinear(rgb.g);
    color[2] = toLinear(rgb.b);

    changeY(color, Ynew);

    // linear RGB to sRGB --------------------------------
    rgb.r = fromLinear(color[0]);
    rgb.g = fromLinear(color[1]);
    rgb.b = fromLinear(color[2]);
};

void changeLuminance2(color3f &rgb, const float &Ynew)
{
    float color[3];

    // sRGB to linear RGB --------------------------------
    color[0] = toLinear(rgb.r);
    color[1] = toLinear(rgb.g);
    color[2] = toLinear(rgb.b);

    changeY(color, Ynew);

    rgb.r = color[0];
    rgb.g = color[1];
    rgb.b = color[2];
};

void checkMaterialColor(color3f &rgb, const float &Y)
{
    float color[3];

    // sRGB to linear RGB --------------------------------
    color[0] = toLinear(rgb.r);
    color[1] = toLinear(rgb.g);
    color[2] = toLinear(rgb.b);

    float sum = K_R * color[0] + K_G * color[1] + K_B * color[2];
    if (sum != Y)
    {
        changeY(color, Y);
    }
};

// Получить цвет пикселя из прямой и отраженной составляющей луча падающий на определенный материал
color3f GetColor(color3f Direct, color3f Photon, float Ymax, color3f diffuse, color3f specular, color3f transmission)
{
    // Нормируем результаты расчета
    Direct /= Ymax;
    Photon /= Ymax;

    // sRGB -> linearRGB
    RGBtoLinear(diffuse);
    RGBtoLinear(specular);
    RGBtoLinear(transmission);

    // Производим расчет
    color3f Yrgb = Direct * specular + Photon * diffuse;
    color3f Yrgb_trans = (Direct + Photon) * transmission; // прозрачная часть если нужна

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

// Получить Dialux материал из OpenGL материала
DialuxMaterial OpenGLToDialuxMaterial(color3f Ka, color3f Kd, color3f Ks, color3f Ke, float Ns, float Ni, float d)
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

    float Refl, Kspec_refl, Trans, N, Shin;
    color3f color = Kd;

    // Trans = 0;
    // N = 1;
    // Shin = Ns;

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

    return DialuxMaterial(color, type, Refl, Kspec_refl, Trans, N, Shin);
}

// Classes ---------------------------------------------------------------------

class Material
{
public:
    string Name;

    color3f diff; // Diffuse color      [0,1]
    color3f spec; // Specular color     [0,1]
    color3f amb;  // Ambient color      [0,1]
    float Tr;     // Transparency       [0,1]

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
        this->diff = color3f(mat.diff);
        this->spec = color3f(mat.spec);
        this->amb = color3f(mat.amb);

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

    string YtoStr(string name, color3f &Yrgb)
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
    color3f mColor;    // sRGB [0,1] (https://en.wikipedia.org/wiki/SRGB)
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
        mColor = color3f(0, 0, 0);
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
        mColor.set(R / 255, G / 255, B / 255);
        SetProps(type, r, k);
    }

    void init(float R, float G, float B, Type type, const float &r, const float &t, const float &n)
    {
        mColor.set(R / 255, G / 255, B / 255);
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
        mColor.set(R / 255, G / 255, B / 255);
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

    void setMaterial(const color3f &d, const color3f &s, const color3f &a, const float &tr, const float &shin, const float &n)
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
        color3f Yrgb;
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
        color3f diff(mColor);
        color3f spec(mColor);
        color3f amb(mColor);

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
            spec = color3f(0, 0, 0);
            changeLuminance(spec, Ys); // grayscale
            amb = spec;

            opacity = 0;
            Shin = 80; // примерно
            N = 1;
        }
        else if (mType == Type::Transparent)
        {
            float Y = mRefl + mTrans;
            diff.set(0, 0, 0);
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

namespace
{
    static const float magic_1 = 0.008856451679036f;   // 216/24389
    static const float magic_2 = 903.296296296296296f; // 24389/27

    float log_a_to_base_b(const float &a, const float &b)
    {
        return log2(a) / log2(b);
    }

    float Y2LS(float Y) { return (Y <= magic_1) ? Y * magic_2 : std::pow(Y, 1.0f / 3.0f) * 116.0f - 16.0f; }

    float Y2Log(float Y, float b = 8.0f) { return log_a_to_base_b(Y, b); }

    void convert(color4f &color, float b = 0.0f)
    {
        float Y = (color.r + color.g + color.b) / 3.0f;
        float Ynew;
        if (Y > 0)
        {
            if (b > 0)
            {
                Ynew = Y2Log(Y, b); // логарифмический контраст
            }
            else
            {
                Ynew = Y2LS(Y) * 0.01f; // фииологический контраст
            }

            color *= Ynew / Y;
            color.a = 1.0f;
        }
    }
}

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

public:
    void prepareColors()
    {
        const color4f &color = color;
        const color4f lamp_color(0.0f, 0.0f, 0.0f); //  цвет источника

        float Y;
        float K;

        if (type == 0) // Metallic
        {
            Y = Refl;
            changeLuminance2(cs, Y);
        }
        else if (type == 1) //  Painted
        {
            float Ys = Refl * Kspec_refl;
            float Yd = Refl * (1 - Ys);
            cl = color3f(lamp_color);
            cd = color3f(color);
            changeLuminance2(cd, Yd);
            changeLuminance2(cl, Ys);
        }
        else if (type == 2) //  Transparent
        {
            if (Refl == 0)
            {
                Y = Trans;
            }
            else
            {
                K = Trans / Refl;
                Y = Refl * (1 + K);
            }
            changeLuminance2(cs, Y);
        }
    }
};

void color_normalize(color4f &c, float nmv)
{
    nmv /= 3;
    float mx = gre_max(c.r, gre_max(c.g, c.b));
    if (mx > nmv)
    {
        c.r /= mx;
        c.g /= mx;
        c.b /= mx;
    }
    else
    {
        c.r /= nmv;
        c.g /= nmv;
        c.b /= nmv;
    }
}

void applyMaterialToPoint(const material &mtl, color4f &c, color3f &vl, color3f &vd)
{
    const color4f &color = mtl.color;
    const u32 type = mtl.type;

    if (type == 1) //  Painted
    {
        c = vd * mtl.cd + vl * mtl.cl;
    }
    else // Metallic && Transparent
    {
        c = (vd + vl) * mtl.cs;
    }
}

bool PuryaMesh::normalizeColor2(float mvt)
{
    if (m_calc_points_count)
    {
        // mvt - максимальное значение (user), после которого считаем все белым (или макс. насыщенность)...
        float sum(0.0f);
        float mx(0.0f);
        float Y(0);
        material &mtl = m_calc_mesh->getMaterial();
        mtl.prepareColors(); // Подготовить цвета материала
        vertex *v(nullptr);
        color3f ic;
        for (u32 i = 0; i < m_calc_points_count; ++i)
        {
            v = (*m_calc_mesh)[i];
            color4f &c = v->c;   // цвет который необходимо расчитать для цветного режима
            color4f &cg = v->cg; // цвет который необходимо расчитать для градаций серого режима

            color3f &vl = v->vl;  // прямая составляющая освещенности
            color3f &vd = v->vd;  // диффузная состовляющая освещенности
            color3f vs = vd + vl; // суммарная освещенность

            // Расчет для градаций серого (тут материал не учитываем, также как в псевдоцветах )
            Y = (vs.r + vs.g + vs.b) / 3.0f;
            cg.set(Y, Y, Y);
            color_normalize(cg, mvt); // Нормируем
            convert(cg, 8.0f);        // Применяем логарифмический контраст
            RGBfromLinear(cg);        // Преобразуем в sRGB цвет

            // Расчет для обычного отображения
            applyMaterialToPoint(mtl, c, vl, vd); // Применяем материал (реализуем последнее отражение в экран)
            color_normalize(c, mvt);              // Нормируем
            convert(c);                           // Применяем физиологический контраст
            RGBfromLinear(c);                     // Преобразуем в sRGB цвет
        }
        return true;
    }
    return false;
}