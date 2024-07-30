// #include <iostream>
#include <iomanip>
#include <cmath>
#include <vector>
#include <string>

#include <assert.h>

#include "utils.h"
//#include "tools.h"

using namespace std;



#define _CALC_TYPE 1 // 0 - old; 1 - new; 2 - color mesh;

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


namespace tools {


    void clamp_value(float& value, float v = 1.0f)
    {
        if (value > v) value = v;
    }

    void clamp_color(color3f& color)
    {
        clamp_value(color.r);
        clamp_value(color.g);
        clamp_value(color.b);
    }


    // Change rgb by Luminance -----------------------------------------------------------------------
    unsigned int rgb_max_i(float* arr, unsigned int no_index = -1)
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



    float YfromRGB(float rgb[3]) {

        return K_R * rgb[0] + K_G * rgb[1] + K_B * rgb[2];
    }

    color3f changeY(color3f c, const float Ynew) {

        float color[3] = { c.r, c.g, c.b };
        const float K[3] = { K_R, K_G, K_B };

        // for no division to null errors --------------------------------
        for (int j = 0; j < 3; j++)
        {
            color[j] = std::max(_epsilon, color[j]);
        }


        float sum = YfromRGB(color);
        float coeff = Ynew / sum;

        for (int j = 0; j < 3; j++) {
            color[j] *= coeff;
        }


        int index = rgb_max_i(color);
        if (color[index] > 1) {
            color[index] = 1;
            coeff = (Ynew - K[index]) / (YfromRGB(color) - K[index]);  //color[index] = 1

            for (int j = 0; j < 3; j++) {
                if (j == index) continue;
                color[j] *= coeff;
            }


            int index2 = rgb_max_i(color, index);
            if (color[index2] > 1) {
                color[index2] = 1;
                coeff = (Ynew - K[index] - K[index2]) / (YfromRGB(color) - K[index] - K[index2]);

                for (int j = 0; j < 3; j++) {
                    if (j == index || j == index2) continue;
                    color[j] *= coeff;
                    clamp_value(color[j]);
                }

            }

        }

        return color3f(color[0], color[1], color[2]);
    }

    color3f changeLuminance(color3f rgb, const float& Ynew)
    {
        color3f color;

        color = utils::to_linear(rgb);

        color = changeY(color, Ynew);

        color = utils::from_linear(color);

        return color;
    };





}


using namespace tools;

class Material
{
public:
    color3f diffuse;               // диффузная часть (Y)
    color3f specular;              // зеркальная часть (Y)
    color3f transmission;          // пропускание часть (Y)

    color3f diffuse_linear;        // диффузная часть (linear)
    color3f specular_linear;       // зеркальная часть (linear)
    color3f transmission_linear;   // пропускание часть (linear)

    // Open GL Phong Shader Params -----------------------------------------------------------------------------
    color3f diffuse_sRGB;        // диффузная часть (sRGB)
    color3f specular_sRGB;       // зеркальная часть (sRGB)
    color3f ambient_sRGB;        // светящаяся часть (sRGB)


    float Tr;          // Transparency             [0,1]
    unsigned int Shin; // Shininess                [0,128]
    float N;           // Refractive index         [1,2]

public:

    Material() {

        Tr = 0.0f;
        Shin = 80;
        N = 1.0f;
    }

    Material(const color3f& d, const color3f& s, const color3f& t, const color3f& a, const float& shin = 80, const float& tr = 0.0f, const float& n = 1.0f) {
        init(d,s,t,a,shin,tr,n);
    }
     
