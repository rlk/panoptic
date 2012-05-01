
uniform float     n0;
uniform float     n1;
uniform vec2      f_mul[64];
uniform vec2      f_add[64];
uniform sampler2D f_img[64];
uniform float     f_age[64];

//------------------------------------------------------------------------------

vec4 img0(vec2 t)
{
    vec4 c = texture2D(f_img[0], f_mul[0] * t + f_add[0]);
    return vec4(c.rgb, c.a * f_age[0]);
}

vec4 img1(vec2 t)
{
    vec4 c = texture2D(f_img[1], f_mul[1] * t + f_add[1]);
    return vec4(c.rgb, c.a * f_age[1]);
}

vec4 img2(vec2 t)
{
    vec4 c = texture2D(f_img[2], f_mul[2] * t + f_add[2]);
    return vec4(c.rgb, c.a * f_age[2]);
}

vec4 img3(vec2 t)
{
    vec4 c = texture2D(f_img[3], f_mul[3] * t + f_add[3]);
    return vec4(c.rgb, c.a * f_age[3]);
}

vec4 img4(vec2 t)
{
    vec4 c = texture2D(f_img[4], f_mul[4] * t + f_add[4]);
    return vec4(c.rgb, c.a * f_age[4]);
}

vec4 img5(vec2 t)
{
    vec4 c = texture2D(f_img[5], f_mul[5] * t + f_add[5]);
    return vec4(c.rgb, c.a * f_age[5]);
}

vec4 img6(vec2 t)
{
    vec4 c = texture2D(f_img[6], f_mul[6] * t + f_add[6]);
    return vec4(c.rgb, c.a * f_age[6]);
}

vec4 img7(vec2 t)
{
    vec4 c = texture2D(f_img[7], f_mul[7] * t + f_add[7]);
    return vec4(c.rgb, c.a * f_age[7]);
}

vec4 img8(vec2 t)
{
    vec4 c = texture2D(f_img[8], f_mul[8] * t + f_add[8]);
    return vec4(c.rgb, c.a * f_age[8]);
}

vec4 img9(vec2 t)
{
    vec4 c = texture2D(f_img[9], f_mul[9] * t + f_add[9]);
    return vec4(c.rgb, c.a * f_age[9]);
}

vec4 img10(vec2 t)
{
    vec4 c = texture2D(f_img[10], f_mul[10] * t + f_add[10]);
    return vec4(c.rgb, c.a * f_age[10]);
}

vec4 img11(vec2 t)
{
    vec4 c = texture2D(f_img[11], f_mul[11] * t + f_add[11]);
    return vec4(c.rgb, c.a * f_age[11]);
}

vec4 img12(vec2 t)
{
    vec4 c = texture2D(f_img[12], f_mul[12] * t + f_add[12]);
    return vec4(c.rgb, c.a * f_age[12]);
}

vec4 img13(vec2 t)
{
    vec4 c = texture2D(f_img[13], f_mul[13] * t + f_add[13]);
    return vec4(c.rgb, c.a * f_age[13]);
}

vec4 img14(vec2 t)
{
    vec4 c = texture2D(f_img[14], f_mul[14] * t + f_add[14]);
    return vec4(c.rgb, c.a * f_age[14]);
}

vec4 img15(vec2 t)
{
    vec4 c = texture2D(f_img[15], f_mul[15] * t + f_add[15]);
    return vec4(c.rgb, c.a * f_age[15]);
}

//------------------------------------------------------------------------------

vec4 blend(vec4 a, vec4 b)
{
    return vec4(mix(b.rgb, a.rgb, a.a), 1.0);
}

vec4 sample(vec2 t)
{
    vec4 c =  img0 (t);
    c = blend(img1 (t), c);
    c = blend(img2 (t), c);
    c = blend(img3 (t), c);
    c = blend(img4 (t), c);
    c = blend(img5 (t), c);
    c = blend(img6 (t), c);
    c = blend(img7 (t), c);
    c = blend(img8 (t), c);
    c = blend(img9 (t), c);
    c = blend(img10(t), c);
    c = blend(img11(t), c);
    c = blend(img12(t), c);
    c = blend(img13(t), c);
    c = blend(img14(t), c);
    c = blend(img15(t), c);
    return c;
}


float peak(float k, float c, float m)
{
    return max(0.0, 1.0 - abs(k - c) * m);
}

vec4 colormap(float k)
{
    return peak(k, 0.0, 5.0) * vec4(1.0, 0.0, 1.0, 1.0) +
           peak(k, 0.2, 5.0) * vec4(0.0, 0.0, 1.0, 1.0) +
           peak(k, 0.4, 5.0) * vec4(0.0, 1.0, 1.0, 1.0) +
           peak(k, 0.6, 5.0) * vec4(1.0, 1.0, 0.0, 1.0) +
           peak(k, 0.8, 5.0) * vec4(1.0, 0.0, 0.0, 1.0) +
           peak(k, 1.0, 5.0) * vec4(1.0, 1.0, 1.0, 1.0);
}

//------------------------------------------------------------------------------

void main()
{
    vec3 c = sample(gl_TexCoord[0].xy).rgb;
    gl_FragColor = vec4((c - vec3(n0)) / (n1 - n0), 1.0);
}