#pragma once

namespace
{
    void logMaterialType(const Material& mtl)
    {
        if (mtl.isTypeMetallic())
        {
            cout << "Type: (Metallic)" << endl;
        }
        else if (mtl.isTypePainted())
        {
            cout << "Type: (Painted)" << endl;
        }
        else if (mtl.isTypeTransparent())
        {
            cout << "Type: (Transparent)" << endl;
        }
    }

    void logMaterialParams(const Material& mtl)
    {
        char buff[256];

        sprintf_s(buff, 255, "name: %s", mtl.getName());
        std::cout << buff << std::endl;

        const color3f& color = mtl.getColor();
        sprintf_s(buff, 255, "color: %.6f, %.6f, %.6f", color.r, color.g, color.b);
        std::cout << buff << std::endl;

        float reflection_factor = mtl.getReflectionFactor();
        float reflection_coating = mtl.getReflectionCoating();
        float transparency = mtl.getTransparency();
        float refractive = mtl.getRefractive(); // коэфф. преломления
        // float coeff = mtl.getCoefficientTransition();
        float shininess = mtl.getShininess();

        sprintf_s(buff, 255, "reflection_factor:    %.6f", reflection_factor);
        std::cout << buff << std::endl;
        sprintf_s(buff, 255, "reflection_coating:   %.6f", reflection_coating);
        std::cout << buff << std::endl;
        sprintf_s(buff, 255, "transparency:         %.6f", transparency);
        std::cout << buff << std::endl;
        sprintf_s(buff, 255, "refractive:           %.6f", refractive);
        std::cout << buff << std::endl;
        // sprintf_s(buff, 255, "coeff:                %.6f", coeff);                  std::cout << buff << std::endl;
        sprintf_s(buff, 255, "shininess:            %.6f", shininess);
        std::cout << buff << std::endl;
    }

    void logMaterialSpectrums(const Material& mtl)
    {
        // energetic (calc)
        const color3f& diffuse_spectrum = mtl.getDiffuseSpectrum();
        const color3f& specular_spectrum = mtl.getSpecularSpectrum();
        const color3f& transmission_spectrum = mtl.getTransmissionSpectrum();

        char buff[256];

        sprintf_s(buff, 255, "diffuse_spectrum:         %.6f, %.6f, %.6f", diffuse_spectrum.r, diffuse_spectrum.g, diffuse_spectrum.b);
        std::cout << buff << std::endl;
        sprintf_s(buff, 255, "specular_spectrum:        %.6f, %.6f, %.6f", specular_spectrum.r, specular_spectrum.g, specular_spectrum.b);
        std::cout << buff << std::endl;
        sprintf_s(buff, 255, "transmission_spectrum:    %.6f, %.6f, %.6f", transmission_spectrum.r, transmission_spectrum.g, transmission_spectrum.b);
        std::cout << buff << std::endl;
    }

    void logMaterialGraphicsColors(const Material& mtl)
    {
        // graphics (opengl)
        const color3f& ambient = mtl.getAmbientColor();
        const color3f& diffuse = mtl.getDiffuseColor();
        const color3f& specular = mtl.getSpecularColor();
        const color3f& emission = mtl.getEmissionColor();

        char buff[256];

        sprintf_s(buff, 255, "ambient:     %.6f, %.6f, %.6f", ambient.r, ambient.g, ambient.b);
        std::cout << buff << std::endl;
        sprintf_s(buff, 255, "diffuse:     %.6f, %.6f, %.6f", diffuse.r, diffuse.g, diffuse.b);
        std::cout << buff << std::endl;
        sprintf_s(buff, 255, "specular:    %.6f, %.6f, %.6f", specular.r, specular.g, specular.b);
        std::cout << buff << std::endl;
        sprintf_s(buff, 255, "emission:    %.6f, %.6f, %.6f", emission.r, emission.g, emission.b);
        std::cout << buff << std::endl;
    }

    void logMaterial(const Material& mtl)
    {
        logMaterialParams(mtl);
        logMaterialType(mtl);
        logMaterialGraphicsColors(mtl);
        logMaterialSpectrums(mtl);

        std::cout << std::endl;
    }
}
