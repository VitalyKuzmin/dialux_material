// #include <iostream>
#include <iomanip>
#include <cmath>
#include <vector>
#include <string>

#include "utils.h"
#include "tools.h"

using namespace std;

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
        switch (mType)
        {
        case Type::Metallic:
            Y = mRefl;
            break;
        case Type::Painted:
            Y = mRefl * (1 - mKspec_refl) / (1 - mKspec_refl * mRefl);
            break;
        case Type::Transparent:
            Y = mRefl + mTrans;
            break;
        }
        return Y;
    }

    // Update
    void updateRGB()
    {
        float Y = getYfromProps();
        changeLuminance(mColor, Y);
    }

    void updateProps()
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

void material::prepareColors()
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

// Colors ----------------------------------------------------------------

class Geometry
{

    material m_material;
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

    void setMaterial(material &mat)
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

    material &getMaterial()
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

    void setMaterial(material &mat)
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

    bool normalizeColor2(float mvt)
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

                // Расчет для град аций серого (тут материал не учитываем, также как в псевдоцветах )
                Y = (vs.r + vs.g + vs.b) / 3.0f;
                cg.set(Y, Y, Y);
                color_normalize(cg, mvt); // Нормируем
                convert(cg, 8.0f);        // Применяем логарифмический контраст
                RGBfromLinear(cg);        // Преобразуем в sRGB цвет

                // Расчет для обычного отображения
                applyMaterialToPoint(v, mtl); // Применяем материал (реализуем последнее отражение в экран)
                color_normalize(c, mvt);      // Нормируем
                convert(c);                   // Применяем физиологический контраст
                RGBfromLinear(c);             // Преобразуем в sRGB цвет
            }
            return true;
        }
        return false;
    }
};

// Main ----------------------------------------------------------------
int main()
{
    cout << std::fixed << std::setprecision(2);

    if (false)
    {
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

    if (true)
    {

        PuryaMesh mesh = PuryaMesh();

        // Create material
        material mat = material();
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
        mesh.normalizeColor2(mvt);
        mesh.show();
        system("pause");
    }
}
