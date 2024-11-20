#pragma once

class Geometry
{

    Material m_material;
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

    void setMaterial(Material& mat)
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

    Material& getMaterial()
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


using namespace mesh_utils;

using namespace material_utils;

namespace {
    static const float L_MAX = 1000.0f; //blind luminance [0, Inf] [cd/m^2]
    static const float IL_MAX = 1000.0f; //blind illuminance [0, Inf] [lux]
    static const float IL_POW = 0.3f;    //illuminance contrast coeff [0, 1]


}

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

    void setMaterial(Material& mat)
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

    bool normalizeColor()
    {


        if (m_calc_points_count)
        {
            // mvt - максимальное значение (user), после которого считаем все белым (или макс. насыщенность)...
            vertex* v(nullptr);
            color3f vs;

            color3f cd = m_calc_mesh->getMaterial().getDiffuseSpectrumColor();
            // mtl.prepareColors(); // Подготовить цвета материала
            
            for (u32 i = 0; i < m_calc_points_count; ++i)
            {
                v = (*m_calc_mesh)[i];
                color4f& c = v->c;              // sRGB для обычного режима [0,1]
                color4f& cg = v->cg;            // sRGB для градаций серого [0,1]
                vs = v->vl + v->vd;    // суммарная освещенность [0,Inf] [лк]

                // Расчет для градаций серого (освещенность)
                cg.set(vs.sum() / 3.0f);     // Переводим в освещенность
                color_normalize(cg, IL_MAX); // Нормируем
                convert(cg, IL_POW);         // Применяем pow контраст
                cg = from_linear(cg);        // Преобразуем в sRGB цвет

                // Расчет для обычного отображения
                c = illum_to_lum(vs, cd);      // Переводим в яркость
                color_normalize(c, L_MAX);     // Нормируем
                convert(c);                    // Применяем физиологический контраст
                c = from_linear(c);            // Преобразуем в sRGB цвет
            }
            return true;
        }
        return false;
    }
};

