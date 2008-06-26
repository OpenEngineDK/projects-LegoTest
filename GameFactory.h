// Game Factory for Lego test.
// -------------------------------------------------------------------
// Copyright (C) 2007 OpenEngine.dk (See AUTHORS) 
// 
// This program is free software; It is covered by the GNU General 
// Public License version 2 or any later version. 
// See the GNU General Public License for more details (see LICENSE). 
//--------------------------------------------------------------------

// header define to avoid including definitions several times
#ifndef _LEGOTEST_PROJECT_GF_H_
#define _LEGOTEST_PROJECT_GF_H_

// Game engine interface
#include <Core/IGameFactory.h>
#include <Scene/ISceneNode.h>
#include <Display/Camera.h>


// Namespaces
using namespace OpenEngine::Core;
using namespace OpenEngine::Display;
using namespace OpenEngine::Renderers;

/**
 * Game factory definition.
 *
 * The factory is implemented in GameFactory.cpp
 * Documentation on game factories can be found in Core/IGameFactory.h
 * 
 * @see IGameFactory
 */
class GameFactory : public IGameFactory {

private:
    IFrame*      frame;         // member variable for the engine frame
    IRenderer*   renderer;      // member variable for the engine renderer
    Camera*      camera;
public:
    GameFactory();
    bool         SetupEngine(IGameEngine& engine);
    IFrame*      GetFrame();
    IRenderer*   GetRenderer();

};

#endif // _LEGOTEST_PROJECT_GF_H_
