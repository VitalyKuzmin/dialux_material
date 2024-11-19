// #include <iostream>
#include <iomanip>
#include <cmath>
#include <vector>
#include <string>

#include <assert.h>

#include "utils.h"
// #include "tools.h"

using namespace std;

#define _CALC_TYPE 0 // 0 - old; 1 - new; 2 - color mesh;

#define _epsilon 0.000001f

// Constants RGB -> Y (https://en.wikipedia.org/wiki/Relative_luminance)
const float K_R = 0.2126;
const float K_G = 0.7152;
const float K_B = 0.0722;

#define gre_max(a, b) (((a) > (b)) ? (a) : (b))

namespace type
{
    enum
    {
        Metallic = 0,
        Painted = 1,
        Transparent = 2,
        Last = 3,
    };
}

// Tools ------------------------------------------------------------------------------------

namespace utils
{

    // sRGB <-> lRGB (https://mina86.com/2019/srgb-xyz-conversion)
    float to_linear(float value)
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

    float from_linear(float value)
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

    color3f to_linear(const color3f color)
    {
        color3f linear;

        // Apply gamma correction to linear RGB values
        linear.r = to_linear(color.r);
        linear.g = to_linear(color.g);
        linear.b = to_linear(color.b);

        return linear;
    }

    color3f from_linear(const color3f color)
    {
        color3f srgb;

        srgb.r = from_linear(color.r);
        srgb.g = from_linear(color.g);
        srgb.b = from_linear(color.b);

        return srgb;
    }

    color3f linear_to_luminance(color3f color)
    {
        color3f luminance;

        luminance.r = color.r * K_R;
        luminance.g = color.g * K_G;
        luminance.b = color.b * K_B;

        return luminance;
    };

    color3f to_luminance(color3f color)
    {
        color3f luminance;

        luminance.r = utils::to_linear(color.r) * K_R;
        luminance.g = utils::to_linear(color.g) * K_G;
        luminance.b = utils::to_linear(color.b) * K_B;
        return luminance;
    };

}

namespace tools
{

    void clamp_value(float &value, float v = 1.0f)
    {
        if (value > v)
            value = v;
    }

    void clamp_color(color3f &color)
    {
        clamp_value(color.r);
        clamp_value(color.g);
        clamp_value(color.b);
    }

    // Change rgb by Luminance -----------------------------------------------------------------------
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

    float YfromRGB(float rgb[3])
    {

        return K_R * rgb[0] + K_G * rgb[1] + K_B * rgb[2];
    }

    color3f changeY(color3f c, const float Ynew)
    {

        float color[3] = {c.r, c.g, c.b};
        const float K[3] = {K_R, K_G, K_B};

        // for no division to null errors --------------------------------
        for (int j = 0; j < 3; j++)
        {
            color[j] = std::max(_epsilon, color[j]);
        }

        float sum = YfromRGB(color);
        float coeff = Ynew / sum;

        for (int j = 0; j < 3; j++)
        {
            color[j] *= coeff;
        }

        int index = rgb_max_i(color);
        if (color[index] > 1)
        {
            color[index] = 1;
            coeff = (Ynew - K[index]) / (YfromRGB(color) - K[index]); // color[index] = 1

            for (int j = 0; j < 3; j++)
            {
                if (j == index)
                    continue;
                color[j] *= coeff;
            }

            int index2 = rgb_max_i(color, index);
            if (color[index2] > 1)
            {
                color[index2] = 1;
                coeff = (Ynew - K[index] - K[index2]) / (YfromRGB(color) - K[index] - K[index2]);

                for (int j = 0; j < 3; j++)
                {
                    if (j == index || j == index2)
                        continue;
                    color[j] *= coeff;
                    clamp_value(color[j]);
                }
            }
        }

        return color3f(color[0], color[1], color[2]);
    }

    color3f changeLuminance(color3f rgb, const float &Ynew)
    {
        color3f color;

        color = utils::to_linear(rgb);

        color = changeY(color, Ynew);

        color = utils::from_linear(color);

        return color;
    };

    color3f partLuminance(color3f rgb, const float &Ypart, const float &Ysum)
    {
        color3f color;

        // k = Ypart / Ysum ;
        // Ypart = k * (Kr * rgb.r + Kg * rgb.g + Kb * rgb.b)
        // Kr * color.r = Kr * k * rgb.r
        // color.r = k * rgb.r

        float k = Ypart / Ysum;

        color.r = rgb.r * k;
        color.g = rgb.g * k;
        color.b = rgb.b * k;

        return color;
    };

}

