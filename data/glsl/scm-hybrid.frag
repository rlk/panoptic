#version 120

varying vec3 var_V;
varying vec3 var_L;
varying vec3 var_N;

struct scm
{
    vec2  r;
    vec2  b[16];
    float a[16];
    float k0;
    float k1;
};

uniform sampler2D color_sampler;

uniform scm color;

uniform vec2 A[16];
uniform vec2 B[16];

//------------------------------------------------------------------------------

vec4 sample_color(vec2 t)
{
    vec4   c = texture2D(color_sampler, (t * A[ 0] + B[ 0]) * color.r + color.b[ 0]);
    c = mix(c, texture2D(color_sampler, (t * A[ 1] + B[ 1]) * color.r + color.b[ 1]), color.a[ 1]);
    c = mix(c, texture2D(color_sampler, (t * A[ 2] + B[ 2]) * color.r + color.b[ 2]), color.a[ 2]);
    c = mix(c, texture2D(color_sampler, (t * A[ 3] + B[ 3]) * color.r + color.b[ 3]), color.a[ 3]);
    c = mix(c, texture2D(color_sampler, (t * A[ 4] + B[ 4]) * color.r + color.b[ 4]), color.a[ 4]);
    c = mix(c, texture2D(color_sampler, (t * A[ 5] + B[ 5]) * color.r + color.b[ 5]), color.a[ 5]);
    c = mix(c, texture2D(color_sampler, (t * A[ 6] + B[ 6]) * color.r + color.b[ 6]), color.a[ 6]);
    c = mix(c, texture2D(color_sampler, (t * A[ 7] + B[ 7]) * color.r + color.b[ 7]), color.a[ 7]);
    c = mix(c, texture2D(color_sampler, (t * A[ 8] + B[ 8]) * color.r + color.b[ 8]), color.a[ 8]);
    c = mix(c, texture2D(color_sampler, (t * A[ 9] + B[ 9]) * color.r + color.b[ 9]), color.a[ 9]);
    c = mix(c, texture2D(color_sampler, (t * A[10] + B[10]) * color.r + color.b[10]), color.a[10]);
    c = mix(c, texture2D(color_sampler, (t * A[11] + B[11]) * color.r + color.b[11]), color.a[11]);
    c = mix(c, texture2D(color_sampler, (t * A[12] + B[12]) * color.r + color.b[12]), color.a[12]);
    c = mix(c, texture2D(color_sampler, (t * A[13] + B[13]) * color.r + color.b[13]), color.a[13]);
    c = mix(c, texture2D(color_sampler, (t * A[14] + B[14]) * color.r + color.b[14]), color.a[14]);
    c = mix(c, texture2D(color_sampler, (t * A[15] + B[15]) * color.r + color.b[15]), color.a[15]);
    return c;
}

//------------------------------------------------------------------------------

vec4 norm(vec4 c, float k0, float k1)
{
    return vec4(mix(vec3(k0), vec3(k1), c.rgb), c.a);
}

void main()
{
    vec3 V = normalize(var_V);
    vec3 L = normalize(var_L);
    vec3 N = normalize(var_N);

    vec4 d = norm(sample_color (gl_TexCoord[0].xy),  color.k0,  color.k1);

    float nl = max(0.0, dot(N, L));
    float nv = max(0.0, dot(N, V));
    float kd = 2.0 * nl / (nl + nv);

    gl_FragColor = vec4(d.rgb * clamp(kd, 0.1, 1.0), 1.0);
}

