#include <GL/glew.h>
#include "FreeFrameClock.h"

#include <cmath>

#define DEBUG

#ifdef DEBUG
#include <cstdio>
void printLog(GLuint obj)
{
    int infologLength = 0;
    char infoLog[1024];

    if (glIsShader(obj))
        glGetShaderInfoLog(obj, 1024, &infologLength, infoLog);
    else
        glGetProgramInfoLog(obj, 1024, &infologLength, infoLog);

    if (infologLength > 0)
        fprintf(stderr, "GLSL :: %s\n", infoLog);
}
#endif

const GLchar *fragmentShaderCode =
        "#version 330 core \n"
        "#define PI  3.14159265359"
        "#define EPS .01"
        "uniform vec3      iResolution;\n"
        "out vec4          FragmentColor;\n"
        "float df_disk(in vec2 p, in vec2 c, in float r) {"
        "    return clamp(length(p - c) - r, 0., 1.);"
        "}"
        "float df_circ(in vec2 p, in vec2 c, in float r) {"
        "    return abs(r - length(p - c));"
        "}"
        "float df_line(in vec2 p, in vec2 a, in vec2 b) {"
        "    vec2 pa = p - a, ba = b - a;"
        "    float h = clamp(dot(pa,ba) / dot(ba,ba), 0., 1.);"
        "    return length(pa - ba * h);"
        "}"
        "float sharpen(in float d, in float w) {"
        "    float e = 1. / min(iResolution.y , iResolution.x);"
        "    return 1. - smoothstep(-e, e, d - w);"
        "}"
        "vec2 rotate(in vec2 p, in float t) {"
        "    t = t * 2. * PI;"
        "    return vec2(p.x * cos(t) - p.y * sin(t),"
        "                p.y * cos(t) + p.x * sin(t));"
        "}"
        "void main(void)"
        "{"
        "    vec3 col = vec3(0);"
        "    vec2 c = vec2(0), u = vec2(0,1);"
        "    float c1 = sharpen(df_circ(uv, c, .90), EPS * 1.5);"
        "    float c2 = sharpen(df_circ(uv, c, .04), EPS * 0.5);"
        "    float d1 = sharpen(df_disk(uv, c, .01), EPS * 1.5);"
        "    col = max(max(c1, c2), d1);"
        "    FragmentColor = vec4(col, 1.0);"
        "}";

/*
float df_scene(vec2 uv)
{
    float thrs = 0. / 3600.;
    float tmin = mod(0., 3600.) / 60.;
    float tsec = mod(mod(0., 3600.), 60.);
    tsec = floor(tsec);
    
    vec2 c = vec2(0), u = vec2(0,1);
    float c1 = sharpen(df_circ(uv, c, .90), EPS * 1.5);
    float c2 = sharpen(df_circ(uv, c, .04), EPS * 0.5);
    float d1 = sharpen(df_disk(uv, c, .01), EPS * 1.5);
    float l1 = sharpen(df_line(uv, c, rotate(u,-thrs / 12.) * .60), EPS * 1.7);
    float l2 = sharpen(df_line(uv, c, rotate(u,-tmin / 60.) * .80), EPS * 1.0);
    float l3 = sharpen(df_line(uv, c, rotate(u,-tsec / 60.) * .85), EPS * 0.5);
    return max(max(max(max(max(l1, l2), l3), c1), c2), d1);
}

void mainImage( out vec4 fragColor, in vec2 fragCoord )
{
    vec2 uv = (fragCoord.xy / iResolution.xy * 2. - 1.);
    uv.x *= iResolution.x / iResolution.y;
    vec3 col = vec3(0);
    col += df_scene(uv);
    
    fragColor = vec4(col, 1);
}
*/

////////////////////////////////////////////////////////////////////////////////////////////////////
//  Plugin information
////////////////////////////////////////////////////////////////////////////////////////////////////

static CFFGLPluginInfo PluginInfo (
    FreeFrameTest::CreateInstance,	// Create method
    "GLCLOCK",             // Plugin unique ID
    "FreeFrameClock",    // Plugin name
    1,                  // API major version number
    500,                // API minor version number
    1,                  // Plugin major version number
    000,                // Plugin minor version number
    FF_SOURCE,          // Plugin type
    "Displays a clock",	 // Plugin description
    "by Bruno Herbelin"  // About
);


