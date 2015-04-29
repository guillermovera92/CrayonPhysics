#ifndef App_h
#define App_h

#include <G3D/G3DAll.h>
#include "TurntableManipulator.h"
#include "PolyLineRenderer.h"
#include <Box2D/Box2D.h>


struct SimCircle {
    float radius;
    b2Body *body;
};

struct SimSquare {
    float length;
    b2Body *body;
};

class App : public GApp {
public:

	App(const GApp::Settings& settings = GApp::Settings());

	virtual void onInit();
	virtual void onSimulation(RealTime rdt, SimTime sdt, SimTime idt);
	virtual void onGraphics3D(RenderDevice* rd, Array<shared_ptr<Surface> >& surface);
	virtual void onGraphics2D(RenderDevice* rd, Array<shared_ptr<Surface2D> >& surface2D);

	virtual bool onEvent(const GEvent& e);
	virtual void onUserInput(UserInput *userInput);

protected:
    
    b2World *world;
    
    Array<SimCircle> circles;

    virtual void addCircle(Vector3 position, float radius);
    virtual void resetWorld();;
    
	virtual void reloadShaders();
	virtual void createGeometryAndSetArgs(const Box &b, Color3 color, Args &args);
	virtual void createGeometryAndSetArgs(const Sphere &s, Color3 color, Args &args);

	shared_ptr<Texture> diffuseRamp;
	shared_ptr<Texture> specularRamp;
	shared_ptr<Texture> noiseTex;
	shared_ptr<Texture> paperTex;
	shared_ptr<Texture> menuTex;  

	shared_ptr<Shader> backgroundShader;
	shared_ptr<Shader> shader;

	AttributeArray backgroundVerts;
	IndexStream	backgroundIndices;

	shared_ptr<TurntableManipulator> turntable;

	enum SketchMode {
		SKETCHING_BACKGROUND = 0,
		SKETCHING_SPHERES = 1,
		SKETCHING_BOXES = 2
	};
	SketchMode              sketchMode;
	Array<Vector2>          sketchedPath;  

	//Array<Sphere>           spheres;
	Array<Box>              boxes;  
	Array< PolylineRenderer > backgroundShapes;
};

#endif