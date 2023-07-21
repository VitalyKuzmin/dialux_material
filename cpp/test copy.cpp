#include <iostream>
#include <iomanip>
#include <cmath>
#include <vector>
#include <string>

using namespace std;

/* Лакированный материал
----------------------------------------------------------------
    Система уравнений:
    Yspec = mProp.Kspec_refl * mProp.Yrefl = K
    Ydiff = (1 - Yspec) * Y
    Yrefl = Yspec + Ydiff
    Решение:
    Yrefl = K + ( 1 - K ) * Y
    Kspec_refl = K / Yrefl
*/

/* Прозрачный материал
----------------------------------------------------------------
    Система уравнений:
    Ytrans/Yrefl = mProp.Ytrans/mProp.Yrefl = K
    Yrefl + Ytrans = Y
    Решение:
    Yrefl = Y/(1+K)
    Ytrans = K*Yrefl
 */

// Constants RGB -> Y (https://en.wikipedia.org/wiki/Relative_luminance)
const float K_R = 0.2126;
const float K_G = 0.7152;
const float K_B = 0.0722;
//---------------------------------------------------------------------

// sRGB <-> lRGB (https://mina86.com/2019/srgb-xyz-conversion)
float gammaCompression(const float &linear)
{
    float nonLinear = (linear <= 0.00313066844250060782371)
                          ? 3294.6 * linear
                          : (269.025 * pow(linear, 5.0 / 12.0) - 14.025);
    return round(nonLinear); // || 0
}

float gammaExpansion(const float &value255)
{
    return (value255 <= 10) ? value255 / 3294.6 : pow((value255 + 14.025) / 269.025, 2.4);
}
//---------------------------------------------------------------------

class sRGB
{
public:
    unsigned int r; // red
    unsigned int g; // green
    unsigned int b; // blue

    unsigned int check(unsigned int color)
    {
        if (color < 0)
            color = 0;
        else if (color > 255)
            color = 255;

        return color;
    }

    void Set(const unsigned int &R, const unsigned int &G, const unsigned int &B)
    {

        r = check(R);
        g = check(G);
        b = check(B);
    }

    sRGB()
    {
        Set(0.0, 0.0, 0.0);
    }
    sRGB(const unsigned int &R, const unsigned int &G, const unsigned int &B)
    {
        Set(R, G, B);
    }

    void show()
    {
        cout << "(" << r << ", " << g << ", " << b << ")";
    }

    void clear()
    {
        Set(0.0, 0.0, 0.0);
    }

    sRGB *clone()
    {
        return new sRGB(r, g, b);
    }
};

class RGB
{
public:
    float r; // red
    float g; // green
    float b; // blue

    void Set(const float &R, const float &G, const float &B)
    {
        r = R;
        g = G;
        b = B;
    }

    RGB()
    {
        Set(0.0, 0.0, 0.0);
    }
    RGB(const float &R, const float &G, const float &B)
    {
        Set(R, G, B);
    }

    void show()
    {
        cout << "(" << r << ", " << g << ", " << b << ")";
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

    RGB *clone()
    {
        return new RGB(r, g, b);
    }
};

class Material
{
public:
    string Name;

    // Brightness
    RGB *Yd; // Diffuse      [0,1]
    RGB *Ys; // Specular     [0,1]
    RGB *Yt; // Transparency [0,1]

    // Phong Params ------------------------------------------------------------------
    const sRGB *Ya = new sRGB(); // Ambient rgb       [0,255]  - всегда (0,0,0)
    sRGB *Drgb;                  // Diffuse rgb       [0,255]
    sRGB *Srgb;                  // Specular rgb      [0,255]
    sRGB *Trgb;                  // Transparency rgb  [0,255]

    unsigned int Shin; // Shininess       [0,128]
    //  -------------------------------------------------------------------------------