    void init(const color3f& d, const color3f& s, const color3f& t, const color3f& a, const float& shin = 80, const float& tr=0.0f, const float& n = 1.0f)
    {
        // Shader params
        diffuse_sRGB = d;
        specular_sRGB = s;
        ambient_sRGB = a;
        Tr = tr;
        Shin = shin;
        N = n;

        // linear
        diffuse_linear = utils::to_linear(d);
        specular_linear = utils::to_linear(s);
        transmission_linear = utils::to_linear(t);

        // Y
        diffuse = utils::linear_to_luminance(diffuse_linear);
        specular = utils::linear_to_luminance(specular_linear);
        transmission = utils::linear_to_luminance(transmission_linear);

    }
    bool isValid() const
    {
        if (  ((diffuse.r + specular.r + transmission.r) <= 1.0f) && 
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
        //cout << buff << endl;
        //sprintf_s(buff, 255, "Shader params:");
        //cout << buff << endl;
        //sprintf_s(buff, 255, "diffuse: (%.6f, %.6f, %.6f)", diffuse_sRGB.r, diffuse_sRGB.g, diffuse_sRGB.b);
        //cout << buff << endl;
        //sprintf_s(buff, 255, "specular: (%.6f, %.6f, %.6f)", specular_sRGB.r, specular_sRGB.g, specular_sRGB.b);
        //cout << buff << endl;
        //sprintf_s(buff, 255, "transmission: (%.6f, %.6f, %.6f)", ambient_sRGB.r, ambient_sRGB.g, ambient_sRGB.b);
        cout << buff << endl;
        sprintf_s(buff, 255, "Transparency: (%.6f)", Tr);
        cout << buff << endl;
        sprintf_s(buff, 255, "Shininess: (%1i)", Shin);
        cout << buff << endl;
        sprintf_s(buff, 255, "N: (%.6f, %.6f, %.6f)", transmission.r, transmission.g, transmission.b);
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
    u32 mType;        // Material type

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
    void init(float R, float G, float B, u32 type, const float& r, const float& k)
    {
        
        mColor.set(R / 255, G / 255, B / 255);
        SetProps(type, r, k);
        
    }

    void init(float R, float G, float B, u32 type, const float& r, const float& t, const float& n)
    {
        mColor.set(R / 255, G / 255, B / 255);
        SetProps(type, r, t, n);
    }

    Material getMaterial()
    {
        return mMaterial;
    }

    // Set
    void SetProps(u32 type, const float& r, const float& k)
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
        update(2);
    }

    void SetProps(u32 type, const float& r, const float& t, const float& N)
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
        update(2);
    }

    void setType(u32 newType)
    {
        if (mType == newType)
            return;
        mType = newType;
        update(1);
    }

    void setColor(const float& R, const float& G, const float& B)
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

        update(2);
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
            update(2);
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

        update(2);
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

    void setMaterial(const color3f& d, const color3f& s, const color3f& t, const color3f& a, const float& tr, const float& shin, const float& n)
    {
        mMaterial.init(d,s,t,a,tr,shin,n);
    }

