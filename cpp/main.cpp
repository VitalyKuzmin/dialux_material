#include <iomanip>
#include <cmath>
#include <vector>
#include <string>
#include <iostream>
#include <assert.h>

#include "utils.h"
#include "material_utils.h"
#include "mesh_utils.h"
#include "Material.h"
#include "log.h"
#include "tests.h"
#include "mesh_utils.h"
#include "PuryaMesh.h"


// Main ----------------------------------------------------------------
int main()
{
    std::cout << std::fixed << std::setprecision(2);

    Material mtl;

    // Что должно рабоать с разными параметрами создание материала (считаем что все пришло из файла json) внутри проверка isValidSpectrums
    mtl.create("", "red", material_type::metallic, color3f(1.0f, 0.5f, 0.2f), 0.5f, 0.0f, 1.0f, 1.2f, 5.0f);

    logMaterial(mtl); //"Init material"

    // имитация UI
    // после всех методов должна отрабатывать проверка isValidSpectrums:
    // mtl.isValidSpectrums()

    if (0)
    {

        // 1
        mtl.updateReflectingCoating(0.2f);
        logMaterial(mtl); //"Change Reflecting Coating to 0.2"

        // 2
        mtl.updateType(material_type::transparent);
        logMaterial(mtl); //"Change type to Transparent"

        // 3
        mtl.updateTransparency(0.5f);
        logMaterial(mtl); //"Change Trans to 0.5"

        // 4
        mtl.updateRefractive(1.3f);
        logMaterial(mtl); //"Change N to 1.3"

        // 5
        mtl.updateColor(color3f(0.2f, 0.2f, 0.2f));
        logMaterial(mtl); //"Change color to (50,50,50)"

        // 6
        mtl.updateReflectionFactor(0.9f);
        logMaterial(mtl); //"Change Refl to 0.9"

        // 7
        mtl.updateType(material_type::painted);
        logMaterial(mtl); //"Change type to Painted"
    }

    if (0)
    {
        mtl.updateType(material_type::metallic);
        mtl.updateReflectingCoating(0.0f);
        mtl.updateColor(color3f(0.4f, 0.2f, 0.2f)); // (102,51,51)
        tests::test_material(mtl, 4, 0.04888, 0.0, 0.0, color3f(0.11958, 0.02979, 0.02979));

        mtl.updateReflectingCoating(0.41);
        tests::test_material(mtl, 4, 0.04888, 0.41, 0.0, color3f(0.07055, 0.01757, 0.01757), color3f(0.04902, 0.01221, 0.01221));

        mtl.updateType(material_type::transparent);
        tests::test_material(mtl, 4, 0.04888, 0.41, 0.0, color3f(), color3f(0.11958, 0.02979, 0.02979), color3f());

        mtl.updateTransparency(0.5);
        tests::test_material(mtl, 4, 0.04888, 0.41, 0.5, color3f(), color3f(0.08905, 0.03803, 0.03803), color3f(0.91094, 0.38904, 0.38904));

        mtl.updateType(material_type::painted);
        tests::test_material(mtl, 4, 0.9, 1.0, 0.5, color3f(), color3f(0.9), color3f());

        mtl.updateReflectingCoating(0.36);
        mtl.updateReflectionFactor(0.59);
        mtl.updateType(material_type::metallic);
        tests::test_material(mtl, 4, 0.47943, 0.21239);

        mtl.updateType(material_type::transparent);
        tests::test_material(mtl, 4, 0.04269, 0.21239, 0.43673);
    }

    if (0)
    {

        // tests::test_input_color(mtl, 0.2f);
        // tests::test_type(mtl, 0.2f);
        tests::test_end(mtl, 0.1f, 0.1f, 0.1f, 0.2f);
    }


    // test Mesh Color
    if (1)
    {
        // Create material
        mtl.updateType(material_type::metallic);
        mtl.updateColor(color3f(0.4f, 0.2f, 0.2f));
        mtl.updateReflectingCoating(0.5f);
        mtl.updateReflectionFactor(0.5f);
        
        logMaterial(mtl);
        

        // Create mesh point
        PuryaMesh mesh = PuryaMesh();
        u32 p_count = 1;
        vertex point = vertex();
        point.vl = color3f(100.0f, 200.0f, 200.0f);
        point.vd = color3f(20.0f, 30.0f, 60.0f);
        point.show_point();
        mesh.setPoints(point, p_count);
        mesh.setMaterial(mtl);


        // Calc
        mesh.normalizeColor();
        mesh.show();
        system("pause");

    }

    system("pause");

    logMaterial(mtl);

    system("pause");
    return 0;
}