    float calcRGB(RGB Yrgb, sRGB *sRgb)
    {
        RGB YRGB = Yrgb;
        YRGB.r /= K_R;
        YRGB.g /= K_G;
        YRGB.b /= K_B;

        sRgb->r = gammaCompression(YRGB.r);
        sRgb->g = gammaCompression(YRGB.g);
        sRgb->b = gammaCompression(YRGB.b);
    }

    // Calc
    float calc()
    {
        calcRGB(*Yd, Drgb); // Diffuse
        calcRGB(*Ys, Srgb); // Specular
        calcRGB(*Yt, Trgb); // Transparency
    }

    string YtoStr(sRGB *Yrgb)
    {
        return "[" + to_string(Yrgb->r) + "," + to_string(Yrgb->g) + "," + to_string(Yrgb->b) + "]";
    }

    string YtoStr(RGB *Yrgb)
    {
        return "[" + to_string(Yrgb->r) + "," + to_string(Yrgb->g) + "," + to_string(Yrgb->b) + "]";
    }

    void render()
    {

        string url = "start http://l-i-n.ru/apps/Phong/index.html?params={";
        url += "%22D%22:" + YtoStr(Drgb) + ","; // string("&");
        url += "%22S%22:" + YtoStr(Srgb) + ",";
        url += "%22T%22:" + to_string(Yt->sum()) + ",";
        url += "%22Sh%22:" + to_string(Shin);
        url += "}";

        system(url.c_str());
    }

    Material()
    {
        Yd = new RGB();
        Ys = new RGB();
        Yt = new RGB();

        Drgb = new sRGB();
        Srgb = new sRGB();
        Trgb = new sRGB();

        Shin = 0;
    }

    Material(RGB *yd, RGB *ys, RGB *yt, unsigned int shin, string name = "")
    {
        Yd = yd;
        Ys = ys;
        Yt = yt;
        Shin = shin;
        Name = name;
    }

    void show()
    {
        calc();

        cout << "Phong: ";
        if (!Name.empty())
            cout << Name << ":\n";

        cout << "diff:";
        Yd->show();
        cout << ", ";

        cout << "spec:";
        Ys->show();
        cout << ", ";

        cout << "trans:";
        Yt->show();
        cout << ", ";
        cout << "shin:" << Shin;
    }

    Material *clone(string name)
    {
        return new Material(Yd->clone(), Ys->clone(), Yt->clone(), Shin, name);
    }
};

class MaterialProperties
{
public:
    enum Type
    {
        Metallic,
        Painted,
        Transparent
    };

    // Dialux material properties
    float mRefl;       // Reflection factor        [0,0.9]
    float mKspec_refl; // Reflective coating       [0,1]
    float mTrans;      // Degree of transmission   [0,1]
    float mN;          // Refractive index         [1,2]
    Type mType;        // Material type

    // Constructors
    MaterialProperties()
    {
        mRefl = 0.0;
        mKspec_refl = 0.0;
        mTrans = 0.0;
        mN = 1.0;
        mType = Type::Painted;
    }

    MaterialProperties(Type type, const float &r, const float &k)
    {
        Set(type, r, k);
    }

    MaterialProperties(Type type, const float &r, const float &t, const float &N)
    {
        Set(type, r, t, N);
    }

    // Set
    void Set(Type type, const float &r, const float &k)
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
    }

