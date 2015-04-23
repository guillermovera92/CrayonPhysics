#ifndef PolylineRenderer_h
#define PolylineRenderer_h

#include <G3D/G3DAll.h>

class PolylineRenderer /* : public  ReferenceCountedObject */ {
public:
	PolylineRenderer() {}
	PolylineRenderer(Array<Vector2> points, bool smooth=false, float thickness=0.04, float depth=0.5);
	void draw(RenderDevice *rd, shared_ptr<Shader> shader, Args &args);
protected:
	Vector2 getNormal(int index, Array<Vector2> points);

	shared_ptr<VertexBuffer> vdatabuf;

	shared_ptr<VertexBuffer> vindexbuf;

	AttributeArray vnormals;

	AttributeArray vcoords;

	AttributeArray vcolors;

	IndexStream vindices;
};

#endif