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

uniform sampler2D  color_sampler;
uniform sampler2D  detail_sampler;
uniform sampler2D normal_sampler;

uniform scm color;
uniform scm detail;
uniform scm normal;

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

vec4 sample_detail(vec2 t)
{
    vec4   c = texture2D(detail_sampler, (t * A[ 0] + B[ 0]) * detail.r + detail.b[ 0]);
    c = mix(c, texture2D(detail_sampler, (t * A[ 1] + B[ 1]) * detail.r + detail.b[ 1]), detail.a[ 1]);
    c = mix(c, texture2D(detail_sampler, (t * A[ 2] + B[ 2]) * detail.r + detail.b[ 2]), detail.a[ 2]);
    c = mix(c, texture2D(detail_sampler, (t * A[ 3] + B[ 3]) * detail.r + detail.b[ 3]), detail.a[ 3]);
    c = mix(c, texture2D(detail_sampler, (t * A[ 4] + B[ 4]) * detail.r + detail.b[ 4]), detail.a[ 4]);
    c = mix(c, texture2D(detail_sampler, (t * A[ 5] + B[ 5]) * detail.r + detail.b[ 5]), detail.a[ 5]);
    c = mix(c, texture2D(detail_sampler, (t * A[ 6] + B[ 6]) * detail.r + detail.b[ 6]), detail.a[ 6]);
    c = mix(c, texture2D(detail_sampler, (t * A[ 7] + B[ 7]) * detail.r + detail.b[ 7]), detail.a[ 7]);
    return c;
}

vec4 sample_normal(vec2 t)
{
    vec4   c = texture2D(normal_sampler, (t * A[ 0] + B[ 0]) * normal.r + normal.b[ 0]);
    c = mix(c, texture2D(normal_sampler, (t * A[ 1] + B[ 1]) * normal.r + normal.b[ 1]), normal.a[ 1]);
    c = mix(c, texture2D(normal_sampler, (t * A[ 2] + B[ 2]) * normal.r + normal.b[ 2]), normal.a[ 2]);
    c = mix(c, texture2D(normal_sampler, (t * A[ 3] + B[ 3]) * normal.r + normal.b[ 3]), normal.a[ 3]);
    c = mix(c, texture2D(normal_sampler, (t * A[ 4] + B[ 4]) * normal.r + normal.b[ 4]), normal.a[ 4]);
    c = mix(c, texture2D(normal_sampler, (t * A[ 5] + B[ 5]) * normal.r + normal.b[ 5]), normal.a[ 5]);
    c = mix(c, texture2D(normal_sampler, (t * A[ 6] + B[ 6]) * normal.r + normal.b[ 6]), normal.a[ 6]);
    c = mix(c, texture2D(normal_sampler, (t * A[ 7] + B[ 7]) * normal.r + normal.b[ 7]), normal.a[ 7]);
    c = mix(c, texture2D(normal_sampler, (t * A[ 8] + B[ 8]) * normal.r + normal.b[ 8]), normal.a[ 8]);
    c = mix(c, texture2D(normal_sampler, (t * A[ 9] + B[ 9]) * normal.r + normal.b[ 9]), normal.a[ 9]);
    c = mix(c, texture2D(normal_sampler, (t * A[10] + B[10]) * normal.r + normal.b[10]), normal.a[10]);
    c = mix(c, texture2D(normal_sampler, (t * A[11] + B[11]) * normal.r + normal.b[11]), normal.a[11]);
    c = mix(c, texture2D(normal_sampler, (t * A[12] + B[12]) * normal.r + normal.b[12]), normal.a[12]);
    c = mix(c, texture2D(normal_sampler, (t * A[13] + B[13]) * normal.r + normal.b[13]), normal.a[13]);
    c = mix(c, texture2D(normal_sampler, (t * A[14] + B[14]) * normal.r + normal.b[14]), normal.a[14]);
    c = mix(c, texture2D(normal_sampler, (t * A[15] + B[15]) * normal.r + normal.b[15]), normal.a[15]);
    return c;
}


//------------------------------------------------------------------------------

void main()
{
    vec3 V = normalize(var_V);
    vec3 L = normalize(var_L);
    vec3 T = normalize(var_N);
    vec3 N = normalize(sample_normal(gl_TexCoord[0].xy).rgb * 2.0 - 1.0);
    vec3 R = reflect(-L, N);

    vec3 color  = sample_color (gl_TexCoord[0].xy).rgb;
    vec3 detail = sample_detail(gl_TexCoord[0].xy).rgb;

    vec3 cloud = vec3(0.0);
    vec3 light = vec3(detail.g);
    vec3 water = vec3(detail.b);

    float kd = pow(max(0.0, dot(N, L)),  0.5);
    float ks = pow(max(0.0, dot(R, V)), 16.0);
    float kt = smoothstep(0.0, 0.25, dot(T, L));

    vec3 night = mix(light      + water * ks * 0.5, vec3(0.1, 0.1, 0.3), cloud);
    vec3 day   = mix(color * kd + water * ks * 0.5, vec3(1.0, 1.0, 1.0), cloud);

    gl_FragColor = vec4(mix(night, day, kt), 1.0);
}

