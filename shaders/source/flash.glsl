// https://www.shadertoy.com/view/4ssfDj
#define PI 3.14159;

float circ(vec2 p) {
    float r = length(p);
    r = sqrt(r);

    return abs(8. * r * fract(iGlobalTime));
}

void mainImage( out vec4 fragColor, in vec2 fragCoord )
{
    vec2 uv = (fragCoord - .5 * iResolution.xy) / iResolution.y;
    vec3 color = vec3(.0);

    float rz = 1.;

    rz *= abs(circ(uv));

    color = vec3(.2, .2, .2) / rz;

    fragColor = vec4(color, 1.0);
}
