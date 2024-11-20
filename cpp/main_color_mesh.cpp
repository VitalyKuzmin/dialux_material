// #include <iostream>
#include <iomanip>
#include <cmath>
#include <vector>
#include <string>

#include <assert.h>

#include "utils.h"
// #include "tools.h"

using namespace std;

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

namespace
{
    static const float magic_1 = 0.008856451679036f;   // 216/24389
    static const float magic_2 = 903.296296296296296f; // 24389/27

    void color_check(color4f &c)
    {
        float mx = gre_max(c.r, gre_max(c.g, c.b));
        if (mx > 1)
        {
            c.r /= mx;
            c.g /= mx;
            c.b /= mx;
        }
    }

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

    float log_a_to_base_b(const float &a, const float &b)
    {
        return log2(a) / log2(b);
    }

    float Y2LS(float Y) { return (Y <= magic_1) ? Y * magic_2 : std::pow(Y, 1.0f / 3.0f) * 116.0f - 16.0f; }

    float Y2Log(float Y, float b = 8.0f) { return log_a_to_base_b(Y, b) / 3.0f + 1.0f; }

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
    color3f ct; // linear transparent Y RGB[0,1]
    color3f cs; // linear sum Y RGB[0,1]

    // // Calc Params
    // color3f diff; // linear diffuse Y RGB[0,1]
    // color3f spec; // linear specular Y RGB[0,1]
    // color3f amb;  // linear sum Y RGB[0,1]

    // float opacity = 0.0f;

    DialuxMaterial() {

    };

    DialuxMaterial(const DialuxMaterial &mat)
    {
        set(mat.color, mat.type, mat.Refl, mat.Kspec_refl, mat.Trans, mat.N, mat.Shin);
    };

    DialuxMaterial(const color3f &c, const unsigned int &t, const float &refl,
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

        // cout << "opacity: ";
        // cout << opacity;
        // cout << "\n";
        // cout << "\n";
    }

    void checkMaterialColor(const color4f &c, color3f &rgb, const float &Y)
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
            float Yd = Refl * (1 - Kspec_refl);
            // Y = Refl;
            // checkMaterialColor(color, cs, Y);
            checkMaterialColor(color, cd, Yd);
        }
        else if (type == 1) // Painted
        {

            // float Ys = Refl * Kspec_refl;
            // float Yd = Refl * (1 - Ys);
            float Yd = Refl * (1 - Refl * Kspec_refl);
            // Y = Refl * (1 - Kspec_refl) / (1 - Kspec_refl * Refl);
            // checkMaterialColor(color, cs, Y);

            checkMaterialColor(color, cd, Yd);
        }
        else if (type == 2) // Transparent
        {
            // Y = Refl + Trans;
            // checkMaterialColor(color, cs, Y);

            checkMaterialColor(color, ct, Trans);
            // opacity = Trans / Y;
        }
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

    float getY(const color3f &rgb)
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

        // this->color = color;
        // this->type = type;
        // this->Refl = Refl;
        // this->Kspec_refl = Kspec_refl;
        // this->Trans = Trans;
        // this->Shin = Shin;
    }
};

class Geometry
{

    DialuxMaterial m_material;
    vertex **points = nullptr;
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

    void setMaterial(DialuxMaterial &mat)
    {
        m_material = mat;
    }

    void setPoints(const vertex &point, u32 count)
    {

        p_count = count;
        points = new vertex *[p_count];
        for (u32 i = 0; i < p_count; i++)
        {
            points[i] = new vertex(point);
        }
    }

    DialuxMaterial &getMaterial()
    {
        return m_material;
    }

    vertex *&operator[](u32 index) const
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
    Geometry *m_calc_mesh;

public:
    PuryaMesh()
    {
        m_calc_mesh = new Geometry();
    };
    ~PuryaMesh()
    {
        delete m_calc_mesh;
    };

    Geometry *&getGeometry()
    {
        return m_calc_mesh;
    }

    void setMaterial(DialuxMaterial &mat)
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

    void applyMaterialToPoint(vertex *v, const DialuxMaterial &mtl)
    {
        const color4f &color = mtl.color;
        const u32 &type = mtl.type;

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
            float Y(0);
            DialuxMaterial &mtl = m_calc_mesh->getMaterial();
            mtl.prepareColors(); // Подготовить цвета материала
            vertex *v(nullptr);
            for (u32 i = 0; i < m_calc_points_count; ++i)
            {
                v = (*m_calc_mesh)[i];
                color4f &c = v->c;   // цвет который необходимо расчитать для цветного режима
                color4f &cg = v->cg; // цвет который необходимо расчитать для градаций серого режима

                color3f &vl = v->vl;  // прямая составляющая освещенности
                color3f &vd = v->vd;  // диффузная состовляющая освещенности
                color3f vs = vd + vl; // суммарная освещенность

                // Расчет для град аций серого (тут материал не учитываем, также как в псевдоцветах )
                Y = vs.sum() / 3.0f;
                cg.set(Y, Y, Y);
                color_normalize(cg, mvt);    // Нормируем
                convert(cg, 8.0f);           // Применяем логарифмический контраст
                cg = utils::from_linear(cg); // Преобразуем в sRGB цвет

                // Расчет для обычного отображения
                applyMaterialToPoint(v, mtl); // Применяем материал (реализуем последнее отражение в экран)
                color_normalize(c, mvt);      // Нормируем
                convert(c);                   // Применяем физиологический контраст
                c = utils::from_linear(c);    // Преобразуем в sRGB цвет
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

int main_color_mesh()
{

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

    return false;
}
