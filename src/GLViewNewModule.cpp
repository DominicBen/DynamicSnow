#include "GLViewNewModule.h"

#include "WorldList.h"          //This is where we place all of our WOs
#include "ManagerOpenGLState.h" //We can change OpenGL State attributes with this
#include "Axes.h"               //We can set Axes to on/off with this
#include "PhysicsEngineODE.h"

// Different WO used by this module
#include "WO.h"
#include "WOStatic.h"
#include "WOStaticPlane.h"
#include "WOStaticTrimesh.h"
#include "WOTrimesh.h"
#include "WOHumanCyborg.h"
#include "WOHumanCal3DPaladin.h"
#include "WOWayPointSpherical.h"
#include "WOLight.h"
#include "WOSkyBox.h"
#include "WOCar1970sBeater.h"
#include "Camera.h"
#include "CameraStandard.h"
#include "CameraChaseActorSmooth.h"
#include "CameraChaseActorAbsNormal.h"
#include "CameraChaseActorRelNormal.h"
#include "Model.h"
#include "ModelDataShared.h"
#include "ModelMesh.h"
#include "ModelMeshDataShared.h"
#include "ModelMeshSkin.h"
#include "WONVStaticPlane.h"
#include "WONVPhysX.h"
#include "WONVDynSphere.h"
#include "WOImGui.h" //GUI Demos also need to #include "AftrImGuiIncludes.h"
#include "AftrImGuiIncludes.h"
#include "AftrGLRendererBase.h"
#include "ManagerTex.h"
#include "GLSLShader.h"
#include "GLSLUniform.h"
#include "GLSLAttribute.h"
#include "GLSLSnowShader.h"
#include "SDL.h"

using namespace Aftr;

GLSLSnowShader *shader = NULL;
float horizontalInput = 0;
float verticalInput = 0;

Vector startingPosition(0, 0, 3);
Vector lastPos = startingPosition;

GLViewNewModule *
GLViewNewModule::New(const std::vector<std::string> &args)
{
   GLViewNewModule *glv = new GLViewNewModule(args);
   glv->init(Aftr::GRAVITY, Vector(0, 0, -1.0f), "aftr.conf", PHYSICS_ENGINE_TYPE::petODE);
   glv->onCreate();
   return glv;
}

GLViewNewModule::GLViewNewModule(const std::vector<std::string> &args) : GLView(args)
{
   // Initialize any member variables that need to be used inside of LoadMap() here.
   // Note: At this point, the Managers are not yet initialized. The Engine initialization
   // occurs immediately after this method returns (see GLViewNewModule::New() for
   // reference). Then the engine invoke's GLView::loadMap() for this module.
   // After loadMap() returns, GLView::onCreate is finally invoked.

   // The order of execution of a module startup:
   // GLView::New() is invoked:
   //     calls GLView::init()
   //        calls GLView::loadMap() (as well as initializing the engine's Managers)
   //     calls GLView::onCreate()

   // GLViewNewModule::onCreate() is invoked after this module's LoadMap() is completed.
}

void GLViewNewModule::onCreate()
{
   // GLViewNewModule::onCreate() is invoked after this module's LoadMap() is completed.
   // At this point, all the managers are initialized. That is, the engine is fully initialized.

   if (this->pe != NULL)
   {
      // optionally, change gravity direction and magnitude here
      // The user could load these values from the module's aftr.conf
      this->pe->setGravityNormalizedVector(Vector(0, 0, -1.0f));
      this->pe->setGravityScalar(Aftr::GRAVITY);
   }
   this->setActorChaseType(STANDARDEZNAV); // Default is STANDARDEZNAV mode
   // this->setNumPhysicsStepsPerRender( 0 ); //pause physics engine on start up; will remain paused till set to 1
}

GLViewNewModule::~GLViewNewModule()
{
   // Implicitly calls GLView::~GLView()
}

void GLViewNewModule::updateWorld()
{
   GLView::updateWorld(); // Just call the parent's update world first.
                          // If you want to add additional functionality, do it after
                          // this call.
   Vector up(0, 0, 1);
   Vector forward = this->cam->getLookDirection();
   Vector right = forward.crossProduct(up);
   float moveSpeed = 0.1;
   forward.z = 0.0f;
   right.z = 0.0f;
   forward.normalize();
   right.normalize();
   Vector moveDirection = (forward * verticalInput + right * horizontalInput) * moveSpeed;
   cam->setPosition(cam->getPosition() + moveDirection);

   float updateFrequency = 2;

   if (cam->getPosition().distanceFrom(lastPos) >= updateFrequency)
   {
      shader->updateHeightMap(this->cam);
      lastPos = cam->getPosition();
   }
}

void GLViewNewModule::onResizeWindow(GLsizei width, GLsizei height)
{
   GLView::onResizeWindow(width, height); // call parent's resize method.
}

void GLViewNewModule::onMouseDown(const SDL_MouseButtonEvent &e)
{
   GLView::onMouseDown(e);
}

