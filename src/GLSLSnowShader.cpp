#include "GLSLSnowShader.h"
#include "ManagerShader.h"
#include "ManagerEnvironmentConfiguration.h"
#include "GLSLUniform.h"
#include "GLSLAttribute.h"
#include "Model.h"
#include "GLView.h"
#include "Camera.h"
#include "ManagerSDLTime.h"
#include "WO.h"
#include <chrono>
using namespace Aftr;

GLSLSnowShader *GLSLSnowShader::New(WO *wo)
{
    auto vert = ManagerEnvironmentConfiguration::getLMM() + "shaders/snow.vert";
    auto frag = ManagerEnvironmentConfiguration::getLMM() + "shaders/snow.frag";

    GLSLShaderDataShared *shdrData = ManagerShader::loadShaderDataShared(vert, frag);
    if (shdrData == nullptr)
        return nullptr;

    GLSLSnowShader *shdr = new GLSLSnowShader(shdrData);
    shdr->parent = wo;
    return shdr;
}

GLSLSnowShader *GLSLSnowShader::New(GLSLShaderDataShared *shdrData)
{
    GLSLSnowShader *shdr = new GLSLSnowShader(shdrData);
    return shdr;
}

GLSLSnowShader::GLSLSnowShader(GLSLShaderDataShared *dataShared) : GLSLShader(dataShared)
{
    // Per vertex attributes
    this->addAttribute(new GLSLAttribute("VertexPosition", atVEC3, this));
    this->addAttribute(new GLSLAttribute("VertexNormal", atVEC3, this));
    this->addAttribute(new GLSLAttribute("VertexTexCoord", atVEC2, this));
    this->addAttribute(new GLSLAttribute("VertexColor", atVEC4, this));

    // Useful Matrices
    this->addUniform(new GLSLUniform("ModelMat", utMAT4, this->getHandle()));      // 0
    this->addUniform(new GLSLUniform("NormalMat", utMAT4, this->getHandle()));     // 1
    this->addUniform(new GLSLUniform("TexMat0", utMAT4, this->getHandle()));       // 2
    this->addUniform(new GLSLUniform("MVPMat", utMAT4, this->getHandle()));        // 3
    this->addUniform(new GLSLUniform("TexUnit0", utSAMPLER2D, this->getHandle())); // 4

    // Material Properties
    this->addUniform(new GLSLUniform("Material.Ka", utVEC4, this->getHandle()));             // 5
    this->addUniform(new GLSLUniform("Material.Kd", utVEC4, this->getHandle()));             // 6
    this->addUniform(new GLSLUniform("Material.Ks", utVEC4, this->getHandle()));             // 7
    this->addUniform(new GLSLUniform("Material.SpecularCoeff", utFLOAT, this->getHandle())); // 8

    // ShadowMap sampler
    this->addUniform(new GLSLUniform("ShadowMap", utSAMPLER2DSHADOW, this->getHandle())); // 9

    // Time Sampler
    this->addUniform(new GLSLUniform("OSTime", utFLOAT, this->getHandle())); // 10

    GLSLUniform *diffuseTexture = new GLSLUniform("diffuseTexture", utSAMPLER2D, this->getHandle());
    diffuseTexture->setValues((GLuint)0);
    // GLSLUniform *normalTexture = new GLSLUniform("normalTexture", utSAMPLER2D, this->getHandle());
    // normalTexture->setValues((GLuint)1);
    // GLSLUniform *specularTexture = new GLSLUniform("specularTexture", utSAMPLER2D, this->getHandle());
    // specularTexture->setValues((GLuint)2);
    GLSLUniform *displacementTexture = new GLSLUniform("displacementTexture", utSAMPLER2D, this->getHandle());
    displacementTexture->setValues((GLuint)1);
    GLSLUniform *groundTexture = new GLSLUniform("groundTexture", utSAMPLER2D, this->getHandle());
    groundTexture->setValues((GLuint)2);

    this->addUniform(diffuseTexture);      // 11
    this->addUniform(displacementTexture); // 12
    this->addUniform(groundTexture);       // 13

    // Camera Information
    this->addUniform(new GLSLUniform("camPos", utVEC3, this->getHandle()));  // 14
    this->addUniform(new GLSLUniform("camNorm", utVEC3, this->getHandle())); // 15
}

