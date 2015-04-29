#include "App.h"
#include "config.h"
#include <Box2D/Box2D.h>

App::App(const GApp::Settings& settings) : GApp(settings) {
	renderDevice->setColorClearValue(Color3(0.2, 0.2, 0.2));
	renderDevice->setSwapBuffersAutomatically(true);
}


void App::onInit() {
	// Turn on the developer HUD
	createDeveloperHUD();
	debugWindow->setVisible(false);
	developerWindow->setVisible(false);
	developerWindow->cameraControlWindow->setVisible(false);
	showRenderingStats = false;

    //create Box2D world, setting gravity vector
	world = new b2World(b2Vec2(0, -9.8));
    

	// This load shaders from disk, we do it once when the program starts up, but
	// since you don't need to recompile to reload shaders, you can even do this
	// inteactively as you debug your shaders!  Press the R key to reload them
	// while your program is running!
	reloadShaders();

	// This is a simple manipulator for moving the camera around in the scene based on mouse movement
	turntable.reset(new TurntableManipulator());
	turntable->setEnabled(true);
	setCameraManipulator(turntable);
	addWidget(turntable);

	// Loading texture and setting wrap mode to "clamp" to prevent texels from
	// the left-hand column being blended with those from the rightmost column
	diffuseRamp = Texture::fromFile(DIFFUSE_RAMP, ImageFormat::AUTO(), Texture::DIM_2D);
	specularRamp = Texture::fromFile(SPECULAR_RAMP, ImageFormat::AUTO(), Texture::DIM_2D);
	noiseTex = Texture::fromFile(NOISE_TEX);
	paperTex = Texture::fromFile(PAPER_TEX);
	menuTex = Texture::fromFile(MENU_TEX);

	//Create background quad
	Array<Vector3> coords;
	Array<int> indices;
	Rect2D vp = renderDevice->viewport();
	coords.append(Vector3(0.0, 0.0, 0.0), Vector3(0.0, vp.height(), 0.0), Vector3(vp.width(), 0.0, 0.0), Vector3(vp.width(), vp.height(), 0.0));
	indices.append(0,1,2,3);
	shared_ptr<VertexBuffer> vdatabuf = VertexBuffer::create((sizeof(Vector3))*4, VertexBuffer::WRITE_ONCE);
	shared_ptr<VertexBuffer> vindexbuf = VertexBuffer::create(sizeof(int)*4, VertexBuffer::WRITE_ONCE);
	backgroundVerts = AttributeArray(coords, vdatabuf);
	backgroundIndices = IndexStream(indices, vindexbuf);
}


bool App::onEvent(const GEvent& e) {
	if (GApp::onEvent(e)) {
		return true;
	}
	// Press R to reload the shaders
	if (e.type == GEventType::KEY_DOWN && e.key.keysym.unicode == 'R') {
		reloadShaders();
		return true;
	}
	return false;
}