    // GetY ----------------------------------------------------------------
    float getYfromRgb()
    {
        color3f Yrgb;

        // sRGB to Y
        Yrgb = to_luminance(mColor);

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
        color3f diff(mColor);
        color3f spec(mColor);
        color3f amb(mColor);
        color3f trans(mColor);

        float opacity;
        float Shin;
        float N;

        if (mType == type::Metallic)
        {
            float Ys = mRefl * mKspec_refl;
            float Yd = mRefl * (1 - mKspec_refl);

            diff = changeLuminance(diff, Yd);
            spec = changeLuminance(spec, Ys);
            amb = spec;
            trans.set(0, 0, 0);

            opacity = 0;
            Shin = 40; // примерно
            N = 1;
        }
        else if (mType == type::Painted)
        {

            float Ys = mRefl * mKspec_refl;
            float Yd = mRefl * (1 - Ys);

            diff = changeLuminance(diff, Yd);
            spec = changeLuminance(color3f(0,0,0), Ys); // grayscale
            amb = spec;
            trans.set(0, 0, 0);

            opacity = 0;
            Shin = 80; // примерно
            N = 1;
        }
        else if (mType == type::Transparent)
        {
            float Y = mRefl + mTrans;
            diff.set(0, 0, 0);
            spec = changeLuminance(spec, mRefl);
            amb = changeLuminance(amb, Y);
            trans = changeLuminance(trans, mTrans);

            opacity = mTrans / Y;
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


// Main ----------------------------------------------------------------
int main()
{
    cout << std::fixed << std::setprecision(2);

        MaterialMaster master;
        master.init(137, 178, 88, type::Metallic, 0.21, 0.13);
        master.show();

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

        void toLog(const color3f& color, u32 type, float reflection_factor, float reflection_coating, float transperancy)
        {
            char buff[256];
            sprintf_s(buff, 255, "Color: (%.6f, %.6f, %.6f), Type: %s, Reflection Factor: %.6f, Reflection Coating: %.6f, Transperancy: %.6f", color.r, color.g, color.b, toString(type).c_str(), reflection_factor, reflection_coating, transperancy);
            cout << buff << endl;
        }
    }



    float getYfromProps(u32 type,float Yrefl,float Kspec, float Ytrans)
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


    Material convert_material(const color3f& color, u32 type, float reflection_factor, float reflection_coating, float transperancy = 1.0f)
    {
        clamp_color(const_cast<color3f&>(color));
        clamp_value(reflection_factor, 0.9f);
        clamp_value(reflection_coating, 1.0f);
        clamp_value(transperancy, 1.0f);


        Material result;
        color3f sRGB = color;

        // IN
        float Yrefl = reflection_factor;                                           // Коэффициент отражения (Reflection factor) [0..0.9]
        float Kspec = reflection_coating;                                          // Отражение (Reflection coating) [0..1]
        float Ytrans = 1.0f - transperancy;                                        // Коэффициент передачи (degree of transmission) [0..1]


        // Check sRGB
        //float Y = getYfromProps(type, Yrefl, Kspec, Ytrans);
        //sRGB = changeLuminance(sRGB, Y);


        // vars
        float Yspec = 0.0f;
        float Ydiff = 0.0f;

        color3f diff(color);
        color3f spec(color);
        color3f amb(color);
        color3f trans(color);

        float opacity;
        float Shin;
        float N = 1.0f;


        if (type == type::Metallic)
        {
            float Ys = Yrefl * Kspec;
            float Yd = Yrefl * (1 - Kspec);

            diff = changeLuminance(diff, Yd);
            spec = changeLuminance(spec, Ys);
            amb = spec;
            trans.set(0, 0, 0);

            opacity = 0;
            Shin = 40; // примерно
            N = 1;
        }
        else if (type == type::Painted)
        {

            float Ys = Yrefl * Kspec;
            float Yd = Yrefl * (1 - Ys);

            diff = changeLuminance(diff, Yd);
            spec = changeLuminance(color3f(0, 0, 0), Ys); // grayscale
            amb = spec;
            trans.set(0, 0, 0);

            opacity = 0;
            Shin = 80; // примерно
            N = 1;
        }
        else if (type == type::Transparent)
        {
            float Y = Yrefl + Ytrans;
            diff.set(0, 0, 0);
            spec = changeLuminance(spec, Yrefl);
            amb = changeLuminance(amb, Y);
            trans = changeLuminance(trans, Ytrans);

            opacity = Ytrans / Y;
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
        else {
            cout << "Valid: -----------------------------------------------------------" << endl;
            output::toLog(color, type, reflection_factor, reflection_coating, transperancy);
            result.show();
        }

        return result;
    }



    Material convert_material_old(const color3f& color, u32 type, float reflection_factor, float reflection_coating, float transperancy = 1.0f)
    {
        clamp_color(const_cast<color3f&>(color));
        clamp_value(reflection_factor, 0.9f);
        clamp_value(reflection_coating, 1.0f);
        clamp_value(transperancy, 1.0f);

        Material result;
        color3f sRGB = color;

        // gamma-expanded values (sometimes called "linear values" or "linear-light values")
        color3f linear = utils::to_linear(color);

        const float Kr = 0.2126f;
        const float Kb = 0.0722f;
        const float Kg = 1.0f - Kr - Kb;

        color3f Y(Kr * linear.r, Kg * linear.g, Kb * linear.b);
        float Ysum = (Y.r + Y.g + Y.b);

        Ysum = std::max(_epsilon, Ysum);

        // IN
        float Yrefl = reflection_factor < 0.9f ? reflection_factor : 0.9f;         // Коэффициент отражения (Reflection factor) [0..0.9]
        float Kspec = reflection_coating;                                          // Отражение (Reflection coating) [0..1]
        float Ytrans = 1.0f - transperancy;                                        // Коэффициент передачи (degree of transmission) [0..1]

        // vars
        float Yspec = 0.0f;
        float Ydiff = 0.0f;

        // out
        color3f specular_Y;
        color3f specular_linear;
        color3f specular_sRGB;

        color3f transmission_Y;
        color3f transmission_linear;
        color3f transmission_sRGB;

        color3f diffuse_Y;
        color3f diffuse_linear;
        color3f diffuse_sRGB;

        color3f reflection_Y;
        color3f reflection_linear;
        color3f reflection_sRGB;



        if (type == type::Transparent)
        {
            Yspec = Yrefl;
            Ydiff = 0.0f;

            // specular (upper-half)
            specular_Y.r = Y.r * Yspec / Ysum;
            specular_Y.g = Y.g * Yspec / Ysum;
            specular_Y.b = Y.b * Yspec / Ysum;

            // specular (bottom-half)
            specular_linear.r = specular_Y.r / Kr;
            specular_linear.g = specular_Y.g / Kg;
            specular_linear.b = specular_Y.b / Kb;

            specular_sRGB = utils::from_linear(specular_linear);

            // transmission
            transmission_Y.r = Y.r * Ytrans / Ysum;
            transmission_Y.g = Y.g * Ytrans / Ysum;
            transmission_Y.b = Y.b * Ytrans / Ysum;

            transmission_linear.r = transmission_Y.r / Kr;
            transmission_linear.g = transmission_Y.g / Kg;
            transmission_linear.b = transmission_Y.b / Kb;

            transmission_sRGB = utils::from_linear(transmission_linear);
        }
        else
        {
            if (type == type::Metallic)
            {
                Yspec = Yrefl * Kspec;
                Ytrans = 0.0f;                   // не прозрачный
                Ydiff = Yrefl * (1.0f - Kspec);

                // specular (upper-half)
                specular_Y.r = Y.r * Yspec / Ysum;
                specular_Y.g = Y.g * Yspec / Ysum;
                specular_Y.b = Y.b * Yspec / Ysum;
            }
            else
            {
                assert(type == type::Painted);

                Yspec = Yrefl * Kspec;
                Ytrans = 0.0f;                   // не прозрачный
                Ydiff = Yrefl * (1.0f - Kspec);

                // specular (upper-half)
                specular_Y.r = Kr * Yspec;
                specular_Y.g = Kg * Yspec;
                specular_Y.b = Kb * Yspec;
            }

            // specular (bottom-half)
            specular_linear.r = specular_Y.r / Kr;
            specular_linear.g = specular_Y.g / Kg;
            specular_linear.b = specular_Y.b / Kb;

            // norm

            specular_sRGB = utils::from_linear(specular_linear);

            // diffuse
            diffuse_Y.r = Y.r * Ydiff / Ysum;
            diffuse_Y.g = Y.g * Ydiff / Ysum;
            diffuse_Y.b = Y.b * Ydiff / Ysum;

            diffuse_linear.r = diffuse_Y.r / Kr;
            diffuse_linear.g = diffuse_Y.g / Kg;
            diffuse_linear.b = diffuse_Y.b / Kb;

            diffuse_sRGB = utils::from_linear(diffuse_linear);

            // reflection
            reflection_Y.r = specular_Y.r + diffuse_Y.r;
            reflection_Y.g = specular_Y.g + diffuse_Y.g;
            reflection_Y.b = specular_Y.b + diffuse_Y.b;

            reflection_linear.r = reflection_Y.r / Kr;
            reflection_linear.g = reflection_Y.g / Kg;
            reflection_linear.b = reflection_Y.b / Kb;

            reflection_sRGB = utils::from_linear(reflection_linear);
        }

        result.diffuse = diffuse_linear;
        result.specular = specular_linear;
        result.transmission = transmission_linear;

        result.diffuse_sRGB = diffuse_sRGB;
        result.specular_sRGB = specular_sRGB;
        result.ambient_sRGB = transmission_sRGB;

        if (!result.isValid())
        {
            cout << "Error: -----------------------------------------------------------" << endl;
            output::toLog(color, type, reflection_factor, reflection_coating, transperancy);
            result.show();
        }

        return result;
    }





}


namespace tests {

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
    //tests::test_type(0.5f, 0.0f, 1.0f, 0.1f);
    //tests::test_end(0.1f, 0.1f, 0.1f, 0.1f);

    system("pause");
    return 0;
};

#elif _CALC_TYPE == 2


namespace
{
    static const float magic_1 = 0.008856451679036f;   // 216/24389
    static const float magic_2 = 903.296296296296296f; // 24389/27



    void color_check(color4f& c)
    {
        float mx = gre_max(c.r, gre_max(c.g, c.b));
        if (mx > 1)
        {
            c.r /= mx;
            c.g /= mx;
            c.b /= mx;
        }
    }

    void color_normalize(color4f& c, float nmv)
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

    float log_a_to_base_b(const float& a, const float& b)
    {
        return log2(a) / log2(b);
    }

    float Y2LS(float Y) { return (Y <= magic_1) ? Y * magic_2 : std::pow(Y, 1.0f / 3.0f) * 116.0f - 16.0f; }

    float Y2Log(float Y, float b = 8.0f) { return log_a_to_base_b(Y, b) / 3.0f + 1.0f; }

    void convert(color4f& color, float b = 0.0f)
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
                Ynew = Y2LS(Y) * 0.01f; // физиологический контраст
            }

            color *= (Ynew / Y);

            color_check(color);
        }
    }

}