void GLViewNewModule::onMouseUp(const SDL_MouseButtonEvent &e)
{
   GLView::onMouseUp(e);
}

void GLViewNewModule::onMouseMove(const SDL_MouseMotionEvent &e)
{
   GLView::onMouseMove(e);
}

void GLViewNewModule::onKeyDown(const SDL_KeyboardEvent &key)
{

   GLView::onKeyDown(key);
   if (key.keysym.sym == SDLK_0)
      this->setNumPhysicsStepsPerRender(1);

   if (key.keysym.sym == SDLK_1)
   {
      shader->updateHeightMap(this->cam);
   }
   if (key.keysym.sym == SDLK_w)
   {
      verticalInput = 1;
   }
   if (key.keysym.sym == SDLK_a)
   {
      horizontalInput = -1;
   }
   if (key.keysym.sym == SDLK_s)
   {
      verticalInput = -1;
   }
   if (key.keysym.sym == SDLK_d)
   {
      horizontalInput = 1;
   }
}

void GLViewNewModule::onKeyUp(const SDL_KeyboardEvent &key)
{
   GLView::onKeyUp(key);

   if (key.keysym.sym == SDLK_w)
   {
      verticalInput = 0;
   }
   if (key.keysym.sym == SDLK_a)
   {
      horizontalInput = 0;
   }
   if (key.keysym.sym == SDLK_s)
   {
      verticalInput = 0;
   }
   if (key.keysym.sym == SDLK_d)
   {
      horizontalInput = 0;
   }
}