void App::onUserInput(UserInput *userInput) {
	GApp::onUserInput(userInput);

	// If the mouse is down, then add to the sketched path
	if (userInput->keyDown(GKey::LEFT_MOUSE)) {
		sketchedPath.append(userInput->mouseXY());
	}

	// When you release the mouse, then interpret the mouse movement as a menu selection or a sketched path
	if (userInput->keyReleased(GKey::LEFT_MOUSE)) {

		// Case 1: Mouse is on the top portion of the screen, so do menu selection
		// convert mouse position in pixels to a floating point number from 0.0 to 1.0
		Vector2 mouse0to1(userInput->mouseXY()[0]/userInput->window()->width(), userInput->mouseXY()[1]/userInput->window()->height());
		// If mouse Y coordinate is in top 15% of the screen, then mouse is inside the menu
		if (mouse0to1[1] < 0.15) {
			if (mouse0to1[0] < 0.33) {
				sketchMode = SKETCHING_BACKGROUND;
			}
			else if (mouse0to1[0] < 0.66) {
				sketchMode = SKETCHING_SPHERES;
			}
			else {
				sketchMode = SKETCHING_BOXES;
			}
			sketchedPath.clear();
		}


		// Case 2: Mouse is on the bottom portion of the screen, so interpret movement as drawing a line.
		else if (sketchedPath.size()) {
			Array<Vector3> sketched3DPath;
			Vector3 minP;
			Vector3 maxP;
			for (int i=0;i<sketchedPath.size();i++) {
				Vector3 p;
				Plane plane1(Vector3(0,0,1),Vector3(0,0,0));
				Plane plane2(Vector3(0,0,-1),Vector3(0,0,0));
				Ray ray = m_activeCamera->worldRay(sketchedPath[i][0], sketchedPath[i][1], renderDevice->viewport());
				Vector3 intersectionPoint = ray.intersection(plane1);
				if (!intersectionPoint.isFinite()) {
					intersectionPoint = ray.intersection(plane2);
					if (!intersectionPoint.isFinite()) {
						intersectionPoint = Vector3(0,0,0);
					}
				}
				p = intersectionPoint;

				// Uncomment the following two lines if you're working with 3D physics
				//Vector3 proj = m_activeCamera->project(Vector3(0,0,0), renderDevice->viewport());
				//p = m_activeCamera->unproject(Vector3(sketchedPath[i][0], sketchedPath[i][1], proj.z), renderDevice->viewport());


				sketched3DPath.append(p);
				if (i==0) {
					minP = p;
					maxP = p;
				}
				else {
					minP[0] = G3D::min(minP[0], p[0]);
					minP[1] = G3D::min(minP[1], p[1]);
					minP[2] = G3D::min(minP[2], p[2]);
					maxP[0] = G3D::max(maxP[0], p[0]);
					maxP[1] = G3D::max(maxP[1], p[1]);
					maxP[2] = G3D::max(maxP[2], p[2]);
				}
			}

			if (sketchMode == SKETCHING_SPHERES) {
				Vector3 center = minP + 0.5*(maxP - minP);
				float rad = (maxP - minP).length() / 2.0;
				if (rad < 100) {
					// TODO: add this sphere to the physics simulation
					//spheres.append(Sphere(center, rad));
                    addCircle(center, rad);
                }
			}
			else if (sketchMode == SKETCHING_BOXES) {
				float rad = (maxP - minP).length() / 2.0;
				if (rad < 100) {
					minP[2] = -0.2;
					maxP[2] = 0.2;
                    Vector3 position = (minP + maxP) / 2;
                    float width = abs(maxP.x - minP.x);
                    float height = abs(maxP.y - minP.y);
					// TODO: add this box to the physics simulation
					//boxes.append(Box(minP, maxP));
                    addBox(position, width, height);
                }
			}
			else {
				float rad = (maxP - minP).length() / 2.0;
				if (rad < 100) {
					Array<Vector2> polyline;
					Vector3 previousPoint;
					for (int x=0; x<sketched3DPath.size(); x++) {
						if (x==0 || (previousPoint-sketched3DPath[x]).magnitude() > 0.1) {
							polyline.append(sketched3DPath[x].xy());
							previousPoint = sketched3DPath[x];
						}
					}
                    addPolyline(polyline);
					// TODO: add this background shape to the physics simulation
					backgroundShapes.append(PolylineRenderer(polyline));
				}
			}

			sketchedPath.clear();
		}
	}
}

void App::addCircle(Vector3 position, float radius) {
    SimCircle simCircle;
    b2BodyDef bodyDef;
    bodyDef.type = b2_dynamicBody;
    bodyDef.position.Set(position.x, position.y);
    simCircle.body = world->CreateBody(&bodyDef);
    simCircle.radius = radius;
    
    b2CircleShape circle;
    circle.m_radius = radius;
    b2FixtureDef fixtureDef;
    fixtureDef.shape = &circle;
    fixtureDef.density = .2f;
    fixtureDef.friction = .3f;
    fixtureDef.restitution = 1.0f;
    simCircle.body->CreateFixture(&fixtureDef);
    
    circles.append(simCircle);
}