using namespace tools;

class Material
{
public:
    color3f diffuse;      // диффузная часть (linear)
    color3f specular;     // зеркальная часть (linear)
    color3f transmission; // пропускание часть (linear)

    // Open GL Phong Shader Params -----------------------------------------------------------------------------
    color3f diffuse_sRGB;  // диффузная часть (sRGB)
    color3f specular_sRGB; // зеркальная часть (sRGB)
    color3f ambient_sRGB;  // светящаяся часть (sRGB)

    float Tr;          // Transparency             [0,1]
    unsigned int Shin; // Shininess                [0,128]
    float N;           // Refractive index         [1,2]

public:
    Material()
    {

        Tr = 0.0f;
        Shin = 80;
        N = 1.0f;
    }

    Material(const color3f &d, const color3f &s, const color3f &t, const color3f &a, const float &shin = 80, const float &tr = 0.0f, const float &n = 1.0f)
    {
        init(d, s, t, a, shin, tr, n);
    }

    void init(const color3f &d, const color3f &s, const color3f &t, const color3f &a, const float &shin = 80, const float &tr = 0.0f, const float &n = 1.0f)
    {
        // linear
        diffuse = d;
        specular = s;
        transmission = t;

        // Shader params
        diffuse_sRGB = utils::from_linear(d);
        specular_sRGB = utils::from_linear(s);
        ambient_sRGB = utils::from_linear(a);
        Tr = tr;
        Shin = shin;
        N = n;
    }
    bool isValid() const
    {
        if (((diffuse.r + specular.r + transmission.r) <= 1.0f) &&
            ((diffuse.g + specular.g + transmission.g) <= 1.0f) &&
            ((diffuse.b + specular.b + transmission.b) <= 1.0f) &&
            Tr >= 0.0f && Tr <= 1.0f && Shin >= 0 && Shin <= 128 && N >= 1.0f && N <= 2.0f)
        {
            return true;
        }

        return false;
    }

    void show()
    {
        char buff[256];
        sprintf_s(buff, 255, "diffuse: (%.6f, %.6f, %.6f)", diffuse.r, diffuse.g, diffuse.b);
        cout << buff << endl;
        sprintf_s(buff, 255, "specular: (%.6f, %.6f, %.6f)", specular.r, specular.g, specular.b);
        cout << buff << endl;
        sprintf_s(buff, 255, "transmission: (%.6f, %.6f, %.6f)", transmission.r, transmission.g, transmission.b);
        // cout << buff << endl;
        // sprintf_s(buff, 255, "Shader params:");
        // cout << buff << endl;
        // sprintf_s(buff, 255, "diffuse: (%.6f, %.6f, %.6f)", diffuse_sRGB.r, diffuse_sRGB.g, diffuse_sRGB.b);
        // cout << buff << endl;
        // sprintf_s(buff, 255, "specular: (%.6f, %.6f, %.6f)", specular_sRGB.r, specular_sRGB.g, specular_sRGB.b);
        // cout << buff << endl;
        // sprintf_s(buff, 255, "transmission: (%.6f, %.6f, %.6f)", ambient_sRGB.r, ambient_sRGB.g, ambient_sRGB.b);
        cout << buff << endl;
        sprintf_s(buff, 255, "Transparency: (%.6f)", Tr);
        cout << buff << endl;
        sprintf_s(buff, 255, "Shininess: (%1i)", Shin);
        cout << buff << endl;
        sprintf_s(buff, 255, "N: (%.6f)", N);
        cout << buff << endl;
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
            cout << i << ".\n";
            mMaterials[i].show();
            cout << "\n";
        }
    }
};

#if _CALC_TYPE == 0

