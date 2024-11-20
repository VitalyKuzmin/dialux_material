#pragma once


namespace mesh_utils
{
    static const float k1 = 0.00885645167903563f; // 216/24389 CIE:0.008856
    static const float k2 = 9.03296296296296296f; // 0.01 * (24389/27)   CIE:903.3
    static const float k3 = 0.33333333333333333f; // 1/3
    static const float PI = 3.14159265358979324f;

    void color_check(color4f& c)
    {
        float mx = c.max();
        if (mx > 1){
            c /= mx;
        }
    }

    // c - illum_rgb ([0, Inf],[0,Inf],[0,Inf])
    // nmv - max [0,Inf]
    // return norm rgb [0,1]
    void color_normalize(color4f& c, float nmv)
    {
        nmv /= 3;
        float mx = c.max();
        if (mx > nmv){
            c /= mx;
        }
        else{
            c /= nmv;
        }
    }

    // Y - relative luminance [0,1]
    // return perceptual lightness [0,1]
    float Y2LS(float Y) { return (Y > k1) ? std::pow(Y, k3) * 1.16f - 0.16f : Y * k2;}

    // Y - relative luminance [0,1]
    // b - number to pow [0,1]
    // return pow lightness 
    float Y2Pow(float Y, float b = 0.333333f) { return pow(Y, b); }


    // illum - illuminance [0, Inf]
    // diff_f - Diffuze reflrcting factor [0, 1]
    // return luminance color [0, Inf]
    float illum_to_lum(const float& illum, const float& diff_f) {
        return illum * diff_f / PI;
    }

    // illum - illuminance ([0, Inf],[0,Inf],[0,Inf])
    // diff_f - Diffuze reflrcting factor [0, 1]
    // return luminance color ([0, Inf],[0,Inf],[0,Inf])
    color3f illum_to_lum(  color3f& illum,   color3f &diff_f) {
        return illum * (diff_f / PI);
    }


    // c - linear RGB ([0,1][0,1][0,1])
    // b - pow contrast coeff [0,1]
    // return contrast color ([0,1][0,1][0,1])
    void convert(color4f& c, float b = 0.0f)
    {
        float Y = c.sum() / 3.0f;
        float Ynew;
        if (Y > 0){
            if (b > 0){
                Ynew = Y2Pow(Y, b);     // pow contrast
            }
            else{
                Ynew = Y2LS(Y);         // Lightness contast
            }

            c *= (Ynew / Y);

            color_check(c);
        }
    }

}
