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

    RGB getNorm()
    {
        float Max = max();
        return RGB((r / Max), (g / Max), (b / Max));
    }
};

// Tools ------------------------------------------------------------------------------------

// Constants RGB -> Y (https://en.wikipedia.org/wiki/Relative_luminance)
const float K_R = 0.2126;
const float K_G = 0.7152;
const float K_B = 0.0722;

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

// Change rgb by Luminance
unsigned int rgb_max_i(float arr[], unsigned int no_index = -1)
{
    float max = 0;
    unsigned int index = 0;

    for (int i = 0; i < 3; i++) // arr.length
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

// classes ---------------------------------------------------------------------

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
        update(1);
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
        update(1);
    }

    void setType(MaterialMaster::Type newType)
    {
        if (mType == newType)
            return;
        mType = newType;
        update(1);
    }

    void setColor(const unsigned int &R, const unsigned int &G, const unsigned int &B)
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
        return;

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
    // master.init(137, 178, 88, MaterialMaster::Type::Transparent, 0.21, 0.13, 1);
    master.init(137, 178, 88, MaterialMaster::Type::Metallic, 0.21, 0.13);
    master.show();

    Materials materials;

    materials.add(master.createMaterial("Init material"));

    // "1. Change type: Painted"
    master.setType(MaterialMaster::Type::Painted);
    materials.add(master.createMaterial("Painted material"));

    // 2. Change type: Transparent"
    master.setType(MaterialMaster::Type::Transparent);
    materials.add(master.createMaterial("Transparent material"));

    materials.show();
}