struct DialuxMaterial
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
    float opacity = 0.0f;

    DialuxMaterial() {

    };

    DialuxMaterial(const DialuxMaterial& mat)
    {
        set(mat.color, mat.type, mat.Refl, mat.Kspec_refl, mat.Trans, mat.N, mat.Shin);
    };

    DialuxMaterial(const color3f& c, const unsigned int& t, const float& refl,
        const float& kspec_refl, const float& trans, const float& n, const float& shin)
    {
        set(c, t, refl, kspec_refl, trans, n, shin);
    }

    void set(const color3f& c, const unsigned int& t, const float& refl,
        const float& kspec_refl, const float& trans, const float& n, const float& shin)
    {
        color = color3f(c);
        type = t;
        Refl = refl;
        Kspec_refl = kspec_refl;
        Trans = trans;
        N = n;
        Shin = shin;
    }

    void show()
    {

        cout << "Material--------------\n";
        cout << "color: ";
        color.show255();
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

        cout << "opacity: ";
        cout << opacity;
        cout << "\n";
        cout << "\n";
    }

    void checkMaterialColor(const color4f& c, color3f& rgb, const float& Y)
    {
        float sum = K_R * rgb.r + K_G * rgb.g + K_B * rgb.b;
        if (abs(sum - Y) > 0.001)
        {
            rgb = changeY(utils::to_linear(c), Y);
        }
    }

    void prepareColors()
    {

        // const color4f lamp_color(1.0f, 1.0f, 1.0f); //  цвет источника
        float Y;
        if (type == 0) // Metallic
        {
            // float Ys = Refl * Kspec_refl;
            // float Yd = Refl * (1 - Kspec_refl);
            Y = Refl;
            checkMaterialColor(color, cs, Y);
        }
        else if (type == 1) // Painted
        {

            // float Ys = Refl * Kspec_refl;
            // float Yd = Refl * (1 - Ys);
            Y = Refl * (1 - Kspec_refl) / (1 - Kspec_refl * Refl);
            checkMaterialColor(color, cs, Y);
        }
        else if (type == 2) // Transparent
        {
            Y = Refl + Trans;
            checkMaterialColor(color, cs, Y);
            opacity = Trans / Y;
        }
    }

    void NormaliseY(float& Ysum, float& Y1, float& Y2)
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


    float getY(const color3f& rgb)
    {
        return K_R * utils::to_linear(rgb.r) + K_G * utils::to_linear(rgb.g) + K_B * utils::to_linear(rgb.b);
    }
    
    // Получить Dialux материал из OpenGL материала
    void fromOpenGLMaterial(color3f Ka, color3f Kd, color3f Ks, color3f Ke, float Ns, float Ni, float d)
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

            color = changeLuminance(color, Y);

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

            color = changeLuminance(color, Y);

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

            color = changeLuminance(color, Y);

            N = Ns;
            Shin = 40;
        }

        //this->color = color;
        //this->type = type;
        //this->Refl = Refl;
        //this->Kspec_refl = Kspec_refl;
        //this->Trans = Trans;
        //this->Shin = Shin;
    }

};


