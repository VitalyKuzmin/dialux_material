#pragma once

namespace tests
{
    float round_to(float value, unsigned int rate)
    {
        return round(value * pow(10, rate)); /// pow(10, rate)
    }

    color3f round_to(color3f color, unsigned int rate)
    {
        color.r = round_to(color.r, rate);
        color.g = round_to(color.g, rate);
        color.b = round_to(color.b, rate);
        return color;
    }

    void material_error(Material mtl, bool is_not_error)
    {
        if (!is_not_error)
        {
            cout << "Error: -----------------------------------------------------------" << endl;
            logMaterial(mtl);
        }
        else
        {
            // cout << "Valid: -----------------------------------------------------------" << endl;
            //  logMaterial(mtl);
        }
    }

    // template <typename T>
    // void test_param(Material mtl, unsigned int accuracy, T param, T value)
    //{
    //     float p = round_to(param, accuracy);
    //     value = round_to(value, accuracy);
    //     material_error(mtl, p == value);
    // }

    void test_param(Material mtl, unsigned int accuracy, float param, float value)
    {
        float p = round_to(param, accuracy);
        value = round_to(value, accuracy);
        material_error(mtl, p == value);
    }

    void test_param(Material mtl, unsigned int accuracy, color3f color, color3f value)
    {
        color3f c = round_to(color, accuracy);
        value = round_to(value, accuracy);
        material_error(mtl, c == value);
    }

    void test_material_params(Material mtl, unsigned int accuracy,
        float reflection_factor = -1,
        float reflection_coating = -1,
        float transparency = -1)
    {
        if (reflection_factor > 0)
        {
            test_param(mtl, accuracy, mtl.getReflectionFactor(), reflection_factor);
        }
        if (reflection_coating > 0)
        {
            test_param(mtl, accuracy, mtl.getReflectionCoating(), reflection_coating);
        }
        if (transparency > 0)
        {
            test_param(mtl, accuracy, mtl.getTransparency(), transparency);
        }
    }

    void test_material_colors(Material mtl, unsigned int accuracy,
        color3f diffuse = color3f(-1),
        color3f specular = color3f(-1),
        color3f transmission = color3f(-1))
    {
        if (diffuse > 0)
        {
            test_param(mtl, accuracy, mtl.getDiffuseSpectrumColor(), diffuse);
        }
        if (specular > 0)
        {
            test_param(mtl, accuracy, mtl.getSpecularSpectrumColor(), specular);
        }
        if (transmission > 0)
        {
            test_param(mtl, accuracy, mtl.getTransmissionSpectrumColor(), transmission);
        }
    }

    void test_material(Material mtl, unsigned int accuracy,
        float reflection_factor, float reflection_coating = -1, float transparency = -1,
        color3f diffuse = color3f(-1), color3f specular = color3f(-1), color3f transmission = color3f(-1))
    {
        test_material_params(mtl, accuracy, reflection_factor, reflection_coating, transparency);
        test_material_colors(mtl, accuracy, diffuse, specular, transmission);
    }

    void material_1_test(Material mtl)
    {
        if (!mtl.isValidSpectrums())
        {
            cout << "Error: -----------------------------------------------------------" << endl;
            logMaterial(mtl);
        }
        else
        {
            // cout << "Valid: -----------------------------------------------------------" << endl;
            //  logMaterial(mtl);
        }
    }

    void test_input_color(Material mtl, float color_step = 0.1f)
    {
        u32 color_count = 1.0f / color_step;

        color3f color;

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

                    mtl.updateColor(color);

                    material_1_test(mtl);
                }
            }
        }
    }

    void test_type(Material mtl, float color_step = 0.1f)
    {
        for (u32 t = material_type::transparent; t <= material_type::painted; ++t)
        {
            mtl.updateType(t);
            material_1_test(mtl);
            test_input_color(mtl, color_step);
        }
    }

    void test_end(Material mtl, float reflection_factor_step, float reflection_coating_step, float transperancy_step, float color_step = 0.1f)
    {
        u32 reflection_factor_count = 0.9f / reflection_factor_step;
        u32 reflection_coating_count = 1.0f / reflection_coating_step;
        u32 transperancy_count = 1.0f / transperancy_step;

        float reflection_factor(0);
        float reflection_coating(0);
        float transperancy(0);

        for (u32 i = 0; i < reflection_factor_count; ++i)
        {
            reflection_factor += reflection_factor_step;

            mtl.updateReflectionFactor(reflection_factor);
            material_1_test(mtl);

            for (u32 j = 0; j < reflection_coating_count; ++j)
            {
                reflection_coating += reflection_coating_step;

                mtl.updateReflectingCoating(reflection_coating);
                material_1_test(mtl);

                for (u32 k = 0; k < transperancy_count; ++k)
                {
                    transperancy += transperancy_step;

                    mtl.updateTransparency(transperancy);
                    material_1_test(mtl);

                    test_type(mtl, color_step);
                }
            }
        }
    }

}
