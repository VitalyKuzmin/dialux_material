#include <iomanip>
#include <cmath>
#include <vector>
#include <string>
#include <iostream>
#include <assert.h>

#include "utils.h"

typedef size_t material_id;

enum material_type
{
    undefined = 0,
    transparent = 1,
    metallic = 2,
    painted = 3,
};

static const u32 default_material_type = material_type::metallic;

static const u32 max_uuid = 37; // example: "eeda01b6-0fc5-4e79-824f-a745ae403ec9" (without brackets!)
static const u32 max_material_name = 256;
static const u32 max_entity_name = 256;
static const u32 max_texture_file_name = 512;

namespace materials
{
    static size_t hash(const std::string &key)
    {
        size_t count = key.size();
        size_t h = 2166136261U;
        for (size_t i = 0; i < count; ++i)
        {
            h ^= size_t(key[i]);
            h *= 16777619U;
        }

        return h;
    }
}

namespace material_utils
{

    static const float epsilon = 0.000001f;

    // Constants RGB -> Y (https://en.wikipedia.org/wiki/Relative_luminance)
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

        // Apply gamma correction to linear RGB values
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

    static float getY(u32 type, float reflection_factor, float reflection_coating, float transparency)
    {
        if (type == material_type::undefined)
        {
            assert(0);
            return 0;
        }

        float Y(0.0f);
        switch (type)
        {
        case material_type::metallic:
            Y = reflection_factor;
            break;
        case material_type::painted:
            Y = reflection_factor * (1.0f - reflection_coating) / (1.0f - reflection_coating * reflection_factor);
            break;
        case material_type::transparent:
            Y = reflection_factor + transparency;
            break;
        }
        return Y;
    }

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

