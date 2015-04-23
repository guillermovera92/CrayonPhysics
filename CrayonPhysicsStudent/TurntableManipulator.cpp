#include "TurntableManipulator.h"

const double X_SCALE = 0.01;
const double Y_SCALE = 0.01;

TurntableManipulator::TurntableManipulator(double d, double a, double u) {
	distance = d;
	around = a;
	up = u;
	m_depth = -200;
	uinput = NULL;
}

Rect2D TurntableManipulator::bounds() const {
	return Rect2D::xywh(0,0,0,0);
}

void TurntableManipulator::setUserInput(UserInput *ui) {
	uinput = ui;
}

void TurntableManipulator::bump(double ar, double u) {
	around += ar;
	up += u;
}

void TurntableManipulator::onSimulation(RealTime rdt, SimTime sdt, SimTime idt) {
	if (uinput) {
		if (uinput->keyDown(GKey(GKey::RIGHT_MOUSE))) {
			around = around + uinput->mouseDX() * X_SCALE;
			up = clamp(up + uinput->mouseDY() * Y_SCALE, -halfPi() + 0.2, halfPi() - 0.2);
		}
		if (uinput->keyDown(GKey('a'))) {
			distance /= pow(1.5,rdt);
		}
		if (uinput->keyDown(GKey('z'))) {
			distance *= pow(1.5,rdt);
		}
	}
}

void TurntableManipulator::onUserInput(UserInput *ui) {
	uinput = ui;
}

CoordinateFrame TurntableManipulator::frame() const {
	Vector3 pos(cos(around)*cos(up)*distance,
		sin(up)*distance,
		sin(around)*cos(up)*distance);
	CoordinateFrame cf(pos + center);
	cf.lookAt(center, Vector3::unitY());
	return cf;
}

void TurntableManipulator::getFrame(CoordinateFrame& c) const {
	c = frame();
}

void TurntableManipulator::setCenterPosition(Vector3 position) {
	center = position;
}