class Geometry
{

    DialuxMaterial m_material;
    vertex** points = nullptr;
    u32 p_count = 0;

public:
    Geometry()
    {
    }

    ~Geometry()
    {
        clear();
    }

    void clear()
    {
        for (u32 i = 0; i < p_count; i++)
        {
            delete points[i];
        }
    }

    void setMaterial(DialuxMaterial& mat)
    {
        m_material = mat;
    }

    void setPoints(const vertex& point, u32 count)
    {

        p_count = count;
        points = new vertex * [p_count];
        for (u32 i = 0; i < p_count; i++)
        {
            points[i] = new vertex(point);
        }
    }

    DialuxMaterial& getMaterial()
    {
        return m_material;
    }

    vertex*& operator[](u32 index) const
    {

        return points[index];
    };

    void show()
    {
        cout << "Mesh------------\n";
        for (u32 i = 0; i < p_count; i++)
        {
            cout << i;
            cout << "\n";
            points[i]->show();
        }
        cout << "\n";
    }
};

// PuryaMesh
class PuryaMesh
{

    u32 m_calc_points_count;
    Geometry* m_calc_mesh;

public:
    PuryaMesh()
    {
        m_calc_mesh = new Geometry();
    };
    ~PuryaMesh()
    {
        delete m_calc_mesh;
    };

