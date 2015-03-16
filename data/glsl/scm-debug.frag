#version 120

struct scm
{
    vec2  r;
    vec2  b[16];
    float a[16];
    float k0;
    float k1;
};

uniform sampler2D color_sampler;
uniform scm       color;

uniform vec2 A[16];
uniform vec2 B[16];

//------------------------------------------------------------------------------

vec4 sample_color(vec2 t)
{
    const float L = 0.2;
    const float H = 0.8;

    vec4   c = vec4(L, L, L, 1.0);
    c = mix(c, vec4(H, L, L, 1.0), color.a[ 1]);
    c = mix(c, vec4(L, H, L, 1.0), color.a[ 2]);
    c = mix(c, vec4(H, H, L, 1.0), color.a[ 3]);
    c = mix(c, vec4(L, L, H, 1.0), color.a[ 4]);
    c = mix(c, vec4(H, L, H, 1.0), color.a[ 5]);
    c = mix(c, vec4(L, H, H, 1.0), color.a[ 6]);
    c = mix(c, vec4(H, H, H, 1.0), color.a[ 7]);
    c = mix(c, vec4(L, L, L, 1.0), color.a[ 8]);
    c = mix(c, vec4(H, L, L, 1.0), color.a[ 9]);
    c = mix(c, vec4(L, H, L, 1.0), color.a[10]);
    c = mix(c, vec4(H, H, L, 1.0), color.a[11]);
    c = mix(c, vec4(L, L, H, 1.0), color.a[12]);
    c = mix(c, vec4(H, L, H, 1.0), color.a[13]);
    c = mix(c, vec4(L, H, H, 1.0), color.a[14]);
    c = mix(c, vec4(H, H, H, 1.0), color.a[15]);

    vec2 d = step(vec2(0.05), t) - step(vec2(0.95), t);

    return mix(c * 0.9, c, d.x * d.y);
}

//------------------------------------------------------------------------------

void main()
{
    vec4 k = sample_color(gl_TexCoord[0].xy);
    gl_FragColor = vec4(mix(vec3(color.k0), vec3(color.k1), vec3(k)), 1.0);
}