void App::addBox(Vector3 position, float width, float height){
    
    SimBox simBox;
    b2BodyDef bodyDef;
    bodyDef.type = b2_dynamicBody;
    bodyDef.position.Set(position.x, position.y);
    simBox.body = world->CreateBody(&bodyDef);
    simBox.width = width;
    simBox.height = height;
    
    b2PolygonShape boxShape;
    boxShape.SetAsBox(width/2, height/2);
    
    b2FixtureDef fixtureDef;
    fixtureDef.shape = &boxShape;
    fixtureDef.density = .2f;
    fixtureDef.friction = .3f;
    fixtureDef.restitution = 1.0f;
    simBox.body->CreateFixture(&fixtureDef);
    
    boxes.append(simBox);
}

void App::addPolyline(Array<Vector2> verts){
    int size = verts.size();
    if (size >=2) {
        Polyline polyline;
        b2BodyDef bodyDef;
        bodyDef.type = b2_staticBody;
    
        polyline.body = world->CreateBody(&bodyDef);
        polyline.size = size;
        b2Vec2 *vs = new b2Vec2[size];
        for(int i=0;i<size;i++){
            vs[i].Set(verts[i].x, verts[i].y);
        }
        b2ChainShape chainShape;
        chainShape.CreateChain(vs, size);
    
        b2FixtureDef fixtureDef;
        fixtureDef.shape = &chainShape;
        fixtureDef.density = .2f;
        fixtureDef.friction = .3f;
        fixtureDef.restitution = 1.0f;
        polyline.body->CreateFixture(&fixtureDef);
        polylines.append(polyline);
    }
}

void App::resetWorld() {
    delete world;
    world = new b2World(b2Vec2(0, -9.8));
    circles = Array<SimCircle>();
}

void App::onSimulation(RealTime rdt, SimTime sdt, SimTime idt) {
	GApp::onSimulation(rdt, sdt, idt);
    world->Step(1/60.0, 6, 2);
}


void App::reloadShaders() {
	shader = Shader::fromFiles(VERTEX_SHADER, FRAGMENT_SHADER);
	backgroundShader = Shader::fromFiles(BACKGROUND_VERTEX_SHADER, BACKGROUND_FRAGMENT_SHADER);
}


