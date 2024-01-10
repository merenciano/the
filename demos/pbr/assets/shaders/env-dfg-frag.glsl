// Functions from filament (https://github.com/google/filament)
#version 330 core

in vec2 uv;
out vec2 frag_color;

const uint  SAMPLE_COUNT = 1024u;
const float F_SAMPLE_COUNT = 1024.0;
const float PI = 3.14159265359;

vec2 Hammersley(uint i)
{
    i = (i << 16u) | (i >> 16u);
    i = ((i & 0x55555555u) << 1u) | ((i & 0xAAAAAAAAu) >> 1u);
    i = ((i & 0x33333333u) << 2u) | ((i & 0xCCCCCCCCu) >> 2u);
    i = ((i & 0x0F0F0F0Fu) << 4u) | ((i & 0xF0F0F0F0u) >> 4u);
    i = ((i & 0x00FF00FFu) << 8u) | ((i & 0xFF00FF00u) >> 8u);
    return vec2(i / SAMPLE_COUNT, i / exp2(32));
}

// Hemisphere importance sample distribution
vec3 Dggx(vec2 u, float a)
{
    float phi = 2.0 * PI * u.x;
    float cos_theta2 = (1.0 - u.y) / (1.0 + (a + 1.0) * ((a - 1.0) * u.y));
    float cos_theta = sqrt(cos_theta2);
    float sin_theta = sqrt(1.0 - cos_theta2);
    return vec3(sin_theta * cos(phi), sin_theta * sin(phi), cos_theta);
}

// Geometric attenuation
float Gggx(float nov, float nol, float a)
{
    float a2 = a * a;
    float light = nov * sqrt((nol - nol * a2) * nol + a2);
    float view = nol * sqrt((nov - nov * a2) * nov + a2);
    return 0.5f / (view + light);
}

// DFG with multiscatter
vec2 DFG(float nov, float roughness)
{
    vec2 r = vec2(0.0, 0.0);
    vec3 v = vec3(sqrt(1 - nov * nov), 0, nov);
    for (uint i = 0u; i < SAMPLE_COUNT; ++i) {
        vec2 u = Hammersley(i);
        vec3 h = Dggx(u, roughness);
        vec3 l = 2.0 * dot(v, h) * h - v;
        float voh = clamp(dot(v, h), 0.0, 1.0);
        float nol = clamp(l.z, 0.0, 1.0);
        float noh = clamp(h.z, 0.0, 1.0);
        if (nol > 0.0) {
            float g = Gggx(nov, nol, roughness) * nol * (voh / noh);
            float fc = pow(1 - voh, 5.0);

            r.x += g * fc;
            r.y += g;
        }
    }
    return r * (4.0 / F_SAMPLE_COUNT);
}

void main()
{
    DFG(uv.x, uv.y);
    frag_color = DFG(uv.x, uv.y);
}
