#include "PolylineRenderer.h"

PolylineRenderer::PolylineRenderer(Array<Vector2> points, bool smooth, float thickness, float depth) {
	Array<int> indices;
	Array<Vector3> coords;
	Array<Vector3> normals;
	Array<Color3> colors;
	int index = 0;

	Color3 color(0.9, 0.5, 0.2);

	for (int i=0; i<points.size()-1; i++) {
		Vector2 n1 = getNormal(i, points);
		Vector2 n2 = getNormal(i+1, points);

		Vector2 p1 = points[i] + n1 * thickness/2;
		Vector2 p2 = points[i+1] + n2 * thickness/2;
		Vector2 p3 = points[i+1] - n2 * thickness/2;
		Vector2 p4 = points[i] - n1 * thickness/2;

		Vector2 norm = (points[i+1]-points[i]).direction();
		Vector3 normal1(-norm.y, norm.x, 0);
		Vector3 normal2(-norm.y, norm.x, 0);
		if (smooth) {
			normal1 = Vector3(n1.x, n1.y, 0);
			normal2 = Vector3(n2.x, n2.y, 0);
		}
		Vector3 znormal(0,0,1);

		coords.append(Vector3(p1.x, p1.y, -depth/2));
		coords.append(Vector3(p2.x, p2.y, -depth/2));
		coords.append(Vector3(p3.x, p3.y, -depth/2));
		coords.append(Vector3(p1.x, p1.y, -depth/2));
		coords.append(Vector3(p3.x, p3.y, -depth/2));
		coords.append(Vector3(p4.x, p4.y, -depth/2));

		normals.append(-znormal,-znormal,-znormal,-znormal, -znormal,-znormal);
		indices.append(index, index+1, index+2, index+3, index+4, index+5);
		index += 6;

		coords.append(Vector3(p1.x, p1.y, -depth/2));
		coords.append(Vector3(p1.x, p1.y, depth/2));
		coords.append(Vector3(p2.x, p2.y, depth/2));
		coords.append(Vector3(p1.x, p1.y, -depth/2));
		coords.append(Vector3(p2.x, p2.y, depth/2));
		coords.append(Vector3(p2.x, p2.y, -depth/2));

		normals.append(normal1,normal1,normal2,normal1, normal2, normal2);
		indices.append(index, index+1, index+2, index+3, index+4, index+5);
		index += 6;

		coords.append(Vector3(p1.x, p1.y, depth/2));
		coords.append(Vector3(p4.x, p4.y, depth/2));
		coords.append(Vector3(p3.x, p3.y, depth/2));
		coords.append(Vector3(p1.x, p1.y, depth/2));
		coords.append(Vector3(p3.x, p3.y, depth/2));
		coords.append(Vector3(p2.x, p2.y, depth/2));

		normals.append(znormal,znormal,znormal,znormal, znormal, znormal);
		indices.append(index, index+1, index+2, index+3, index+4, index+5);
		index += 6;

		coords.append(Vector3(p3.x, p3.y, -depth/2));
		coords.append(Vector3(p3.x, p3.y, depth/2));
		coords.append(Vector3(p4.x, p4.y, depth/2));
		coords.append(Vector3(p3.x, p3.y, -depth/2));
		coords.append(Vector3(p4.x, p4.y, depth/2));
		coords.append(Vector3(p4.x, p4.y, -depth/2));
		normals.append(-normal2,-normal2,-normal1,-normal2, -normal1, -normal1);
		indices.append(index, index+1, index+2, index+3, index+4, index+5);
		index += 6;


	}

	for (int i=0; i < coords.size(); i++) {
		colors.append(color);
	}

	vdatabuf = VertexBuffer::create(
		sizeof(Vector3) * coords.size()
		+ sizeof(Vector3) * normals.size() + sizeof(Color3) * colors.size(),
		VertexBuffer::WRITE_ONCE);

	vindexbuf = VertexBuffer::create(sizeof(int)*indices.size(),
		VertexBuffer::WRITE_ONCE/*,
								VertexBuffer::INDEX*/);

	vcoords = AttributeArray(coords, vdatabuf);
	vnormals = AttributeArray(normals, vdatabuf);
	vcolors = AttributeArray(colors, vdatabuf);
	vindices = IndexStream(indices, vindexbuf);
}

void PolylineRenderer::draw(RenderDevice *rd, shared_ptr<Shader> shader, Args &args) {
	rd->pushState();
	args.setAttributeArray("g3d_Vertex", vcoords);
	args.setAttributeArray("g3d_Normal", vnormals);
	args.setAttributeArray("color", vcolors);
	args.setPrimitiveType(PrimitiveType::TRIANGLES);
	args.setIndexStream(vindices);
	rd->apply(shader, args);
	rd->popState();
}

Vector2 PolylineRenderer::getNormal(int index, Array<Vector2> points) {
	Vector2 diff;
	if (index == 0) {
		diff = (points[1] - points[0]).direction();

	} else if (index == points.size() - 1) {
		diff = (points[index] - points[index - 1]).direction();
	} else {
		diff = (points[index+1] - points[index-1]).direction();
	}
	return Vector2(-diff.y, diff.x);
}