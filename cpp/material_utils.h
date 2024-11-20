#pragma once

enum material_type
{
    undefined = 0,
    transparent = 1,
    metallic = 2,
    painted = 3,
};

namespace material_utils
{

    static const float epsilon = 0.000001f;

    // Constants linear_RGB -> Y (https://en.wikipedia.org/wiki/Relative_luminance)
    static const float K_R = 0.2126f;
    static const float K_G = 0.7152f;
    static const float K_B = 0.0722f;

    static color3f K(K_R, K_G, K_B);

    static void clamp_zero(float &value, float v = 1.0f)
    {
        if (value < 0)
            value = 0;
    }

    static void clamp_value(float &value, float v = 1.0f)
    {
        if (value > v)
            value = v;
    }

    static void clamp_value(float &value, float l, float r)
    {
        assert(r > l);
        if (value < l)
            value = l;
        if (value > r)
            value = r;
    }

    static void clamp_color(color3f &color)
    {
        clamp_value(color.r);
        clamp_value(color.g);
        clamp_value(color.b);
    }

    static float to_linear(float value)
    {
        if (value <= 0.04045f)
        {
            value = value / 12.92f;
        }
        else
        {
            value = pow((value + 0.055f) / 1.055f, 2.4f);
        }

        return value;
    }

    static float from_linear(float value)
    {
        if (value <= 0.0031308f)
        {
            value = value * 12.92f;
        }
        else
        {
            value = 1.055f * pow(value, 1.0f / 2.4f) - 0.055f;
        }

        return value;
    }

    static color3f to_linear(const color3f color)
    {
        color3f linear;

        linear.r = to_linear(color.r);
        linear.g = to_linear(color.g);
        linear.b = to_linear(color.b);

        return linear;
    }

    static color3f from_linear(const color3f color)
    {
        color3f srgb;

        srgb.r = from_linear(color.r);
        srgb.g = from_linear(color.g);
        srgb.b = from_linear(color.b);

        return srgb;
    }

    static color3f to_luminance(color3f color)
    {
        return to_linear(color) * K;
    }

    static color3f from_luminance(color3f luminance)
    {
        return from_linear(luminance / K);
    }

    //static float getY(u32 type, float reflection_factor, float reflection_coating, float transparency)
    //{
    //    if (type == material_type::undefined)
    //    {
    //        assert(0);
    //        return 0;
    //    }

    //    float Y(0.0f);
    //    switch (type)
    //    {
    //    case material_type::metallic:
    //        Y = reflection_factor;
    //        break;
    //    case material_type::painted:
    //        Y = reflection_factor * (1.0f - reflection_coating) / (1.0f - reflection_coating * reflection_factor);
    //        break;
    //    case material_type::transparent:
    //        Y = reflection_factor + transparency;
    //        break;
    //    }
    //    return Y;
    //}

    static void clamp_color_zero(color3f &c)
    {

        c.r = std::max(epsilon, c.r);
        c.g = std::max(epsilon, c.g);
        c.b = std::max(epsilon, c.b);
    }

    static float check_Yrgb(color3f &Yrgb)
    {

        float sum = Yrgb.sum();

        if (Yrgb.r > K_R)
        {
            Yrgb.r = K_R;
        }

        if (Yrgb.g > K_G)
        {
            Yrgb.g = K_G;
        }

        if (Yrgb.b > K_B)
        {
            Yrgb.b = K_B;
        }

        float diff = (sum - Yrgb.sum());

        return diff;
    }

    static color3f changeY(color3f color, const float &Ynew)
    {
        color3f Yrgb = to_luminance(color);

        clamp_color_zero(Yrgb);
        Yrgb = Yrgb.getNormalize() * Ynew;
        float diff = check_Yrgb(Yrgb);
        if (diff > 0)
        {
            color3f coeff = (K - Yrgb).getNormalize();
            Yrgb += coeff * diff;
        }

        return Yrgb / K; // to linear
    }

    // static color3f changeLuminance(const color3f rgb, const float &Y)
    // {
    //     color3f color;
    //     color = to_luminance(rgb);
    //     color = changeY(color, Y);
    //     color = from_luminance(color);
    //     return color;
    // };
}