void App::onGraphics3D(RenderDevice* rd, Array<shared_ptr<Surface> >& surface3D) {
	rd->clear();


	// FIRST: Draw the background paper texture
	rd->push2D();
	Args backgroundArgs;
	Sampler s(WrapMode::TILE, InterpolateMode::BILINEAR_NO_MIPMAP);
	backgroundArgs.setUniform("paperTex", paperTex, s);
	backgroundArgs.setAttributeArray("g3d_Vertex", backgroundVerts);
	backgroundArgs.setIndexStream(backgroundIndices);
	backgroundArgs.setPrimitiveType(PrimitiveType::TRIANGLE_STRIP);
	rd->apply(backgroundShader, backgroundArgs);
	rd->pop2D();


	// SECOND: Draw the spheres, boxes, and background objects in the scene

	// Always good practice to start with a pushState
	rd->pushState();
	Args args;
	// Passing textures to the shader
	Sampler s2(WrapMode::CLAMP, InterpolateMode::BILINEAR_NO_MIPMAP);
	args.setUniform("diffuseRamp", diffuseRamp, s2);
	args.setUniform("specularRamp", specularRamp, s2);
	args.setUniform("noiseTex", noiseTex, s);
	args.setUniform("paperTex", paperTex, s);

	// Passing material properties
	args.setUniform("ambientReflectionCoeff", Color3(0.5,0.5,0.5));
	args.setUniform("diffuseReflectionCoeff", Color3(0.5,0.5,0.5));
	args.setUniform("specularReflectionCoeff", Color3(0.5,0.5,0.5));
	args.setUniform("specularExponent", (float)5.0);

	// Passing light properties
	args.setUniform("lightPosition", Vector4(10,10,10,1));  
	args.setUniform("ambientLightIntensity", Color3(0.3,0.3,0.3));
	args.setUniform("diffuseLightIntensity", Color3(0.8,0.8,0.8));
	args.setUniform("specularLightIntensity", Color3(0.5,0.5,0.5));

	Vector3 eyePosition = activeCamera()->frame().translation;
    args.setUniform("eyePosition", eyePosition);



	// TODO: you should change this to draw physics objects instead of stationary objects  
	

    // render circles
    for (int x=0; x<circles.size(); x++) {
        
        b2Vec2 pos2 = circles[x].body->GetPosition();
        float radius = circles[x].radius;
        Vector3 pos3(pos2.x, pos2.y, 0);
        createGeometryAndSetArgs(Sphere(pos3, radius), Color3(0.20,0.79,0.20), args);
        rd->apply(shader, args);
    
    }
 
	for (int i=0;i<boxes.size();i++) {
        b2Vec2 pos2 = boxes[i].body->GetPosition();
        float width = boxes[i].width;
        float height = boxes[i].height;
        
        Vector3 minP = Vector3(pos2.x - (width / 2), pos2.y - (height / 2), -0.2);
        Vector3 maxP = Vector3(pos2.x + (width / 2), pos2.y + (height / 2), 0.2);
        
		createGeometryAndSetArgs(Box(minP, maxP), Color3(0.44,0.52,0.93), args);
		rd->apply(shader, args);
	}
	for (int i=0;i<backgroundShapes.size();i++) {
		backgroundShapes[i].draw(rd, shader, args);
	}
	

	// Good practice to pop the state here since we just finished doing some complex rendering calls
	rd->popState();


	// THIRD: Draw the menu on the top of the screen
	rd->push2D();

	rd->pushState();  
	Draw::rect2D(Rect2D(Vector2(rd->width(), round(0.15f*rd->height()))), rd, Color3::white(), menuTex);
	rd->popState();

	rd->setBlendFunc(RenderDevice::BLEND_SRC_ALPHA, RenderDevice::BLEND_ONE_MINUS_SRC_ALPHA, RenderDevice::BLENDEQ_ADD);
	if (sketchMode == SKETCHING_BACKGROUND) {
		Draw::rect2D(Rect2D(Vector2(round(0.33f*rd->width()), round(0.15f*rd->height()))), rd, Color4(0.95,0.25,0.25,0.4));
	}
	else if (sketchMode == SKETCHING_SPHERES) {
		Draw::rect2D(Rect2D::xywh(round(0.33f*rd->width()), 0, round(0.33f*rd->width()), round(0.15f*rd->height())), rd, Color4(0.20,0.79,0.20,0.4));
	}
	else {
		Draw::rect2D(Rect2D::xywh(round(0.66f*rd->width()), 0, round(0.33f*rd->width()), round(0.15f*rd->height())), rd, Color4(0.44,0.52,0.93,0.4));
	}
	rd->pop2D();


	// FOURTH: Draw the 2D path that the mouse sketched on the screen
	rd->push2D();
	Array<Vector3> normals;
	Array<Color3> colors;
	Array<int> indices;
	Vector3 normal(0,0,1);
	for (int i=0;i<sketchedPath.size();i++) {
		normals.append(normal);
		colors.append(Color3::black());
		indices.append(i);
	}
	shared_ptr<VertexBuffer> vdatabuf = VertexBuffer::create((sizeof(Vector3)+sizeof(Vector3)+sizeof(Color3))*sketchedPath.size(), VertexBuffer::WRITE_ONCE);
	shared_ptr<VertexBuffer> vindexbuf = VertexBuffer::create(sizeof(int)*sketchedPath.size(), VertexBuffer::WRITE_ONCE);
	args.setAttributeArray("g3d_Vertex", AttributeArray(sketchedPath, vdatabuf));
	args.setAttributeArray("g3d_Normal", AttributeArray(normals, vdatabuf));
	args.setAttributeArray("color", AttributeArray(colors, vdatabuf));
	args.setPrimitiveType(PrimitiveType::LINE_STRIP);
	args.setIndexStream(IndexStream(indices, vindexbuf));
	rd->apply(shader, args);
	rd->pop2D();


	// Call to make the GApp show the output of debugDraw
	drawDebugShapes();
}


