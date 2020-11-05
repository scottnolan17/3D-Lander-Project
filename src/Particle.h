#pragma once

#include "ofMain.h"

class ParticleForceField;

class Particle {
public:
	Particle();

	ofVec3f position;
	ofVec3f velocity;
	ofVec3f acceleration;
	ofVec3f forces;
	float	damping;
	float   mass;
	float   lifespan;
	float   radius;
	float   birthtime;
	void    integrate();
	void    draw();
	float   age();        // sec
	ofColor color;
};

// A special particle to represent the lander, since it needs to remeber its rotation
//
class LanderParticle : public Particle {
public:
	float angle = 0;
	float rVelocity = 0; // rotational velocity
	void integrate();
};