    void Set(Type type, const float &r, const float &t, const float &N)
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
    }

    bool setRefl(float r)
    {
        if (mRefl == r)
            return false;

        if (r < 0)
            r = 0.0;

        if (r > 0.9)
            r = 0.9;

        mRefl = r;
        if (mType == Type::Painted && mRefl == 0)
        {
            mKspec_refl = 0;
        }
        else if (mType == Type::Transparent)
        {
            if (mRefl > (1 - mTrans))
            {
                mTrans = 1 - mRefl;
            }
        }
        return true;
    }

    bool setKs_d(float k)
    {
        if (mKspec_refl == k)
            return false;

        if (mType == Type::Transparent)
            return false;

        if (k < 0)
            k = 0.0;

        if (k > 1.0)
            k = 1.0;

        mKspec_refl = k;

        return true;
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

        if (mTrans > (1 - mRefl))
        {
            mRefl = 1 - mTrans;
        }

        return true;
    }

    bool setN(float n)
    {
        if (mType != Type::Transparent)
            return false;

        if (n < 1.0)
            n = 1.0;

        if (n > 2.0)
            n = 2.0;

        mN = n;
        return true;
    }

    // Brightness
    float getY()
    {
        float Y;
        float K;
        switch (mType)
        {
        case Type::Metallic:
            Y = mRefl;
            break;
        case Type::Painted:
            K = mKspec_refl * mRefl;
            Y = (mRefl - K) / (1 - K);
            break;
        case Type::Transparent:
            K = mTrans / mRefl;
            Y = mRefl * (1 + K);
            break;
        }
        return Y;
    }

    float setY(const float &Y)
    {
        float K;
        switch (mType)
        {
        case Type::Metallic:
            mRefl = Y;
            break;
        case Type::Painted:
            K = mKspec_refl * mRefl;
            mRefl = K + (1 - K) * Y;
            mKspec_refl = mRefl ? K / mRefl : 0;
            mRefl = Y;
            break;
        case Type::Transparent:
            K = mTrans / mRefl;
            mRefl = Y / (1 + K);
            mTrans = K * mRefl;
            break;
        }
    }

    // Show
    unsigned int format(const float &val)
    {
        return round(100 * val);
    }

    void show()
    {
        cout << "Type: ";
        switch (mType)
        {
        case Type::Metallic:
            cout << "Metallic\n";
            cout << "Reflection factor: " << format(mRefl) << "%\n";
            cout << "Reflective coating: " << format(mKspec_refl) << "%";
            break;
        case Type::Painted:
            cout << "Painted\n";
            cout << "Reflection factor: " << format(mRefl) << "%\n";
            cout << "Reflective coating: " << format(mKspec_refl) << "%";
            break;
        case Type::Transparent:
            cout << "Transparent\n";
            cout << "Reflection factor: " << format(mRefl) << "%\n";
            cout << "Degree of transmission: " << format(mTrans) << "%\n";
            cout << "Refractive index: " << mN;
            break;
        }
    }
};

class MaterialMaster
{
public:
    sRGB *mRGB;                 // sRGB (https://en.wikipedia.org/wiki/SRGB)
    MaterialProperties *mProps; // Dialux materials properties
    Material *mMaterial;        // Phong material

    // constructors
    MaterialMaster()
    {
        mRGB = new sRGB();
        mProps = new MaterialProperties();
        mMaterial = new Material();
    }

    // Tools
    float updateRGB()
    {
        // Y to sRGB
        RGB *Yrgb = new RGB();
        float Yold = getY(Yrgb);
        float Ynew = mProps->getY(); // 0.9; // 10% идет на поглощение
        float K = Ynew / Yold;

        mRGB->r = gammaCompression(Yrgb->r * K / K_R);
        mRGB->g = gammaCompression(Yrgb->g * K / K_G);
        mRGB->b = gammaCompression(Yrgb->b * K / K_B);
    }

    // Init
    void init(RGB rgb, MaterialProperties::Type type, const float &r, const float &k)
    {
        mRGB->Set(rgb.r, rgb.g, rgb.b);
        mProps->Set(type, r, k);
        updateRGB();
    }

    void init(RGB rgb, MaterialProperties::Type type, const float &r, const float &t, const float &n)
    {
        mRGB->Set(rgb.r, rgb.g, rgb.b);
        mProps->Set(type, r, t, n);
        updateRGB();
    }

    // Set
    void setRGB(const unsigned int &R, const unsigned int &G, const unsigned int &B)
    {

        mRGB->Set(R, G, B);
        mProps->setY(getY());
    }