void App::onGraphics2D(RenderDevice* rd, Array<Surface2D::Ref>& posed2D) {
	Surface2D::sortAndRender(rd, posed2D);
}

void App::createGeometryAndSetArgs(const Box &b, Color3 color, Args &args) {

	Array<Vector3> vertices;
	Array<Vector3> normals;
	Array<Color3> colors;
	Array<int> indices;
	for (int i = 0; i < 6; ++i) {
        Vector3 v0, v1, v2, v3;
        b.getFaceCorners(i, v0, v1, v2, v3);

        Vector3 n = (v1 - v0).cross(v3 - v0);
		n = n.unit();
        vertices.append(v0, v1, v2, v0, v2, v3);
		normals.append(n, n, n, n, n, n);
		int base = indices.size();
		indices.append(base, base+1, base+2, base+3, base+4, base+5);
		colors.append(color, color, color, color, color, color);
    }
	shared_ptr<VertexBuffer> vdatabuf = VertexBuffer::create((sizeof(Vector3)+sizeof(Vector3)+sizeof(Color3))*vertices.size(), VertexBuffer::WRITE_ONCE);
	shared_ptr<VertexBuffer> vindexbuf = VertexBuffer::create(sizeof(int)*indices.size(), VertexBuffer::WRITE_ONCE);
	args.setAttributeArray("g3d_Vertex", AttributeArray(vertices, vdatabuf));
	args.setAttributeArray("g3d_Normal", AttributeArray(normals, vdatabuf));
	args.setAttributeArray("color", AttributeArray(colors, vdatabuf));
	args.setPrimitiveType(PrimitiveType::TRIANGLES);
	args.setIndexStream(IndexStream(indices, vindexbuf));
}

void App::createGeometryAndSetArgs(const Sphere &s, Color3 color, Args &args) {
	Array<Vector3> vertices;
	Array<Vector3> normals;
	Array<Color3> colors;
	Array<int> indices;
	
	const int SLICES = 40;
	const int STACKS = 20;
	for (int p = 0; p < STACKS; ++p) {
        const float pitch0 = p * (float)pi() / (STACKS);
        const float pitch1 = (p + 1) * (float)pi() / (STACKS);

        const float sp0 = sin(pitch0);
        const float sp1 = sin(pitch1);
        const float cp0 = cos(pitch0);
        const float cp1 = cos(pitch1);

        for (int y = 0; y <= SLICES; ++y) {
            const float yaw = -y * (float)twoPi() / SLICES;

            const float cy = cos(yaw);
            const float sy = sin(yaw);

			Vector3 v0(cy * sp0, cp0, sy * sp0);
            Vector3 v1(cy * sp1, cp1, sy * sp1);
			normals.append(v0.unit(), v1.unit());
			v0 = (s.radius*v0) + s.center;
			v1 = (s.radius*v1) + s.center;

            vertices.append(v0, v1);
			colors.append(color, color);
                    
        }

        Vector3 degen(1.0f * sp1, cp1, 0.0f * sp1);
		degen = (s.radius*degen)+s.center;
        vertices.append(degen, degen);
		normals.append(degen, degen);
		colors.append(color, color);
                  
    }

	for(int i=0; i < vertices.size(); i++){
		indices.append(i);
	}

	shared_ptr<VertexBuffer> vdatabuf = VertexBuffer::create((sizeof(Vector3)+sizeof(Vector3)+sizeof(Color3))*vertices.size(), VertexBuffer::WRITE_ONCE);
	shared_ptr<VertexBuffer> vindexbuf = VertexBuffer::create(sizeof(int)*indices.size(), VertexBuffer::WRITE_ONCE);
	args.setAttributeArray("g3d_Vertex", AttributeArray(vertices, vdatabuf));
	args.setAttributeArray("g3d_Normal", AttributeArray(normals, vdatabuf));
	args.setAttributeArray("color", AttributeArray(colors, vdatabuf));
	args.setPrimitiveType(PrimitiveType::TRIANGLE_STRIP);
	args.setIndexStream(IndexStream(indices, vindexbuf));
}