    static color3f changeY(color3f Yrgb, const float &Ynew)
    {
        Yrgb = to_luminance(Yrgb);

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

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Material
class Material
{

#ifdef _HIDE
private:
    material_id m_id = 0;        // id = hash(uuid)
    char m_uuid[max_uuid] = {0}; // Example: "00000000-0000-0000-0000-00000000000"
#endif

private:
#ifdef _HIDE
    u32 m_opts = 0; // TODO: serialize !!!
#endif
    char m_name[max_material_name] = {0};

    // params
    u32 m_type = default_material_type;

    color3f m_color;                   ///< user color
    float m_reflection_factor = 0.5f;  ///< reflection factor (quantifies the amount of light that is reflected off a surface)
    float m_reflection_coating = 0.5f; ///< reflection coating
    float m_transparency = 1.0f;       ///< transparency (0 - completely transparent, 1.0 - completely opaque)
    float m_refractive = 1.0f;         ///< index of refraction

    float m_coeff = 0.0f;   ///<  coefficient for transition between material types
    float m_Y = 0.0f;       ///<  global Y for material
    float m_coeff_T = 0.0f; ///<  coefficient for Transparent material

    // energetic (calc)
    color3f m_diffuse_spectrum;      ///< коэффициент диффузного отражения
    color3f m_specular_spectrum;     ///< коэффициент зеркального отражения
    color3f m_transmission_spectrum; ///< прозрачность материала

    // graphics (opengl)
    color3f m_ambient;
    color3f m_diffuse;
    color3f m_specular;
    color3f m_emission;
    float m_shininess = 0.5f;
    float m_N = 1.0f;
    float m_opacity = 1.0f;

#ifdef _HIDE
    // textures params (common)
    texture_params m_texture_prms;
    // textures (names)
    char m_diff_tex_name[max_texture_file_name] = {0}; ///< texture name
    u32 m_diff_tex = 0;                                ///< texture ID
#endif

    // pbr ------------------------------------------------------------------------------------------------
#ifdef _HIDE
    vec4 m_pbr_prms = {1.0f, 1.0f, 1.0f, 1.0f}; // pbr params (x - metallness, y - roughness, z - displacement, w - not used)

    // pbr textures (ID)
    u32 m_a_map = 0;  // albedo
    u32 m_n_map = 0;  // normals
    u32 m_m_map = 0;  // metallness
    u32 m_r_map = 0;  // roughness
    u32 m_ao_map = 0; // ambient occlusion
    u32 m_d_map = 0;  // displacement
    u32 m_e_map = 0;  // environment

#endif

public:
    Material() {}
    Material(const Material &other)
    {
        copy(other);
    }

    Material &operator=(const Material &other)
    {
        if (this == &other)
            return (*this);
        copy(other);
        return (*this);
    }

    ~Material() {}

public:
    void reset()
    {

#ifdef _HIDE
        m_id = 0;
        m_uuid[0] = '\0';
        m_opts = 0;
#endif

        m_name[0] = '\0';

        m_type = material_type::undefined;
        m_color.set(0.0f, 0.0f, 0.0f);

        m_reflection_factor = 0.0f;
        m_reflection_coating = 0.0f;
        m_transparency = 0.0f;
        m_refractive = 1.0f;
        m_coeff = -1.0f;

        // energetic
        m_diffuse_spectrum.set(0.0f, 0.0f, 0.0f);
        m_specular_spectrum.set(0.0f, 0.0f, 0.0f);
        m_transmission_spectrum.set(0.0f, 0.0f, 0.0f);

        // graphics (opengl data)
        m_ambient.set(0.0f, 0.0f, 0.0f);
        m_diffuse.set(0.0f, 0.0f, 0.0f);
        m_specular.set(0.0f, 0.0f, 0.0f);
        m_emission.set(0.0f, 0.0f, 0.0f);
        m_shininess = 0.5f;

#ifdef _HIDE
        // textures (names)
        m_diff_tex_name[0] = '\0';
        m_diff_tex = 0;

        // pbr (textures id)
        m_a_map = 0;  // albedo
        m_n_map = 0;  // normals
        m_m_map = 0;  // metallness
        m_r_map = 0;  // roughness
        m_ao_map = 0; // ambient occlusion
        m_d_map = 0;  // displacement
        m_e_map = 0;  // environment
#endif
    }

    void copy(const Material &other)
    {
        copyParams(other);
        copyEnergetic(other);
        copyGraphics(other);
    }

    void copyParams(const Material &other)
    {
#ifdef _HIDE
        m_id = other.m_id;
        std::strcpy(m_uuid, other.m_uuid);
        m_opts = other.m_opts;
#endif

        std::strcpy(m_name, other.m_name);

        m_type = other.m_type;
        m_color = other.m_color;
        m_reflection_factor = other.m_reflection_factor;
        m_reflection_coating = other.m_reflection_coating;
        m_transparency = other.m_transparency;
        m_refractive = other.m_refractive;
    }

    void copyEnergetic(const Material &other)
    {
        m_diffuse_spectrum = other.m_diffuse_spectrum;
        m_specular_spectrum = other.m_specular_spectrum;
        m_transmission_spectrum = other.m_transmission_spectrum;
    }

    void copyGraphics(const Material &other)
    {
        // graphics (opengl data)
        m_ambient = other.m_ambient;
        m_diffuse = other.m_diffuse;
        m_specular = other.m_specular;
        m_emission = other.m_emission;
        m_shininess = other.m_shininess;

#ifdef _HIDE
        m_texture_prms = other.m_texture_prms;

        // textures (names)
        std::strcpy(m_diff_tex_name, other.m_diff_tex_name);
        m_diff_tex = other.m_diff_tex;

        // pbr
        m_pbr_prms = other.m_pbr_prms;

        m_a_map = other.m_a_map;
        m_n_map = other.m_n_map;
        m_m_map = other.m_m_map;
        m_r_map = other.m_r_map;
        m_ao_map = other.m_ao_map;
        m_d_map = other.m_d_map;
        m_e_map = other.m_e_map;

#endif
    }

public:
#ifdef _HIDE
    indc::Var getAsVar() const
    {
        indc::Var res;
        std::memcpy(res.getBlob(sizeof(Material)), this, sizeof(Material));
        return res;
    }

    void copyFromVar(const indc::Var &var)
    {
        assert(var.is(indc::Var::VT_BLOB));
        if (var.getSize() == sizeof(Material))
            std::memcpy(this, var.getBlob(), sizeof(Material));
    }

    void setId(const material_id &id) { m_id = id; }
    const material_id &getId() const { return m_id; }

    material_id setUuid(const char *suuid, bool update_id = true)
    {
        assert_suuid(suuid);
        strcpy(m_uuid, suuid);

        if (update_id)
            m_id = materials::hash(m_uuid);

        return m_id;
    }

    const char *getUuid() const { return m_uuid; }

#endif

    void setType(u32 type) { m_type = type; }
    const u32 &getType() const { return m_type; }

    bool isTypeUndefined() const { return (m_type == material_type::undefined) ? true : false; }
    bool isTypeTransparent() const { return (m_type == material_type::transparent) ? true : false; }
    bool isTypeMetallic() const { return (m_type == material_type::metallic) ? true : false; }
    bool isTypePainted() const { return (m_type == material_type::painted) ? true : false; }

#ifdef _HIDE
    void setReadOnly(bool on) { on ? m_opts |= material_options::read_only : m_opts &= ~material_options::read_only; }
    bool isReadOnly() const { return (m_opts & material_options::read_only) ? true : false; }

    void setHidden(bool on) { on ? m_opts |= material_options::hidden : m_opts &= ~material_options::hidden; }
    bool isHidden() const { return (m_opts & material_options::hidden) ? true : false; }

    void setPbr(bool on) { on ? m_opts |= material_options::pbr : m_opts &= ~material_options::pbr; }
    bool isPbr() const { return (m_opts & material_options::pbr) ? true : false; }

    void setOptions(u32 opts) { m_opts = opts; }
    const u32 &getOptions() const { return m_opts; }
#endif

    void setName(const char *n)
    {
        assert(n && n[0]);
        std::strcpy(m_name, n);
    }

    const char *getName() const { return m_name; }

    void setColor(const color3f &color) { m_color = color; }
    const color3f &getColor() const { return m_color; }

    const color3f &getDiffuseSpectrum() const { return m_diffuse_spectrum; }
    const color3f &getSpecularSpectrum() const { return m_specular_spectrum; }
    const color3f &getTransmissionSpectrum() const { return m_transmission_spectrum; }

    void setSpectrums(const color3f &diffuse, const color3f &specular, const color3f &transmission)
    {
        m_diffuse_spectrum = diffuse;
        m_specular_spectrum = specular;
        m_transmission_spectrum = transmission;
    }

    bool isValidSpectrums() const
    {
        return (((m_diffuse_spectrum.r + m_specular_spectrum.r + m_transmission_spectrum.r) <= 1.0f) && ((m_diffuse_spectrum.g + m_specular_spectrum.g + m_transmission_spectrum.g) <= 1.0f) && ((m_diffuse_spectrum.b + m_specular_spectrum.b + m_transmission_spectrum.b) <= 1.0f)) ? true : false;
    }

    void setReflectionFactor(float reflection_factor) { m_reflection_factor = reflection_factor; }
    const float &getReflectionFactor() const { return m_reflection_factor; }

    void setReflectionCoating(float reflection_coating) { m_reflection_coating = reflection_coating; }
    const float &getReflectionCoating() const { return m_reflection_coating; }

    void setTransparency(float transparency) { m_transparency = transparency; }
    const float &getTransparency() const { return m_transparency; }

    void setRefractive(float refractive) { m_refractive = refractive; }
    const float &getRefractive() const { return m_refractive; }

    void setCoefficientTransition(float coeff) { m_coeff = coeff; }
    const float &getCoefficientTransition() const { return m_coeff; }

    void setCoefficientT(float coeff) { m_coeff_T = coeff; }
    const float &getCoefficientT() const { return m_coeff_T; }

    void setGlobalY(float coeff) { m_Y = coeff; }
    const float &getGlobalY() const { return m_Y; }

    void setShininess(float shininess) { m_shininess = shininess; }
    const float &getShininess() const { return m_shininess; }

    void setMaterialParams(float reflection_factor, float reflection_coating, float transparency, float refractive)
    {
        m_reflection_factor = reflection_factor;
        m_reflection_coating = reflection_coating;
        m_transparency = transparency;
        m_refractive = refractive;
    }

#ifdef _HIDE
    // diffuse texture (name / id)
    void setDiffuseTextureName(const char *tn)
    {
        assert(tn);
        std::strcpy(m_diff_tex_name, tn);
    }
    void setDiffuseTexture(u32 t) { m_diff_tex = t; }

    void resetDiffuseTexture()
    {
        m_diff_tex_name[0] = '\0';
        m_diff_tex = 0;
    }

    const char *getDiffuseTextureName() const { return m_diff_tex_name; }
    const u32 &getDiffuseTexture() const { return m_diff_tex; }
#endif

    void setAmbientColor(const color3f &color) { m_ambient = color; }
    const color3f &getAmbientColor() const { return m_ambient; }
    void setDiffuseColor(const color3f &color) { m_diffuse = color; }
    const color3f &getDiffuseColor() const { return m_diffuse; }
    void setSpecularColor(const color3f &color) { m_specular = color; }
    const color3f &getSpecularColor() const { return m_specular; }
    void setEmissionColor(const color3f &color) { m_emission = color; }
    const color3f &getEmissionColor() const { return m_emission; }

    void setDiffuseSpectrumColor(const color3f &color) { m_diffuse_spectrum = color; }
    const color3f &getDiffuseSpectrumColor() const { return m_diffuse_spectrum; }
    void setSpecularSpectrumColor(const color3f &color) { m_specular_spectrum = color; }
    const color3f &getSpecularSpectrumColor() const { return m_specular_spectrum; }
    void setTransmissionSpectrumColor(const color3f &color) { m_transmission_spectrum = color; }
    const color3f &getTransmissionSpectrumColor() const { return m_transmission_spectrum; }

    void setColors(const color3f &abient, const color3f &diffuse, const color3f &specular, const color3f &emission)
    {
        m_ambient = abient;
        m_diffuse = diffuse;
        m_specular = specular;
        m_emission = emission;
        m_type = material_type::undefined;
    }

#ifdef _HIDE
    // material texture params ---------------------------------------------------------------------------
    void setTextureParmas(const texture_params &mp) { m_texture_prms = mp; }
    const texture_params &getTextureParmas() const { return m_texture_prms; }

    void setTextureSize(float cx, float cy) { m_texture_prms.setSize(cx, cy); }
    void setTextureSize(const size2f &size) { m_texture_prms.setSize(size); }
    const size2f &getTextureSize() const { return m_texture_prms.getSize(); }

    void setTextureTailing(bool on) { m_texture_prms.setTailing(on); }
    void setTextureStretching(bool on) { m_texture_prms.setStretching(on); }
    bool isTextureTailing() const { return m_texture_prms.isTailing(); }
    bool isTextureStretching() const { return m_texture_prms.isStretching(); }

    void setTextureOffset(float x, float y) { m_texture_prms.offset.set(x, y); }
    void setTextureOffset(const vec2 &offset) { m_texture_prms.offset = offset; }
    const vec2 &getTextureOffset() const { return m_texture_prms.offset; }
    vec2 getTextureOffsetUV() const { return m_texture_prms.getOffsetUV(); }
    void setTextureAngle(float angle) { m_texture_prms.setAngle(angle); }
    const float &getTextureAngle() const { return m_texture_prms.angle; }

    // pbr ---------------------------------------------------------------------------
    void setPbrParams(const vec4 &prms) { m_pbr_prms = prms; }
    void setPbrParams(float metallness, float roughness, float displacement) { m_pbr_prms.set(metallness, roughness, displacement); } // params (x - metallness, y - roughness, z - displacement, w - not used)
    const vec4 &getPbrParams() const { return m_pbr_prms; }

    void setPbrTextures(u32 albedo = 0, u32 normals = 0, u32 metallness = 0, u32 roughness = 0, u32 ao = 0, u32 disp = 0, u32 env = 0)
    {
        m_a_map = albedo;
        m_n_map = normals;
        m_m_map = metallness;
        m_r_map = roughness;
        m_ao_map = ao;
        m_d_map = disp;
        m_e_map = env;
    }

    const u32 &getPbrTextureAlbedo() const { return m_a_map; }
    const u32 &getPbrTextureNormals() const { return m_n_map; }
    const u32 &getPbrTextureMetallness() const { return m_m_map; }
    const u32 &getPbrTextureRoughness() const { return m_r_map; }
    const u32 &getPbrTextureAmbientOcclusion() const { return m_ao_map; }
    const u32 &getPbrTextureDisplacement() const { return m_d_map; }
    const u32 &getPbrTextureEnvironment() const { return m_e_map; }

    std::string getPbrTextureName(const char *texture) const
    {
        assert(texture && texture[0]);
        return indc::str::strFormat("%s-%s", m_name, texture);
    }

#endif

    bool create(const char *suuid, const char *name, u32 type, const color3f &color, float reflection_factor, float reflection_coating, float transparency, float refractive, float shininess)
    {

#ifdef _HIDE
        assert(suuid && (strlen(suuid) == (max_uuid - 1)) && name && name[0] && (type >= 0 && type <= material_type::painted));
#endif

        if (type == material_type::undefined)
        {
            assert(0);
            return false;
        }

        material_utils::clamp_color(const_cast<color3f &>(color));   // Цвет в формате sRGB [0..1] [0..1] [0..1]
        material_utils::clamp_value(reflection_factor, 0.0f, 0.9f);  // Коэффициент отражения (Reflection factor) [0..0.9]
        material_utils::clamp_value(reflection_coating, 0.0f, 1.0f); // Отражение (Reflection coating) [0..1]
        material_utils::clamp_value(transparency, 0.0f, 1.0f);       // Коэффициент передачи (degree of transmission) [0..1]
        material_utils::clamp_value(refractive, 1.0f, 2.0f);         // Показатель преломления (Refractive index) [1..2]
        material_utils::clamp_value(shininess, 0.0f, 1.0f);          // Блескость (Shininess) [0..1]

#ifdef _HIDE
        setUuid(suuid);
#endif

        setName(name);

        setType(type);

        if (isTypeUndefined())
        {
            assert(0);
        }

        setColor(color);

        m_reflection_factor = reflection_factor;
        m_reflection_coating = reflection_coating;

        setTransparency(transparency); // 1.0f - transparency;
        setRefractive(refractive);
        setShininess(shininess);

        recalcGlobalYK();
        recalcProps(); 
        recalcMaterial();

        return isValidSpectrums();
    }

    float getYfromRgb()
    {
        color3f Yrgb = material_utils::to_luminance(m_color); // sRGB to Y
        float Y = 0.9f * Yrgb.sum();                          // 10% идет на поглощение
        return Y;
    };

    bool updateType(u32 type)
    {
        if (m_type == type)
            return false;

        m_type = type;

        return recalcProps();
    }

    bool updateColor(const color3f &color)
    {
        setColor(color);
        setGlobalY(getYfromRgb());
        return recalcProps();
    }

    bool updateReflectionFactor(float reflection_factor)
    {
        if (isTypeUndefined())
        {
            assert(0);
            return false;
        }

        material_utils::clamp_value(reflection_factor, 0.0f, 0.9f);

        if (m_reflection_factor == reflection_factor)
            return false;

        setReflectionFactor(reflection_factor);

        if (isTypePainted())
        {

            if (reflection_factor == 0.0f)
                setReflectionCoating(0.0f);
        }
        else if (isTypeTransparent())
        {
            if (reflection_factor > (1.0f - m_transparency))
            {
                setTransparency(1 - 1.0f - reflection_factor);
            }
        }

        recalcGlobalYK();
        return recalcMaterial();
    }

    bool updateReflectingCoating(float reflection_coating)
    {
        if (isTypeTransparent())
            return false;

        material_utils::clamp_value(reflection_coating, 0.0f, 1.0f);

        if (m_reflection_coating == reflection_coating)
            return false;

        setReflectionCoating(reflection_coating);

        recalcGlobalYK();
        return recalcMaterial();
    }

    bool updateTransparency(float transparency)
    {
        if (!isTypeTransparent())
            return false;

        material_utils::clamp_value(transparency, 0.0f, 1.0f);

        if (m_transparency == transparency)
            return false;

        setTransparency(transparency);

        if (transparency > (1.0f - m_reflection_factor))
        {
            m_reflection_factor = 1.0f - transparency;
        }

        recalcGlobalYK();
        return recalcMaterial();
    }

    bool updateRefractive(float refractive)
    {
        if (!isTypeTransparent())
            return false;

        material_utils::clamp_value(refractive, 1.0f, 2.0f);
        m_refractive = refractive;
        return true;
    }

private:
    // Render --------------------------------------------------------
    bool recalcMaterial()
    {
        color3f diff;
        color3f spec;
        color3f amb;
        color3f trans;

        color3f color = getColor();

        float ReflF = getReflectionFactor();
        float ReflC = getReflectionCoating();
        float Trans = getTransparency();
        float N = getRefractive();
        float Shin = 0.0f;
        float opacity = 0.0f;

        if (isTypeMetallic())
        {

            float Ys = ReflF * ReflC;
            float Yd = ReflF * (1.0f - ReflC);

            float Ysum = ReflF;

            if (Ysum > 0)
            {
                color3f Yrgb = material_utils::changeY(color, Ysum);
                diff = Yrgb * (Yd / Ysum);
                spec = Yrgb * (Ys / Ysum);
                amb = spec;
            }

            trans.set(0.0f, 0.0f, 0.0f);

            opacity = 0.0f;
            Shin = 0.0f;
            N = 1.0f;
        }
        else if (isTypePainted())
        {

            float Ys = ReflF * ReflC;
            float Yd = ReflF * (1 - ReflC); // Ys

            float Ysum = ReflF;

            if (Ysum > 0)
            {
                color3f Yrgb = material_utils::changeY(color, Ysum);
                diff = Yrgb * (Yd / Ysum);
                spec.set(Ys, Ys, Ys); // grayscale
                amb = spec;
            }

            trans.set(0.0f, 0.0f, 0.0f);

            opacity = 0.0f;
            Shin = 0.6f; // 80/128
            N = 1.0f;
        }
        else if (isTypeTransparent())
        {
            float Ysum = ReflF + Trans;

            color3f Yrgb = material_utils::changeY(color, Ysum);

            diff.set(0, 0, 0);
            if (Ysum > 0)
            {
                spec = Yrgb * (ReflF / Ysum);
                trans = Yrgb * (Trans / Ysum);
                opacity = Trans / Ysum;
                amb = Yrgb;
            }

            Shin = 0.3f; // 40/128
            N = N;
        }

        return convertColors(amb, diff, spec, trans, opacity, N, Shin);
    };

    bool convertColors(const color3f &ambient, const color3f &diffuse, const color3f &specular, const color3f &transmission, float opacity, float N, float shininess)
    {
        // linear
        m_diffuse_spectrum = diffuse;
        m_specular_spectrum = specular;
        m_transmission_spectrum = transmission;

        // graphics (opengl data)
        m_diffuse = material_utils::from_linear(diffuse);
        m_specular = material_utils::from_linear(specular);
        m_ambient = material_utils::from_linear(ambient);

        m_opacity = opacity;
        m_N = N;
        m_shininess = shininess;

        return true;
    }

    bool recalcGlobalYK()
    {

        float ReflF = getReflectionFactor();
        float ReflC = getReflectionCoating();
        float Trans = getTransparency();

        if (isTypeMetallic())
        {
            setCoefficientTransition(ReflC);
            setGlobalY(ReflF);
        }
        else if (isTypePainted())
        {

            float K = ReflC * ReflF;
            float Y = (ReflF - K) / (1 - K);

            setCoefficientTransition(K);
            setGlobalY(Y);
        }
        else if (isTypeTransparent())
        {
            float K_T = Trans / ReflF;
            float Y = ReflF + Trans;
            if (Trans > 0)
            {
                setCoefficientTransition(1.0f);
            }

            setCoefficientT(K_T);
            setGlobalY(Y);
        }
        return true;
    }

    bool recalcProps()
    {
        if (isTypeUndefined())
        {
            assert(0);
            return false;
        }

        float Y = getGlobalY();
        float K = getCoefficientTransition();
        float K_T = getCoefficientT();

        if (isTypeMetallic())
        {

            setReflectionCoating(K);
            setReflectionFactor(Y);
        }
        else if (isTypePainted())
        {
            float ReflF = K + (1 - K) * Y;
            float ReflC = ReflF ? K / ReflF : 0;
            if (ReflF > 0.9)
                ReflF = 0.9;

            setReflectionFactor(ReflF);
            setReflectionCoating(ReflC);
        }
        else if (isTypeTransparent())
        {
            float ReflF = Y / (1 + K_T);
            float Trans = Y - ReflF;

            setReflectionFactor(ReflF);
            setTransparency(Trans);
        }

        recalcGlobalYK();
        return recalcMaterial();
    }
};

namespace
{

    void logMaterialType(const Material &mtl)
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

    void logMaterialParams(const Material &mtl)
    {
        char buff[256];

        sprintf_s(buff, 255, "name: %s", mtl.getName());
        std::cout << buff << std::endl;

        const color3f &color = mtl.getColor();
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

    void logMaterialSpectrums(const Material &mtl)
    {
        // energetic (calc)
        const color3f &diffuse_spectrum = mtl.getDiffuseSpectrum();
        const color3f &specular_spectrum = mtl.getSpecularSpectrum();
        const color3f &transmission_spectrum = mtl.getTransmissionSpectrum();

        char buff[256];

        sprintf_s(buff, 255, "diffuse_spectrum:         %.6f, %.6f, %.6f", diffuse_spectrum.r, diffuse_spectrum.g, diffuse_spectrum.b);
        std::cout << buff << std::endl;
        sprintf_s(buff, 255, "specular_spectrum:        %.6f, %.6f, %.6f", specular_spectrum.r, specular_spectrum.g, specular_spectrum.b);
        std::cout << buff << std::endl;
        sprintf_s(buff, 255, "transmission_spectrum:    %.6f, %.6f, %.6f", transmission_spectrum.r, transmission_spectrum.g, transmission_spectrum.b);
        std::cout << buff << std::endl;
    }

    void logMaterialGraphicsColors(const Material &mtl)
    {
        // graphics (opengl)
        const color3f &ambient = mtl.getAmbientColor();
        const color3f &diffuse = mtl.getDiffuseColor();
        const color3f &specular = mtl.getSpecularColor();
        const color3f &emission = mtl.getEmissionColor();

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

    void logMaterial(const Material &mtl)
    {
        logMaterialParams(mtl);
        logMaterialType(mtl);
        logMaterialGraphicsColors(mtl);
        logMaterialSpectrums(mtl);

        std::cout << std::endl;
    }
}

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

    if (1)
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

    system("pause");

    logMaterial(mtl);

    system("pause");
    return 0;
}