    void setRefl(const float &r)
    {
        if (mProps->setRefl(r))
            updateRGB();
    }

    void setKs_d(const float &k)
    {
        if (mProps->setKs_d(k))
            updateRGB();
    }

    void setTrans(const float &t)
    {
        if (mProps->setTrans(t))
            updateRGB();
    }

    void setN(const float &t)
    {
        mProps->setN(t);
    }

    void setType(MaterialProperties::Type newType)
    {

        if (mProps->mType == newType)
        {
            return;
        }

        MaterialProperties::Type oldType = mProps->mType;
        mProps->mType = newType;
        // Painted to Metallic
        if (oldType == MaterialProperties::Type::Painted && newType == MaterialProperties::Type::Metallic)
        {
            /* Система уравнений ----------------------------------------------
                Kspec_refl(металл.) = Yspec(лак.) = Kspec_refl(лак.)*Yrefl(лак.)
                Refl(металл.) = Y
            ----------------------------------------------------------------- */
            mProps->mKspec_refl = mProps->mKspec_refl * mProps->mRefl;

            mProps->setY(getY());
            // mProps->mRefl = getY();
        }
        // Metallic to Painted
        else if (oldType == MaterialProperties::Metallic && newType == MaterialProperties::Painted)
        {

            /* Система уравнений ----------------------------------------------
             Yspec(лак.) = Kspec_refl(металл.) = K
             Refl(лак.) = K + Y*(1 - K) ;
             Kspec_refl(лак.) = K/Refl(лак.)
             ----------------------------------------------------------------- */
            float K = mProps->mKspec_refl;
            mProps->mRefl = K + getY() * (1 - K);
            mProps->mKspec_refl = K / mProps->mRefl;
        }
        // Transparent to Metallic
        else if (oldType == MaterialProperties::Transparent && newType == MaterialProperties::Metallic)
        {
            /* Система уравнений ----------------------------------------------
             Yspec(метал.) = Yrefl(прозрачн.);
             Yrefl(метал.) = Yrefl(прозрачн.)+Ytrans(прозрачн.)
             mKspec_refl(метал.) = Yspec(метал.)/Yrefl(метал.)
             ----------------------------------------------------------------- */
            //  float Yspec = mProps->mRefl;
            mProps->mRefl = mProps->mRefl + mProps->mTrans;
            // if(mProps->mRefl > )

            mProps->mKspec_refl = 1; // Yspec / mProps->mRefl;
        }
        // Metallic to Transparent
        else if (oldType == MaterialProperties::Metallic && newType == MaterialProperties::Transparent)
        {
            /* Система уравнений ----------------------------------------------
             Yrefl(прозрачн.) =  mKspec_refl(метал.)/Yrefl(метал.)
             Ytrans(прозрачн.) = Yrefl(метал.)-Yrefl(прозрачн.)
             ----------------------------------------------------------------- */
            /// float K = Ytrans / Yrefl;

            // mProps->mRefl = getY() / (1 + K);
            //  float Yspec = mProps->mKspec_refl * mProps->mRefl;
            //  mProps->mRefl = Yspec; // mProps->mKspec_refl / mProps->mRefl;
            //  mProps->mTrans = mProps->mRefl - Yspec;
        }
    }

    // Brightness
    float getY(RGB *Yrgb = new RGB())
    {
        // sRGB to Y
        Yrgb->r = gammaExpansion(mRGB->r) * K_R;
        Yrgb->g = gammaExpansion(mRGB->g) * K_G;
        Yrgb->b = gammaExpansion(mRGB->b) * K_B;

        float Y = 0.9 * Yrgb->sum(); // 10% идет на поглощение
        return Y;
    }