void Aftr::GLViewNewModule::loadMap()
{
   this->worldLst = new WorldList(); // WorldList is a 'smart' vector that is used to store WO*'s
   this->actorLst = new WorldList();
   this->netLst = new WorldList();

   ManagerOpenGLState::GL_CLIPPING_PLANE = 1000.0;
   ManagerOpenGLState::GL_NEAR_PLANE = 0.1f;
   ManagerOpenGLState::enableFrustumCulling = false;
   Axes::isVisible = false;
   this->glRenderer->isUsingShadowMapping(false); // set to TRUE to enable shadow mapping, must be using GL 3.2+

   this->cam->setPosition(startingPosition);

   std::string shinyRedPlasticCube(ManagerEnvironmentConfiguration::getSMM() + "/models/cube4x4x4redShinyPlastic_pp.wrl");
   // std::string snowField(ManagerEnvironmentConfiguration::getLMM() + "models/snow_02_4k.obj");
   std::string wheeledCar(ManagerEnvironmentConfiguration::getSMM() + "/models/rcx_treads.wrl");
   std::string grass(ManagerEnvironmentConfiguration::getSMM() + "/models/grassFloor400x400_pp.wrl");
   std::string human(ManagerEnvironmentConfiguration::getSMM() + "/models/human_chest.wrl");

   std::string snowDiffuse(ManagerEnvironmentConfiguration::getLMM() + "/images/snow_02_diff_4k.jpg");
   std::string snowDisplacement(ManagerEnvironmentConfiguration::getLMM() + "/images/snow_02_disp_4k.png");
   std::string groundDiffuse(ManagerEnvironmentConfiguration::getLMM() + "/images/forestGround.jpg");

   // SkyBox Textures readily available
   std::vector<std::string> skyBoxImageNames; // vector to store texture paths
   // skyBoxImageNames.push_back( ManagerEnvironmentConfiguration::getSMM() + "/images/skyboxes/sky_water+6.jpg" );
   // skyBoxImageNames.push_back( ManagerEnvironmentConfiguration::getSMM() + "/images/skyboxes/sky_dust+6.jpg" );
   skyBoxImageNames.push_back(ManagerEnvironmentConfiguration::getSMM() + "/images/skyboxes/sky_mountains+6.jpg");

   {
      // Create a light
      float ga = 0.1f; // Global Ambient Light level for this module
      ManagerLight::setGlobalAmbientLight(aftrColor4f(ga, ga, ga, 1.0f));
      WOLight *light = WOLight::New();
      light->isDirectionalLight(true);
      light->setPosition(Vector(0, 0, 100));
      // Set the light's display matrix such that it casts light in a direction parallel to the -z axis (ie, downwards as though it was "high noon")
      // for shadow mapping to work, this->glRenderer->isUsingShadowMapping( true ), must be invoked.
      light->getModel()->setDisplayMatrix(Mat4::rotateIdentityMat({0, 1, 0}, 90.0f * Aftr::DEGtoRAD));
      light->setLabel("Light");
      worldLst->push_back(light);
   }

   {
      // Create the SkyBox
      WO *wo = WOSkyBox::New(skyBoxImageNames.at(0), this->getCameraPtrPtr());
      wo->setPosition(Vector(0, 0, 0));
      wo->setLabel("Sky Box");
      wo->renderOrderType = RENDER_ORDER_TYPE::roOPAQUE;
      worldLst->push_back(wo);
   }

   {
      ////Create the infinite grass plane (the floor)

      // WO *wo = WOGrid::New(mesh, Vector(1, 1, 1), colors);

      WO *wo = WO::New(grass, Vector(1, 1, 1), MESH_SHADING_TYPE::mstFLAT);

      wo->setPosition(Vector(0, 0, 0));
      wo->renderOrderType = RENDER_ORDER_TYPE::roOPAQUE;
      wo->upon_async_model_loaded([wo, snowDiffuse, snowDisplacement, groundDiffuse]()
                                  {
                                     auto colorTex = ManagerTex::loadTexAsync(snowDiffuse);
                                     auto dispTex = ManagerTex::loadTexAsync(snowDisplacement);
                                     auto groundTex = ManagerTex::loadTexAsync(groundDiffuse);

                                     ModelMeshSkin &crazyBumpSkin = wo->getModel()->getModelDataShared()->getModelMeshes().at(0)->getSkins().at(0);
                                     crazyBumpSkin.setMeshShadingType(MESH_SHADING_TYPE::mstFLAT);
                                     crazyBumpSkin.getMultiTextureSet().pop_back();
                                     crazyBumpSkin.getMultiTextureSet().push_back(std::move(*colorTex));
                                     crazyBumpSkin.getMultiTextureSet().push_back(std::move(*dispTex));
                                     crazyBumpSkin.getMultiTextureSet().push_back(std::move(*groundTex));
                                    //   crazyBumpSkin.getMultiTextureSet().at(0).setTexRepeats(1.0f);
                                    //   crazyBumpSkin.getMultiTextureSet().at(1).setTexRepeats(1.0f);
                                                           
                                     std::cout << crazyBumpSkin.getMultiTextureSet().size() << "==========================" << std::endl;
                                     std::cout << "=================================================================================" << std::endl;
                                     std::cout << "==========================" << std::endl;
                                     shader = GLSLSnowShader::New(wo);
                                      crazyBumpSkin.setShader(shader);
                                      
                                      wo->getModel()->getSkins().push_back(std::move(crazyBumpSkin));
                                      wo->getModel()->useNextSkin(); });
      wo->setLabel("Grass");

      worldLst->push_back(wo);
   }

   // {

   //    const std::string path = ManagerEnvironmentConfiguration::getSMM() + "models/WOStoneWallBump/";
   //    WO *wo = WO::New(path + "WOStoneWallBump.wrl", Vector(7, 7, 7), MESH_SHADING_TYPE::mstFLAT);
   //    wo->upon_async_model_loaded([wo, path]()
   //                                {
   //                                   //  auto colorTex = ManagerTex::loadTexAsync(path + "stoneWall_COLOR.png");
   //                                   //  auto nrmTex = ManagerTex::loadTexAsync(path + "stoneWall_NRM.png");
   //                                   //  auto specTex = ManagerTex::loadTexAsync(path + "stoneWall_SPEC.png");
   //                                   //  auto dispTex = ManagerTex::loadTexAsync(path + "stoneWall_DISP.png");

   //                                   ModelMeshSkin &crazyBumpSkin = wo->getModel()->getModelDataShared()->getModelMeshes().at(0)->getSkins().at(0);
   //                                   //  crazyBumpSkin.setMeshShadingType(MESH_SHADING_TYPE::mstFLAT);
   //                                   //  crazyBumpSkin.getMultiTextureSet().push_back(std::move(*colorTex));
   //                                   //  crazyBumpSkin.getMultiTextureSet().push_back(std::move(*nrmTex));
   //                                   //  crazyBumpSkin.getMultiTextureSet().push_back(std::move(*specTex));
   //                                   //  crazyBumpSkin.getMultiTextureSet().push_back(std::move(*dispTex));
   //                                   std::cout << crazyBumpSkin.getMultiTextureSet().size() << "==========================" << std::endl;
   //                                   std::cout << "=================================================================================" << std::endl;
   //                                   std::cout << "==========================" << std::endl;
   //                                   //  crazyBumpSkin.setShader(GLSLSnowShader::New());
   //                                   //  crazyBumpSkin.setShader(ManagerShader::loadShaderCrazyBumpParallaxMapping());
   //                                   //  wo->getModel()->getSkins().push_back(std::move(crazyBumpSkin));
   //                                   //  wo->getModel()->useNextSkin();
   //                                });
   //    wo->setPosition(Vector(0, 0, 20));
   //    wo->setLabel("Stone Wall");
   //    worldLst->push_back(wo);
   // }

   // createNewModuleWayPoints();
}

void GLViewNewModule::createNewModuleWayPoints()
{
   // Create a waypoint with a radius of 3, a frequency of 5 seconds, activated by GLView's camera, and is visible.
   WayPointParametersBase params(this);
   params.frequency = 5000;
   params.useCamera = true;
   params.visible = true;
   WOWayPointSpherical *wayPt = WOWayPointSpherical::New(params, 3);
   wayPt->setPosition(Vector(50, 0, 3));
   worldLst->push_back(wayPt);
}