class MaterialMaster
{
public:
    // Properties
    color3f mColor;    // sRGB [0,1] (https://en.wikipedia.org/wiki/SRGB)
    float mRefl;       // Reflection factor        [0,0.9]
    float mKspec_refl; // Reflective coating       [0,1]
    float mTrans;      // Degree of transmission   [0,1]
    float mN;          // Refractive index         [1,2]
    u32 mType;         // Material type

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
        mType = type::Metallic;
        mKoeff = -1;
    }

    // Init
    void init(float R, float G, float B, u32 type, const float &r, const float &k)
    {

        mColor.set(R / 255, G / 255, B / 255);
        SetProps(type, r, k);
    }

    void init(float R, float G, float B, u32 type, const float &r, const float &t, const float &n)
    {
        mColor.set(R / 255, G / 255, B / 255);
        SetProps(type, r, t, n);
    }

    Material getMaterial()
    {
        return mMaterial;
    }

    // Set
    void SetProps(u32 type, const float &r, const float &k)
    {
        mType = type;
        if (type == type::Painted || type == type::Metallic)
        {
            mRefl = r;
            mKspec_refl = k;
        }
        else
        {
            throw invalid_argument("Invalid material type for this args");
        }
        update(); // update(2)
    }

    void SetProps(u32 type, const float &r, const float &t, const float &N)
    {
        mType = type;
        if (type == type::Transparent)
        {
            mRefl = r;
            mTrans = t;
            mN = N;
        }
        else
        {
            throw invalid_argument("Invalid material type for this args");
        }
        update(); // update(2)
    }

    void setType(u32 newType)
    {
        if (mType == newType)
            return;
        mType = newType;
        update(1);
    }

    void setColor(color3f rgb)
    {
        mColor = rgb;
        clamp_color(mColor);
        mKoeff = -1;
        update(1);
    }

    void setColor(const float &R, const float &G, const float &B)
    {
        mColor.set(R / 255, G / 255, B / 255);
        clamp_color(mColor);
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
        if (mType == type::Painted)
        {
            mKoeff = mRefl * mKspec_refl;
            if (mRefl == 0)
                mKspec_refl = 0;
        }
        else if (mType == type::Transparent)
        {
            mKoeff = 1.0;
            if (mRefl > (1 - mTrans))
                mTrans = 1 - mRefl;
        }

        update(); // update(2)
    }

    void setReflectingCoating(float k)
    {
        if (mKspec_refl == k)
            return;

        if (mType == type::Transparent)
            return;

        if (k < 0)
            k = 0.0;

        if (k > 1.0)
            k = 1.0;

        mKspec_refl = k;

        if (mType == type::Metallic)
        {
            mKoeff = mKspec_refl;
            update();
        }
        else if (mType == type::Painted)
        {
            mKoeff = mKspec_refl * mRefl;
            update(); // update(2)
        }
    }

    bool setTrans(float t)
    {
        if (mTrans == t)
            return false;

        if (mType != type::Transparent)
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

        update(); // update(2)
    }

    void setN(float n)
    {
        if (mType != type::Transparent)
            return;

        if (n < 1.0)
            n = 1.0;

        if (n > 2.0)
            n = 2.0;

        mN = n;

        update();
    }

    void setMaterial(const color3f &d, const color3f &s, const color3f &t, const color3f &a, const float &tr, const float &shin, const float &n)
    {
        mMaterial.init(d, s, t, a, shin, tr, n);
    }

    // GetY ----------------------------------------------------------------
    float getYfromRgb()
    {
        color3f Yrgb;

        // sRGB to Y
        Yrgb = utils::to_luminance(mColor);

        float Y = 0.9 * Yrgb.sum(); // 10% идет на поглощение
        return Y;
    }

    float getYfromProps()
    {
        float Y;
        switch (mType)
        {
        case type::Metallic:
            Y = mRefl;
            break;
        case type::Painted:
            Y = mRefl * (1 - mKspec_refl) / (1 - mKspec_refl * mRefl);
            break;
        case type::Transparent:
            Y = mRefl + mTrans;
            break;
        }
        return Y;
    }

    // Update
    void updateRGB()
    {
        float Y = getYfromProps();
        mColor = changeLuminance(mColor, Y);
    }

    void updateProps()
    {
        float Y = getYfromRgb();
        float K = mKoeff;
        switch (mType)
        {
        case type::Metallic:
            if (K != -1)
                mKspec_refl = mKoeff;
            mRefl = Y;
            break;
        case type::Painted:

            if (K == -1)
                K = mKspec_refl * mRefl;

            mRefl = K + (1 - K) * Y;
            mKspec_refl = mRefl ? K / mRefl : 0;

            if (mKoeff != -1 && mRefl > 0.9)
                mRefl = 0.9;

            break;
        case type::Transparent:

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
        color3f diff;
        color3f spec;
        color3f amb;
        color3f trans;

        color3f Yrgb(mColor);

        float opacity;
        float Shin;
        float N;

        if (mType == type::Metallic)
        {

            float Ys = mRefl * mKspec_refl;
            float Yd = mRefl * (1 - mKspec_refl);

            float Ysum = mRefl;

            Yrgb = changeY(utils::to_linear(Yrgb), Ysum);

            diff = partLuminance(Yrgb, Yd, Ysum);
            spec = partLuminance(Yrgb, Ys, Ysum);
            amb = spec;
            trans.set(0, 0, 0);

            opacity = 0;
            Shin = 40; // примерно
            N = 1;
        }
        else if (mType == type::Painted)
        {

            float Ys = mRefl * mKspec_refl;
            float Yd = mRefl * (1 - mKspec_refl); // Ys

            float Ysum = mRefl;

            Yrgb = changeY(utils::to_linear(Yrgb), Ysum);

            diff = partLuminance(Yrgb, Yd, Ysum);
            spec.set(Ys, Ys, Ys); // grayscale
            amb = spec;
            trans.set(0, 0, 0);

            opacity = 0;
            Shin = 80; // примерно
            N = 1;
        }
        else if (mType == type::Transparent)
        {
            float Ysum = mRefl + mTrans;

            Yrgb = changeY(utils::to_linear(Yrgb), Ysum);

            diff.set(0, 0, 0);
            spec = partLuminance(Yrgb, mRefl, Ysum);
            trans = partLuminance(Yrgb, mTrans, Ysum);

            amb = Yrgb;

            opacity = mTrans / Ysum;
            Shin = 40; // примерно
            N = mN;
        }

        setMaterial(diff, spec, trans, amb, opacity, Shin, N);
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
        case type::Metallic:
            cout << "Metallic\n";
            cout << "Reflection factor: " << round(mRefl * 100) << "%\n";
            cout << "Reflective coating: " << round(mKspec_refl * 100) << "%";
            break;
        case type::Painted:
            cout << "Painted\n";
            cout << "Reflection factor: " << round(mRefl * 100) << "%\n";
            cout << "Reflective coating: " << round(mKspec_refl * 100) << "%";
            break;
        case type::Transparent:
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

private:
};

namespace tests
{

    void material_1_test(MaterialMaster master)
    {

        Material result = master.getMaterial();

        if (!result.isValid())
        {
            cout << "Error: -----------------------------------------------------------" << endl;
            master.show();
        }
        else
        {
            cout << "Valid: -----------------------------------------------------------" << endl;
            // master.show();
        }
    }

    void test_input_color(MaterialMaster master, float color_step = 0.1f)
    {

        cout << "test_input_color: -----------------------------------------------------------" << endl;

        Material mtl;

        u32 color_count = 1.0f / color_step;

        color3f color;

        // master.setColor(0, 0, 0);

        for (u32 i = 0; i < color_count; ++i)
        {
            color.r += color_step;
            color.b = 0.0f;
            color.g = 0.0f;

            for (u32 j = 0; j < color_count; ++j)
            {
                color.g += color_step;
                color.b = 0.0f;

                for (u32 k = 0; k < color_count; ++k)
                {
                    color.b += color_step;

                    master.setColor(color);

                    material_1_test(master);
                }
            }
        }
    }

    void test_type(MaterialMaster master, float color_step = 0.1f)
    {
        cout << "test_type: -----------------------------------------------------------" << endl;

        for (u32 t = type::Metallic; t < type::Last; ++t)
        {
            master.setType(t);
            material_1_test(master);
            test_input_color(master, color_step);
        }
    }

    void test_end(MaterialMaster master, float reflection_factor_step, float reflection_coating_step, float transperancy_step, float color_step = 0.1f)
    {
        cout << "test_end: -----------------------------------------------------------" << endl;

        u32 reflection_factor_count = 0.9f / reflection_factor_step;
        u32 reflection_coating_count = 1.0f / reflection_coating_step;
        u32 transperancy_count = 1.0f / transperancy_step;

        float reflection_factor(0);
        float reflection_coating(0);
        float transperancy(0);

        for (u32 i = 0; i < reflection_factor_count; ++i)
        {
            reflection_factor += reflection_factor_step;

            master.setRefl(reflection_factor);
            material_1_test(master);

            for (u32 j = 0; j < reflection_coating_count; ++j)
            {
                reflection_coating += reflection_coating_step;

                master.setReflectingCoating(reflection_coating);
                material_1_test(master);

                for (u32 k = 0; k < transperancy_count; ++k)
                {
                    transperancy += transperancy_step;

                    master.setTrans(transperancy);
                    material_1_test(master);

                    test_type(master, color_step);
                }
            }
        }
    }

}

// Main ----------------------------------------------------------------
int main_old()
{
    cout << std::fixed << std::setprecision(2);

    MaterialMaster master;
    master.init(137, 178, 88, type::Metallic, 0.21, 0.13);
    master.show();

    if (0)
    {
        Materials materials;
        // 0
        materials.add(master.getMaterial()); //"Init material"

        // 1
        master.setReflectingCoating(0.2);
        materials.add(master.getMaterial()); //"Change Reflecting Coating to 0.2"

        // 2
        master.setType(type::Transparent);
        materials.add(master.getMaterial()); //"Change type to Transparent"

        // 3
        master.setTrans(0.5);
        materials.add(master.getMaterial()); //"Change Trans to 0.5"

        // 4
        master.setN(1.3);
        materials.add(master.getMaterial()); //"Change N to 1.3"

        // 5
        master.setColor(50, 50, 50);
        materials.add(master.getMaterial()); //"Change color to (50,50,50)"

        // 6
        master.setRefl(0.9);
        materials.add(master.getMaterial()); //"Change Refl to 0.9"

        // 7
        master.setType(type::Painted);
        materials.add(master.getMaterial()); //"Change type to Painted"

        materials.show();
        system("pause");
    }

    if (1)
    {

        // tests::test_input_color(master, 0.2f);
        // tests::test_type(master, 0.2f);
        tests::test_end(master, 0.1f, 0.1f, 0.1f, 0.2f);

        system("pause");
    }

    return 0;
}

#elif _CALC_TYPE == 1
namespace lin
{

    namespace output
    {
        std::string toString(u32 type)
        {
            if (type == type::Metallic)
                return "Metallic";

            if (type == type::Painted)
                return "Painted";

            if (type == type::Transparent)
                return "Transparent";

            assert(0);
            return "Unknown";
        }

        void toLog(const color3f &color, u32 type, float reflection_factor, float reflection_coating, float transperancy)
        {
            char buff[256];
            sprintf_s(buff, 255, "Color: (%.6f, %.6f, %.6f), Type: %s, Reflection Factor: %.6f, Reflection Coating: %.6f, Transperancy: %.6f", color.r, color.g, color.b, toString(type).c_str(), reflection_factor, reflection_coating, transperancy);
            cout << buff << endl;
        }
    }

    float getYfromProps(u32 type, float Yrefl, float Kspec, float Ytrans)
    {
        float Y;
        switch (type)
        {
        case type::Metallic:
            Y = Yrefl;
            break;
        case type::Painted:
            Y = Yrefl * (1 - Kspec) / (1 - Kspec * Yrefl);
            break;
        case type::Transparent:
            Y = Yrefl + Ytrans;
            break;
        }
        return Y;
    }

    Material convert_material(const color3f &color, u32 type, float reflection_factor, float reflection_coating, float transperancy = 1.0f)
    {
        clamp_color(const_cast<color3f &>(color));
        clamp_value(reflection_factor, 0.9f);
        clamp_value(reflection_coating, 1.0f);
        clamp_value(transperancy, 1.0f);

        Material result;
        color3f sRGB = color;

        // IN
        float Yrefl = reflection_factor;    // Коэффициент отражения (Reflection factor) [0..0.9]
        float Kspec = reflection_coating;   // Отражение (Reflection coating) [0..1]
        float Ytrans = 1.0f - transperancy; // Коэффициент передачи (degree of transmission) [0..1]

        // vars
        float Yspec = 0.0f;
        float Ydiff = 0.0f;

        color3f diff;
        color3f spec;
        color3f amb;
        color3f trans;

        color3f Yrgb(sRGB);

        float opacity;
        float Shin;
        float N = 1.0f;

        if (type == type::Metallic)
        {

            float Ys = Yrefl * Kspec;
            float Yd = Yrefl * (1 - Kspec);

            float Ysum = Yrefl;

            Yrgb = changeY(utils::to_linear(Yrgb), Ysum);

            diff = partLuminance(Yrgb, Yd, Ysum);
            spec = partLuminance(Yrgb, Ys, Ysum);
            amb = spec;
            trans.set(0, 0, 0);

            opacity = 0;
            Shin = 40; // примерно
            N = 1;
        }
        else if (type == type::Painted)
        {

            float Ys = Yrefl * Kspec;
            float Yd = Yrefl * (1 - Kspec); // Ys

            float Ysum = Yrefl;

            Yrgb = changeY(utils::to_linear(Yrgb), Ysum);

            diff = partLuminance(Yrgb, Yd, Ysum);
            spec.set(Ys, Ys, Ys); // grayscale
            amb = spec;
            trans.set(0, 0, 0);

            opacity = 0;
            Shin = 80; // примерно
            N = 1;
        }
        else if (type == type::Transparent)
        {
            float Ysum = Yrefl + Ytrans;

            Yrgb = changeY(utils::to_linear(Yrgb), Ysum);

            diff.set(0, 0, 0);
            spec = partLuminance(Yrgb, Yrefl, Ysum);
            trans = partLuminance(Yrgb, Ytrans, Ysum);

            amb = Yrgb;

            opacity = Ytrans / Ysum;
            Shin = 40; // примерно
            N = N;
        }

        // ------------------------------------

        result.init(diff, spec, trans, amb);

        if (!result.isValid())
        {
            cout << "Error: -----------------------------------------------------------" << endl;
            output::toLog(color, type, reflection_factor, reflection_coating, transperancy);
            result.show();
        }
        else
        {
            cout << "Valid: -----------------------------------------------------------" << endl;
            output::toLog(color, type, reflection_factor, reflection_coating, transperancy);
            result.show();
        }

        return result;
    }

}

namespace tests
{

    void test_input_color(u32 mtltype, float reflection_factor, float reflection_coating, float transperancy, float color_step = 0.1f)
    {

        cout << "test_input_color: -----------------------------------------------------------" << endl;

        Material mtl;

        u32 color_count = 1.0f / color_step;

        color3f color;

        for (u32 i = 0; i < color_count; ++i)
        {
            color.r += color_step;

            for (u32 j = 0; j < color_count; ++j)
            {
                color.g += color_step;

                for (u32 k = 0; k < color_count; ++k)
                {
                    color.b += color_step;

                    mtl = lin::convert_material(color, mtltype, reflection_factor, reflection_coating, transperancy);
                }
            }
        }
    }

    void test_type(float reflection_factor, float reflection_coating, float transperancy, float color_step = 0.1f)
    {
        cout << "test_type: -----------------------------------------------------------" << endl;

        for (u32 t = type::Metallic; t < type::Last; ++t)
        {
            test_input_color(t, reflection_factor, reflection_coating, transperancy, color_step);
        }
    }

    void test_end(float reflection_factor_step, float reflection_coating_step, float transperancy_step, float color_step = 0.1f)
    {
        cout << "test_end: -----------------------------------------------------------" << endl;

        u32 reflection_factor_count = 0.9f / reflection_factor_step;
        u32 reflection_coating_count = 1.0f / reflection_coating_step;
        u32 transperancy_count = 1.0f / transperancy_step;

        float reflection_factor(0);
        float reflection_coating(0);
        float transperancy(0);

        for (u32 i = 0; i < reflection_factor_count; ++i)
        {
            reflection_factor += reflection_factor_step;

            for (u32 j = 0; j < reflection_coating_count; ++j)
            {
                reflection_coating += reflection_coating_step;

                for (u32 k = 0; k < transperancy_count; ++k)
                {
                    transperancy += transperancy_step;

                    test_type(reflection_factor, reflection_coating, transperancy, color_step);
                }
            }
        }
    }

}

// Main ----------------------------------------------------------------
int main()
{
    cout << std::fixed << std::setprecision(2);

    tests::test_input_color(type::Painted, 0.5f, 0.0f, 1.0f, 0.1f);
    // tests::test_type(0.5f, 0.0f, 1.0f, 0.1f);
    // tests::test_end(0.1f, 0.1f, 0.1f, 0.1f);

    system("pause");
    return 0;
};

#endif