    // Calc
    void calcMaterial(const float &Kd, const float &Ks, const float &Kt, const float &shin)
    {

        RGB *Yrgb = new RGB();
        float Y = getY(Yrgb);

        RGB *Yd = mMaterial->Yd;
        RGB *Ys = mMaterial->Ys;
        RGB *Yt = mMaterial->Yt;

        if (Y > 0)
        {
            if (Kd > 0)
            { // diffuse
                Yd->r = Kd * (Yrgb->r / Y);
                Yd->g = Kd * (Yrgb->g / Y);
                Yd->b = Kd * (Yrgb->b / Y);
            }

            if (Ks > 0)
            { // specular
                Ys->r = Ks * (Yrgb->r / Y);
                Ys->g = Ks * (Yrgb->g / Y);
                Ys->b = Ks * (Yrgb->b / Y);
            }

            if (Kt > 0)
            { // transparency
                Yt->r = Kt * (Yrgb->r / Y);
                Yt->g = Kt * (Yrgb->g / Y);
                Yt->b = Kt * (Yrgb->b / Y);
            }
        }
        mMaterial->Shin = shin;
    }

    void calc()
    {
        // Phong params
        float Shin;
        float Kt;
        float Ks;
        float Kd;

        if (mProps->mType == MaterialProperties::Type::Metallic)
        {
            Shin = 15; // примерно
            Kt = 0;
            Ks = mProps->mRefl * mProps->mKspec_refl;
            Kd = mProps->mRefl * (1 - mProps->mKspec_refl);
        }
        else if (mProps->mType == MaterialProperties::Type::Painted)
        {
            Shin = 40; // примерно
            Kt = 0;
            Ks = mProps->mRefl * mProps->mKspec_refl;
            Kd = mProps->mRefl * (1 - mProps->mKspec_refl);
        }
        else if (mProps->mType == MaterialProperties::Type::Transparent)
        {
            Shin = 40; // примерно
            Kd = 0;
            Kt = mProps->mTrans;
            Ks = mProps->mRefl;
        }

        calcMaterial(Kd, Ks, Kt, Shin);
    };

    Material *createMaterial(string name)
    {
        calc();
        return mMaterial->clone(name);
    }

    // Show
    void showRGB()
    {
        cout << "sRGB: ";
        mRGB->show();
    }

    void showProps()
    {
        mProps->show();
    }

    void showCalc()
    {
        calc();
        mMaterial->show();
    }

    void show()
    {
        cout << "-------------------------------------------------------";
        cout << "\n";
        showRGB();
        cout << "\n";
        showProps();
        cout << "\n";
        showCalc();
        cout << "\n\n";
    }

    // Render
    void render()
    {
        mMaterial->render();
    }
};

class Materials
{
public:
    std::vector<Material *> mMaterials;

    Materials() {}

    void add(Material *mat)
    {
        mMaterials.push_back(mat);
    }

    void show()
    {
        for (int i = 0; i < mMaterials.size(); i++)
        {
            cout << i << ". ";
            mMaterials[i]->show();
            cout << "\n";
        }
    }
};

int main()
{

    cout << std::fixed << std::setprecision(2);

    MaterialMaster *master = new MaterialMaster();
    master->init(RGB(137, 178, 88), MaterialProperties::Type::Transparent, 0.21, 0.13, 1);
    master->show();
    master->render();

    // Materials *materials = new Materials();

    // // "1. Change type: Metallic";
    // master->setType(MaterialProperties::Type::Metallic);
    // master->show();

    // // "2. Change type: Painted";
    // master->setType(MaterialProperties::Type::Painted);
    // master->show();

    master->setType(MaterialProperties::Type::Metallic);
    master->show();

    master->setType(MaterialProperties::Type::Transparent);
    master->show();

    // master->setType(MaterialProperties::Type::Metallic);
    // master->show();

    // materials->add(master->createMaterial("Painted material"));
    //  // "2. Change type: Transparent";
    //  master->setType(MaterialProperties::Type::Transparent);
    //  master->show();
    //  materials->add(master->createMaterial("Transparent material"));

    // materials->show();
}