////////////////////////////////////////////////////////////////////////////////////////////////////
//  Constructor and destructor
////////////////////////////////////////////////////////////////////////////////////////////////////

FreeFrameTest::FreeFrameTest()
: CFreeFrameGLPlugin()
{
    fbo = 0;
    shaderProgram = 0;
    fragmentShader = 0;

    // Input properties
    SetMinInputs(0);
    SetMaxInputs(0);

}

////////////////////////////////////////////////////////////////////////////////////////////////////
//  Methods
////////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef FF_FAIL
// FFGL 1.5
DWORD   FreeFrameTest::InitGL(const FFGLViewportStruct *vp)
#else
// FFGL 1.6
FFResult FreeFrameTest::InitGL(const FFGLViewportStruct *vp)
#endif
{

    glewInit();
    if (!GLEW_VERSION_2_0)
    {
#ifdef DEBUG
        fprintf(stderr, "OpenGL 2.0 not supported. Exiting freeframe plugin.\n");
#endif
        return FF_FAIL;
    }

    fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderCode, NULL);
    glCompileShader(fragmentShader);
#ifdef DEBUG
    printLog(fragmentShader);
#endif
    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
#ifdef DEBUG
    printLog(shaderProgram);
#endif

    uniform_viewportsize = glGetUniformLocation(shaderProgram, "iResolution");

    glUseProgram(shaderProgram);
    glUniform3f(uniform_viewportsize, vp->width, vp->height, 0.0);
    glUseProgram(0);

    glEnable(GL_TEXTURE);
    glActiveTexture(GL_TEXTURE0);

    return FF_SUCCESS;
}


#ifdef FF_FAIL
// FFGL 1.5
DWORD   FreeFrameTest::DeInitGL()
#else
// FFGL 1.6
FFResult FreeFrameTest::DeInitGL()
#endif
{
    if (fragmentShader) glDeleteShader(fragmentShader);
    if (shaderProgram)  glDeleteProgram(shaderProgram);

    return FF_SUCCESS;
}

#ifdef FF_FAIL
// FFGL 1.5
DWORD   FreeFrameTest::SetTime(double time)
#else
// FFGL 1.6
FFResult FreeFrameTest::SetTime(double time)
#endif
{
  m_curTime = time;
  return FF_SUCCESS;
}

#ifdef FF_FAIL
// FFGL 1.5
DWORD	FreeFrameTest::ProcessOpenGL(ProcessOpenGLStruct* pGL)
#else
// FFGL 1.6
FFResult FreeFrameTest::ProcessOpenGL(ProcessOpenGLStruct *pGL)
#endif
{
  if (pGL->numInputTextures<1)
    return FF_FAIL;

  if (pGL->inputTextures[0]==NULL)
    return FF_FAIL;

  FFGLTextureStruct &Texture = *(pGL->inputTextures[0]);

  glClearColor(0.f, 0.f, 0.f, 0.f);
  glClear(GL_COLOR_BUFFER_BIT);

  // use the blurring shader program
  glUseProgram(shaderProgram);

  //enable texturemapping
  glEnable(GL_TEXTURE_2D);

  //modulate texture colors with white (just show
  //the texture colors as they are)
  glColor4f(1.f, 1.f, 1.f, 1.f);
  //(default texturemapping behavior of OpenGL is to
  //multiply texture colors by the current gl color)

  //bind the texture handle to its target
  glBindTexture(GL_TEXTURE_2D, Texture.Handle);

  glBegin(GL_QUADS);

  //lower left
  glTexCoord2d(0.0, 0.0);
  glVertex2f(-1,-1);

  //upper left
  glTexCoord2d(0.0, 1.0);
  glVertex2f(-1,1);

  //upper right
  glTexCoord2d(1.0, 1.0);
  glVertex2f(1,1);

  //lower right
  glTexCoord2d(1.0, 0.0);
  glVertex2f(1,-1);
  glEnd();

  // disable shader program
  glUseProgram(0);

  //unbind the texture
  glBindTexture(GL_TEXTURE_2D, 0);

  //disable texturemapping
  glDisable(GL_TEXTURE_2D);

  return FF_SUCCESS;
}