GLSLSnowShader::GLSLSnowShader(const GLSLSnowShader &toCopy) : GLSLShader(toCopy.dataShared)
{
    *this = toCopy;
}

GLSLSnowShader::~GLSLSnowShader()
{
    // Parent destructor deletes all uniforms and attributes
}

GLSLSnowShader &Aftr::GLSLSnowShader::operator=(const GLSLSnowShader &shader)
{
    if (this != &shader)
    {
        // copy all of parent info in base shader, then copy local members in this subclass instance
        GLSLShader::operator=(shader);

        // Now copy local members from this subclassed instance
        // this->mySubclassedLocalVariable = shader.mySubclassedLocalVariable
    }
    return *this;
}

GLSLShader *GLSLSnowShader::getCopyOfThisInstance()
{
    GLSLSnowShader *copy = new GLSLSnowShader(*this);
    return copy;
}

void GLSLSnowShader::bind(const Mat4 &modelMatrix, const Mat4 &normalMatrix, const Camera &cam, const ModelMeshSkin &skin)
{
    GLSLShader::bind(); // Must Bind this shader program handle to GL before updating any uniforms

    // update the shader. We must assume it has at least all the uniforms of the Default Shader
    const std::vector<GLSLUniform *> *uniforms = this->getUniforms();
    uniforms->at(0)->setValues(modelMatrix.getPtr());
    uniforms->at(1)->setValues(normalMatrix.getPtr());
    uniforms->at(2)->setValues(skin.getMultiTextureSet()[0].getTexMatrixWithRotScaleAndTrans().getPtr());
    Mat4 MVPMat = cam.getCameraProjectionMatrix() * cam.getCameraViewMatrix() * modelMatrix;
    uniforms->at(3)->setValues(MVPMat.getPtr());
    // uniforms->at( 4 ) //4 is  sampler2D TexUnit0, this is already set to the active texture bound to TEXTURE0

    // Now we populate the Material Properties taken from this skin and send them to the corresponding uniforms in the shader
    uniforms->at(5)->setValues(&(skin.getAmbient().r));
    uniforms->at(6)->setValues(&(skin.getDiffuse().r));
    uniforms->at(7)->setValues(&(skin.getSpecular().r));
    uniforms->at(8)->set(skin.getSpecularCoefficient());
    float time = ((int)SDL_GetTicks() % 10000) / 10000.0;
    // std::cout << time << std::endl;
    uniforms->at(10)->set(time);

    Vector camPos = cam.getPosition();
    Vector camNorm = cam.getNormalDirection();

    this->uniforms.at(14)->set(camPos.x, camPos.y, camPos.z);
    this->uniforms.at(15)->set(camNorm.x, camNorm.y, camNorm.z);

    // std::cout << "Shader info:\n" << this->toString() << "\n";
}

void GLSLSnowShader::setModelMatrix(const Mat4 &modelMatrix)
{
    this->getUniforms()->at(0)->setValues(modelMatrix.getPtr());
}
void GLSLSnowShader::setNormalMatrix(const Mat4 &normalMatrix)
{
    this->getUniforms()->at(1)->setValues(normalMatrix.getPtr());
}
void GLSLSnowShader::setTex0Matrix(const Mat4 &tex0Matrix)
{
    this->getUniforms()->at(2)->setValues(tex0Matrix.getPtr());
}
void GLSLSnowShader::setMVPMatrix(const Mat4 &mvpMatrix)
{
    this->getUniforms()->at(3)->setValues(mvpMatrix.getPtr());
}
void GLSLSnowShader::setMaterialAmbient(const aftrColor4f &materialAmbient)
{
    this->getUniforms()->at(5)->setValues(&materialAmbient.r);
}
void GLSLSnowShader::setMaterialDiffuse(const aftrColor4f &materialDiffuse)
{
    this->getUniforms()->at(6)->setValues(&materialDiffuse.r);
}
void GLSLSnowShader::setMaterialSpecular(const aftrColor4f &materialSpecular)
{
    this->getUniforms()->at(7)->setValues(&materialSpecular.r);
}
void GLSLSnowShader::setSpecularCoefficient(const float specularCoefficient)
{
    this->getUniforms()->at(8)->set(specularCoefficient);
}

