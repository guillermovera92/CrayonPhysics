#ifndef TurntableManipulator_h
#define TurntableManipulator_h

#include <G3D/G3DAll.h>

class TurntableManipulator: public Manipulator {
public:
	TurntableManipulator(double d=10.0, double a=1.57079632678, double u=0.0);
	virtual void setUserInput(UserInput *ui);
	virtual void onUserInput(UserInput *ui);
	virtual CoordinateFrame frame() const;
	virtual void getFrame(CoordinateFrame& c) const;
	virtual void onSimulation(RealTime rdt, SimTime sdt, SimTime idt);
	virtual Rect2D bounds() const;
	void setCenterPosition(Vector3 position);
	void bump(double ar, double u);

protected:
	double around;
	double up;
	double distance;
	Vector3 center;
	UserInput *uinput;
};

#endif