    Geometry*& getGeometry()
    {
        return m_calc_mesh;
    }

    void setMaterial(DialuxMaterial& mat)
    {
        m_calc_mesh->setMaterial(mat);
    }

    void setPoints(vertex point, u32 count)
    {
        m_calc_points_count = count;
        m_calc_mesh->setPoints(point, count);
    }

    void show()
    {
        m_calc_mesh->show();
    }


    void applyMaterialToPoint(vertex* v, const DialuxMaterial& mtl)
    {
        const color4f& color = mtl.color;
        const u32& type = mtl.type;

        v->c = v->vs * mtl.cs;

        // if (type == 1) // Painted
        // {
        //     c = vd * mtl.cd+ vl * mtl.cl
        // }
        // else // Metallic && Transparent
        // {
        //     c = (vd + vl) * mtl.cs;
        // }
    }



    bool normalizeColor(float mvt)
    {
        if (m_calc_points_count)
        {
            // mvt - максимальное значение (user), после которого считаем все белым (или макс. насыщенность)...
            float sum(0.0f);
            float mx(0.0f);
            float Y(0);
            DialuxMaterial& mtl = m_calc_mesh->getMaterial();
            mtl.prepareColors(); // Подготовить цвета материала
            vertex* v(nullptr);
            color3f ic;
            for (u32 i = 0; i < m_calc_points_count; ++i)
            {
                v = (*m_calc_mesh)[i];
                color4f& c = v->c;   // цвет который необходимо расчитать для цветного режима
                color4f& cg = v->cg; // цвет который необходимо расчитать для градаций серого режима

                color3f& vl = v->vl;  // прямая составляющая освещенности
                color3f& vd = v->vd;  // диффузная состовляющая освещенности
                color3f vs = vd + vl; // суммарная освещенность

                // Расчет для град аций серого (тут материал не учитываем, также как в псевдоцветах )
                Y = (vs.r + vs.g + vs.b) / 3.0f;
                cg.set(Y, Y, Y);
                color_normalize(cg, mvt); // Нормируем
                convert(cg, 8.0f);        // Применяем логарифмический контраст
                cg = utils::from_linear(cg);        // Преобразуем в sRGB цвет

                // Расчет для обычного отображения
                applyMaterialToPoint(v, mtl); // Применяем материал (реализуем последнее отражение в экран)
                color_normalize(c, mvt);      // Нормируем
                convert(c);                   // Применяем физиологический контраст
                c = utils::from_linear(c);             // Преобразуем в sRGB цвет
            }
            return true;
        }
        return false;
    }
};


// Получить цвет пикселя из прямой и отраженной составляющей луча падающий на определенный материал
color3f GetColor(color3f Direct, color3f Photon, float Ymax, color3f diffuse, color3f specular, color3f transmission)
{
    // Нормируем результаты расчета
    Direct /= Ymax;
    Photon /= Ymax;

    // sRGB -> linearRGB
    diffuse = utils::to_linear(diffuse);
    specular = utils::to_linear(specular);
    transmission = utils::to_linear(transmission);

    // Производим расчет
    color3f Yrgb = Direct * specular + Photon * diffuse;
    color3f Yrgb_trans = (Direct + Photon) * transmission; // прозрачная часть если нужна

    // linearRGB -> sRGB
    Yrgb = utils::from_linear(Yrgb);
    Yrgb_trans = utils::from_linear(Yrgb_trans); // прозрачная часть если нужна

    return Yrgb;
}


int main() {

    cout << std::fixed << std::setprecision(2);

    PuryaMesh mesh = PuryaMesh();

    // Create material
    DialuxMaterial mat = DialuxMaterial();
    mat.color = color3f(0.2f, 1.0f, 0.4f);
    mat.type = 1;
    mat.Refl = 0.5f;
    mat.Kspec_refl = 0.5f;
    mat.prepareColors();
    mat.show();
    mesh.setMaterial(mat);

    // Create mesh point
    u32 p_count = 1;
    vertex point = vertex();
    point.vl = color3f(100.0f, 200.0f, 200.0f);
    point.vd = color3f(20.0f, 30.0f, 60.0f);
    point.show_point();
    mesh.setPoints(point, p_count);

    // Calc
    float mvt = 1000.0f; // max lux point (лм)
    mesh.normalizeColor(mvt);
    mesh.show();
    system("pause");

}
#endif




