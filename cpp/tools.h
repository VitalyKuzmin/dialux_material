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

color3f changeLuminance(color3f rgb, const float &Ynew)
{
    color3f color;

    color = utils::to_linear(rgb);

    color = changeY(color, Ynew);

    color = utils::from_linear(color);

    return color;
};


