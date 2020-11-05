#pragma once
//  Kevin M. Smith - CS 134 SJSU

#include "ofMain.h"
#include "Particle.h"


//  Pure Virtual Function Class - must be subclassed to create new forces.
//
class ParticleForce {
protected:
public:
	bool applyOnce = false;
	bool applied = false;
	virtual void updateForce(Particle *) = 0;
};

class ParticleSystem {
public:
	void add(const Particle &);
	void addForce(ParticleForce *);
	void remove(int);
	void update();
	void setLifespan(float);
	void reset();
	int removeNear(const ofVec3f & point, float dist);
	void draw();
	vector<Particle> particles;
	vector<ParticleForce *> forces;
};



// Some convenient built-in forces
//
class GravityForce: public ParticleForce {
	ofVec3f gravity;
public:
	void set(const ofVec3f &g) { gravity = g; }
	GravityForce(const ofVec3f & gravity);
	void updateForce(Particle *);
};

class TurbulenceForce : public ParticleForce {
	ofVec3f tmin, tmax;
public:
	void set(const ofVec3f &min, const ofVec3f &max) { tmin = min; tmax = max; }
	TurbulenceForce(const ofVec3f & min, const ofVec3f &max);
	TurbulenceForce() { tmin.set(0, 0, 0); tmax.set(0, 0, 0); }
	void updateForce(Particle *);
};

class ImpulseRadialForce : public ParticleForce {
	float magnitude = 1.0;
	float height = .2;
public:
	void set(float mag) { magnitude = mag; }
	void setHeight(float h) { height = h; }
	ImpulseRadialForce(float magnitude);
	ImpulseRadialForce() {}
	void updateForce(Particle *);
};

//  define your ThrusterForce  ...here...
//
class ThrusterForce : public ParticleForce {
	ofVec3f thrust;
public: 
	void set(ofVec3f newThrust) { thrust = newThrust;  }
	ThrusterForce(ofVec3f initThrust) { thrust = initThrust; }
	ThrusterForce() { thrust = ofVec3f(0, 0, 0);  }
	void incrementThrust(ofVec3f, float);				// change thrust when pressing w or s or the arrow keys
	void updateForce(Particle*);
	bool isThrusting() { return thrust == ofVec3f(0) ? false : true; }
};

// an angular force to implement physics-based rotation:
//	functions like a normal force, but in only one dimension - the angle
class RotationForce : public ParticleForce {
public:	
	LanderParticle *lParticle;
	RotationForce(LanderParticle *lander) { lParticle = lander; }
	RotationForce() { lParticle = new LanderParticle(); }
	void incrementRot(bool positive) { lParticle->rVelocity += positive ? 0.02 : -0.02; }	// if the input boolean is true, you want to increment the rotation in a positive direction, and vice versa for negative
	void updateForce(Particle *) { ; }	// don't do anything; need a special method to update the rotation acceleration value, since only LanderParticles have this
};

// Duplicated from the 3D collision example code
//
class ImpulseForce : public ParticleForce {
public:
	ImpulseForce() {
		force = ofVec3f(0, 0, 0);
	}
	void updateForce(Particle *particle) {
		particle->forces += force;
	}

	ofVec3f force;
};