void GLSLSnowShader::updateHeightMap(const Camera *cam)
{
    GLuint framebuffer;
    int heightmapWidth = 4096;
    int heightmapHeight = 4096;
    float terrainScale = 41;
    float radius = 30;
    Vector playerPosition = cam->getPosition();
    std::cout << this->parent->getModel()->getModelDataShared()->getModelMeshes().at(0)->getSkins().size() << std::endl;
    ModelMeshSkin &crazyBumpSkin = this->parent->getModel()->getModelDataShared()->getModelMeshes().at(0)->getSkins().at(1);
    GLuint heightmapTexture = crazyBumpSkin.getMultiTextureSet().at(1).getGLTex();
    // Create and bind a framebuffer object (FBO)
    GLuint fbo;
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    // Attach the heightmap texture to the framebuffer
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, heightmapTexture, 0);

    // Check if the framebuffer is complete
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        // Handle error
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glDeleteFramebuffers(1, &fbo);
        return;
    }

    // Read the current heightmap data
    float *heightmapData = new float[heightmapWidth * heightmapHeight];
    glReadPixels(0, 0, heightmapWidth, heightmapHeight, GL_RED, GL_FLOAT, heightmapData);
    Vector up(0, 0, 1);
    Vector forward = cam->getLookDirection();
    Vector right = forward.crossProduct(up);
    right.z = 0;
    right.normalize();
    right = right * 0.5;
    int centerY;
    int centerX;
    // Calculate the position on the heightmap based on the player's world coordinates
    if (leftStep)
    {
        centerY = static_cast<int>(abs(((playerPosition.y + right.x) * terrainScale) - 4096));
        centerX = static_cast<int>(abs(((playerPosition.x + right.y) * terrainScale) - 4096));
    }
    else
    {
        centerY = static_cast<int>(abs(((playerPosition.y - right.x) * terrainScale) - 4096));
        centerX = static_cast<int>(abs(((playerPosition.x - right.y) * terrainScale) - 4096));
    }

    std::cout << centerX << "," << centerY << std::endl;

    for (int dy = -radius; dy <= radius; ++dy)
    {
        for (int dx = -radius; dx <= radius; ++dx)
        {
            float distanceFromCir = (dx * dx + dy * dy) / (radius * radius);
            // Check if the current point is within the circular region
            if (distanceFromCir <= 1)
            {
                int x = centerX + dx;
                int y = centerY + dy;

                // Ensure that the calculated indices are within bounds
                if (x >= 0 && x < heightmapWidth && y >= 0 && y < heightmapHeight)
                {
                    // Modify the height at the current position (set it to 0 in this example)
                    // std::cout << x << "," << y << " Data:" << heightmapData[y * heightmapWidth + x] << std::endl;
                    float newvalue = abs(distanceFromCir - 1) + 0.2;
                    if (heightmapData[y * heightmapWidth + x] < newvalue)
                        heightmapData[y * heightmapWidth + x] = newvalue;
                }
            }
        }
    }
    // Write the modified heightmap data back to the texture
    int level = 0;
    int width;
    glBindTexture(GL_TEXTURE_2D, heightmapTexture);
    glGetTexLevelParameteriv(GL_TEXTURE_2D, level, GL_TEXTURE_WIDTH, &width);
    std::cout << "Texture width: " << width << std::endl;
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, heightmapWidth, heightmapHeight, GL_RED, GL_FLOAT, heightmapData);

    // Clean up
    delete[] heightmapData;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDeleteFramebuffers(1, &fbo);
    leftStep = !leftStep;
}