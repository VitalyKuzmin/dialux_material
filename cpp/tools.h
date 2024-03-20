// Tools ------------------------------------------------------------------------------------

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

void RGBtoLinear(color3f &rgb)
{
    rgb.r = toLinear(rgb.r);
    rgb.g = toLinear(rgb.g);
    rgb.b = toLinear(rgb.b);
}

void RGBfromLinear(color4f &rgb)
{
    rgb.r = fromLinear(rgb.r);
    rgb.g = fromLinear(rgb.g);
    rgb.b = fromLinear(rgb.b);
    // color_check(rgb);
}

// Change rgb by Luminance
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

float getY(const color3f &rgb)
{
    return K_R * toLinear(rgb.r) + K_G * toLinear(rgb.g) + K_B * toLinear(rgb.b);
}

void toGray(color3f &rgb)
{
    float Y = fromLinear(getY(rgb));
    rgb = color3f(Y, Y, Y);
}


void changeY(float *color, const float &Ynew)
{
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

    for (int j = 0; j < 3; j++) {
        color[j] *= coeff;
    }
    unsigned int index = rgb_max_i(color);
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

        for (int j = 0; j < 3; j++) {
            if (j == index) 
                continue;
            color[j] *= coeff;
        }
        unsigned int index2 = rgb_max_i(color, index);
        if (color[index2] > 1)
        {
            color[index2] = 1;
            unsigned int index3;
            for (int j = 0; j < 3; j++)
            {
                if (j == index || j == index2)
                    continue;
                sum = color[j] * K[j];
                index3 = j;
            }
            coeff = (Ynew - K[index] - K[index2]) / sum;

            color[index3] *= coeff;

            if (color[index3] > 1)
                color[index3] = 1;
        }
    }
}

void changeLuminance(color3f &rgb, const float &Ynew)
{
    float color[3];

    // sRGB to linear RGB --------------------------------
    color[0] = toLinear(rgb.r);
    color[1] = toLinear(rgb.g);
    color[2] = toLinear(rgb.b);

    changeY(color, Ynew);

    // linear RGB to sRGB --------------------------------
    rgb.r = fromLinear(color[0]);
    rgb.g = fromLinear(color[1]);
    rgb.b = fromLinear(color[2]);
};

void changeLuminance2(color3f &rgb, const float &Ynew)
{
    float color[3];

    // sRGB to linear RGB --------------------------------
    color[0] = toLinear(rgb.r);
    color[1] = toLinear(rgb.g);
    color[2] = toLinear(rgb.b);

    changeY(color, Ynew);

    rgb.r = color[0];
    rgb.g = color[1];
    rgb.b = color[2];
};

void checkMaterialColor(const color4f &c, color3f &rgb, const float &Y)
{
    float sum = K_R * rgb.r + K_G * rgb.g + K_B * rgb.b;
    if (abs(sum - Y) > 0.001)
    {
        float color[3] = {0.0f};

        // sRGB to linear RGB --------------------------------
        color[0] = toLinear(c.r);
        color[1] = toLinear(c.g);
        color[2] = toLinear(c.b);

        changeY(color, Y);
        rgb.r = color[0];
        rgb.g = color[1];
        rgb.b = color[2];
    }
};

void checkMaterialColor2(const color4f &c, color3f &rgb, const float &Ynew)
{
    float Y = K_R * rgb.r + K_G * rgb.g + K_B * rgb.b;
    if (abs(Y - Ynew) > 0.001)
    {
        // sRGB to linear RGB --------------------------------
        rgb.r = toLinear(c.r);
        rgb.g = toLinear(c.g);
        rgb.b = toLinear(c.b);

        rgb *= Ynew / Y;
        color_check(rgb);
    }
};

// Получить цвет пикселя из прямой и отраженной составляющей луча падающий на определенный материал
color3f GetColor(color3f Direct, color3f Photon, float Ymax, color3f diffuse, color3f specular, color3f transmission)
{
    // Нормируем результаты расчета
    Direct /= Ymax;
    Photon /= Ymax;

    // sRGB -> linearRGB
    RGBtoLinear(diffuse);
    RGBtoLinear(specular);
    RGBtoLinear(transmission);

    // Производим расчет
    color3f Yrgb = Direct * specular + Photon * diffuse;
    color3f Yrgb_trans = (Direct + Photon) * transmission; // прозрачная часть если нужна

    // linearRGB -> sRGB
    RGBfromLinear(Yrgb);
    RGBfromLinear(Yrgb_trans); // прозрачная часть если нужна

    return Yrgb;
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

// Получить Dialux материал из OpenGL материала
material OpenGLToDialuxMaterial(color3f Ka, color3f Kd, color3f Ks, color3f Ke, float Ns, float Ni, float d)
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

        changeLuminance(color, Y);

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

        changeLuminance(color, Y);

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

        changeLuminance(color, Y);

        N = Ns;
        Shin = 40;
    }

    return material(color, type, Refl, Kspec_refl, Trans, N, Shin);
}

namespace
{
    static const float magic_1 = 0.008856451679036f;   // 216/24389
    static const float magic_2 = 903.296296296296296f; // 24389/27

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

void applyMaterialToPoint(vertex *v, const material &mtl)
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
