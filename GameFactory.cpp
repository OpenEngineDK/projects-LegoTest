// Game Factory for an example project.
// -------------------------------------------------------------------
// Copyright (C) 2007 OpenEngine.dk (See AUTHORS) 
// 
// This program is free software; It is covered by the GNU General 
// Public License version 2 or any later version. 
// See the GNU General Public License for more details (see LICENSE). 
//--------------------------------------------------------------------

// Class header
#include "GameFactory.h"

// OpenEngine library
#include <Display/Viewport.h>
#include <Display/ViewingVolume.h>
#include <Display/SDLFrame.h>

#include <Display/Frustum.h>
#include <Renderers/OpenGL/Renderer.h>
#include <Renderers/OpenGL/RenderingView.h>
#include <Scene/SceneNode.h>
#include <Scene/PointLightNode.h>
#include <Scene/TransformationNode.h>
#include <Scene/SimpleTransformationNode.h>
#include <Utils/Statistics.h>
#include <Resources/TGAResource.h>
#include <Resources/OBJResource.h>
#include <Resources/LDRAWResource.h>
#include <Resources/DirectoryManager.h>
#include <Resources/ResourceManager.h>
#include <Devices/SDLInput.h>

// Example extension
#include <Particles/ParticleSystem.h>
#include <Particles/ParticleGroups.h>
#include <Particles/Particles.h>
#include <Particles/Modifiers.h>
#include <Math/Interpolators.h>
#include <Particles/PointEmitter.h>
#include <Utils/PropertyList.h>

// Project files
#include "DownCameraEventHandler.h"

#include "CustomLevel.h"
#include "QuitHandler.h"

// Additional namespaces (others are in the header).
using namespace OpenEngine::Devices;
using namespace OpenEngine::Renderers::OpenGL;
using namespace OpenEngine::Particles;
using namespace OpenEngine::Math;
using namespace OpenEngine::Utils;
using namespace OpenEngine::Display;
using OpenEngine::EventHandlers::DownCameraEventHandler;
using namespace OpenEngine::Level;

/**
 * Game quit handler.
 *
 * This is an example of a keyboard listener.
 * Every time the a key is released a key (UP) event is sent to the
 * handle method. If the key symbol is escape it quits the game.
 *
 * Note that this will not work before you have implemented the
 * keyboard and mouse interfaces in SDLInput.
 *
 * The code to register the handler is found in the
 * GameFactory::SetupEngine method.
 */
class QuitEventHandler {
public:
    void HandleQuit(KeyboardEventArg arg) {
        if (arg.sym == KEY_ESCAPE) {
            //OpenEngine::Hello::World();
            IGameEngine::Instance().Stop();
        }
    }
};


class MyRenderingView : public RenderingView {

public:
    MyRenderingView(Viewport& viewport)
    : IRenderingView(viewport),
        RenderingView(viewport) {
        
    }
    
    void VisitSimpleTransformationNode(SimpleTransformationNode* node) {
            // push transformation matrix
        Matrix<4,4,float> m = node->GetTransformationMatrix();
        float f[16];
        m.ToArray(f);
        glPushMatrix();
        glMultMatrixf(f);
        // traverse sub nodes
        node->VisitSubNodes(*this);
        // pop transformation matrix
        glPopMatrix();
    }
    

};

#define WINDOW_WIDTH 1024
#define WINDOW_HEIGHT 768

/**
 * Factory constructor.
 *
 * Initializes the different components so they are accessible when
 * the setup method is executed.
 */
GameFactory::GameFactory() {
    
    // Create a frame and viewport.
    this->frame = new SDLFrame(WINDOW_WIDTH, WINDOW_HEIGHT, 32);
    
    // Main viewport
    Viewport* viewport = new Viewport(*frame);
    camera = new Camera(*(new ViewingVolume()));
    camera->SetPosition(Vector<3,float>(-100,0,-10));
    camera->LookAt(Vector<3,float>(0,0,0));
    
    // Bind the camera to the viewport
    Frustum* frustum = new Frustum(*camera);
    frustum->SetFar(1000);
    viewport->SetViewingVolume(frustum);
    
    
    // Create a renderer.
    this->renderer = new Renderer();
    this->renderer->SetFarPlane(50000.0);
    this->renderer->initialize.Attach(*(new TextureLoader())); // space leak

    // Add a rendering view to the renderer
    this->renderer->process.Attach(*(new MyRenderingView(*viewport)));
}

/**
 * Setup handler.
 *
 * This is the main setup method of the game factory. Here you can add
 * any non-standard modules and perform other setup tasks prior to the
 * engine start up.
 *
 * @param engine The game engine instance.
 */
bool GameFactory::SetupEngine(IGameEngine& engine) {
    
#define ParticleType BillBoardParticle<EnergyParticle<DirectionParticle<IParticle> > >
#define GroupType EnergyParticleGroup<ParticleType >
    

    DirectoryManager::AppendPath("./projects/PatSys/data/");
    DirectoryManager::AppendPath("./projects/LegoTest/data/");

    DirectoryManager::AppendPath("./projects/LegoTest/data/ldraw/parts/");
    DirectoryManager::AppendPath("./projects/LegoTest/data/ldraw/p/");
    DirectoryManager::AppendPath("./projects/LegoTest/data/ldraw/models/");
    
    ResourceManager<ITextureResource>::AddPlugin(new TGAPlugin());
    ResourceManager<IModelResource>::AddPlugin(new OBJPlugin());
    ResourceManager<IModelResource>::AddPlugin(new LDRAWPlugin());
    
    // Add your mouse and keyboard module here
    SDLInput* input = new SDLInput();

        
    
    DownCameraEventHandler* cameraHandler = new DownCameraEventHandler(camera,
                                                                       this);

    cameraHandler->BindToEventSystem();

		
    engine.AddModule(*input);
    engine.AddModule(*cameraHandler);
    
    
    
    
    // Create a root scene node
    SceneNode* scene = new SceneNode();
    
    
    PropertyList* pSimple = new PropertyList("levels/simple.lvl");
    CustomLevel* level = new CustomLevel(*pSimple);
    level->Load();
    //SceneNode* physNode = level->GetPhysicsGraph();
    SceneNode* staticScene = level->GetSceneGraph();
    
    scene->AddNode(staticScene);

    TransformationNode* lightTrans = new TransformationNode();

    PointLightNode* pl = new PointLightNode();
    
    pl->ambient = Vector<4,float>(0,0,0,1);
    pl->diffuse = Vector<4,float>(1,1,1,1);
    pl->specular = Vector<4,float>(0,0,0,1);

    lightTrans->AddNode(pl);
    lightTrans->Move(0,10,0);

    scene->AddNode(lightTrans);

    IModelResourcePtr res = ResourceManager<IModelResource>::Create("car.dat");
    res->Load();
    TransformationNode* tnode = new TransformationNode();
    tnode->Scale(.5, .5, .5);
    tnode->Move(0,-20,0);
    tnode->AddNode(res->GetSceneNode());
    res->Unload();
    
    scene->AddNode(tnode);

    // Supply the scene to the renderer
    this->renderer->SetSceneRoot(scene);
    
    QuitHandler* quit_h = new QuitHandler();
    quit_h->BindToEventSystem();
    


    
    // Add some module
    engine.AddModule(*(new OpenEngine::Utils::Statistics(1000)));
    
    // Return true to signal success.
    return true;
    
}

// Other factory methods. The returned objects are all created in the
// factory constructor.
IFrame*      GameFactory::GetFrame()    { return this->frame;    }
IRenderer*   GameFactory::GetRenderer() { return this->renderer; }
