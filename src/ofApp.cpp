
//--------------------------------------------------------------
//
//  SJSU CS134 Spring 2020
//
//  Final Project
// 

//
//  Student Name:   Scott Nolan
//  Date: 05-15-2020


#include "ofApp.h"
#include "Util.h"
#include <glm/gtx/intersect.hpp>

const int TREE_DEPTH = 40;
const int INIT_FUEL = 30;	// seconds of fuel
const int INIT_HEALTH = 10;	// total velocity above 1 m/s that can be absorbed in crashes before the lander breaks

//--------------------------------------------------------------
// setup scene, lighting, state and load geometry
//
void ofApp::setup(){

	bWireframe = false;
	bDisplayPoints = false;
	bAltKeyDown = false;
	bCtrlKeyDown = false;
	bLanderLoaded = false;
	bTerrainSelected = true;
//	ofSetWindowShape(1024, 768);
	cam.setDistance(10);
	cam.setNearClip(.1);
	cam.setFov(65.5);   // approx equivalent to 28mm in 35mm format
	bTargetCam = true;
	camTarget = glm::vec3(0, 0, 0);
	ofSetVerticalSync(true);
	cam.disableMouseInput();
	ofEnableSmoothing();
	ofEnableDepthTest();

	tracking.setPosition(10, 10, 10);		// a camera that stays at a fixed location and points toward the lander
	tracking.lookAt(glm::vec3(0, 0, 0));
	tracking.setNearClip(.1);
	tracking.setFov(65.5);   

	mobile.setPosition(10, 10, 10);			// an 'over-the-shoulder' style of camera for the lander, except instead of shoulders the ship has antennas to look over
	mobile.lookAt(glm::vec3(0, 0, 0));
	mobile.setNearClip(.1);
	mobile.setFov(65.5);   

	onboard.setPosition(0, 20, 0);			// a camera pointed directly down from the bottom of the lander; rotates with the craft
	onboard.lookAt(glm::vec3(0, 0, 0));
	onboard.setNearClip(1.0);
	onboard.setFov(80);   

	top.setPosition(0, 100, 0);				// a top down camera on the whole scene (Michael Collins mode)
	top.lookAt(glm::vec3(0, 0, 0));
	top.setNearClip(.1);
	top.setFov(65.5);   

	theCam = &mobile;

	// setup rudimentary lighting 
	//
	initLightingAndMaterials();

	topLight.setup();
	topLight.enable();
	topLight.setDirectional();
	topLight.setAttenuation(.2, .001, .001);
	topLight.setAmbientColor(ofFloatColor(0.1, 0.1, 0.1));
	topLight.setDiffuseColor(ofFloatColor(0.9, 0.9, 0.8));		// a slightly yellow looking light
	topLight.setSpecularColor(ofFloatColor(1, 1, 1));
	topLight.rotate(10, ofVec3f(1, 0, 0));
	topLight.rotate(-70, ofVec3f(0, 1, 0));						// coming in from the west of the scene, at a slight downwards angle
	

	sideLight.setup();
	sideLight.enable();
	sideLight.setDirectional();
	sideLight.setAttenuation(.2, .001, .001);
	sideLight.setAmbientColor(ofFloatColor(0.1, 0.1, 0.1));		// a pure white light
	sideLight.setDiffuseColor(ofFloatColor(1, 1, 1));
	sideLight.setSpecularColor(ofFloatColor(1, 1, 1));
	sideLight.rotate(5, ofVec3f(1, 0, 0));						// coming in from the north of the scene, at an even more slight angle
	sideLight.rotate(180, ofVec3f(0, 1, 0));						

	thrusterLight.setup();
	thrusterLight.setPointLight();
	thrusterLight.setScale(0.02);
	thrusterLight.setAttenuation(.2, .02, .02);
	thrusterLight.setAmbientColor(ofFloatColor(0.1, 0.1, 0.1));
	thrusterLight.setDiffuseColor(ofFloatColor(0.2, 0.15, 0));	// a yellow-orange light
	thrusterLight.setSpecularColor(ofColor::orange);
																// will stay underneath the lander, and represent the glow from its thruster


	terrain.loadModel("geo/moon-houdini.obj");
	terrain.setScaleNormalization(false);
	mesh = terrain.getMesh(0);

	// load the sound effects
	if (!beep.load("sounds/quindar.mp3")) {
		ofLogFatalError("can't load sound: quindar.mp3");
		ofExit();
	}
	beep.setVolume(0.3);
	if (!exhaust.load("sounds/rocket.wav")) {
		ofLogFatalError("can't load sound: rocket.wav");
		ofExit();
	}
	exhaust.setVolume(0.3);
	exhaust.setSpeed(0.8);
	exhaust.setLoop(true);

	boundingBox = meshBounds(terrain.getMesh(0));

	ofDisableArbTex();     // disable rectangular textures

	// load textures
	//
	if (!ofLoadImage(particleTex, "images/nova_0.png")) {
		cout << "Particle Texture File: images/nova_0.png not found" << endl;
		ofExit();
	}		

	// load the shader (from example code)
	//
#ifdef TARGET_OPENGLES
	shader.load("shaders_gles/shader");
#else
	shader.load("shaders/shader");
#endif

	// load BG image
	//
	bBackgroundLoaded = backgroundImage.load("images/starfield-plain.jpg");

	// Add gui
	//
	gui.setup();
	gui.add(depth.setup("Octree Depth", 1, 0, TREE_DEPTH));
	gui.setPosition(glm::vec3(0, 35, 0));


	// Create terrain kdtree
	//
	octree.create(mesh, TREE_DEPTH);
	

	// Init booleans
	//
	bWireframe = false;
	bDisplayPoints = false;
	bAltKeyDown = false;
	bCtrlKeyDown = false;
	bLanderLoaded = false;


	// load lander model
	//
	if (lander.loadModel("geo/lander.obj")) {
		lander.setScaleNormalization(false);
		lander.setScale(.5, .5, .5);
		//	lander.setRotation(0, -180, 1, 0, 0);
		lander.setPosition(0, 0, 0);

		bLanderLoaded = true;

		ofVec3f min = lander.getSceneMin() + lander.getPosition();
		ofVec3f max = lander.getSceneMax() + lander.getPosition();

		landerBounds = Box(Vector3(min.x, min.y, min.z), Vector3(max.x, max.y, max.z));
	}
	else {
		cout << "Error: Can't load model" << "geo/lander.obj" << endl;
		ofExit(0);
	}

	// setup LEM
	//
	particles = new ParticleSystem();
	emitter = new ParticleEmitter(particles);
	emitter->init();
	emitter->type = DiscEmitter;
	emitter->setPosition(ofVec3f(0, 10, 0));
	emitter->setVelocity(ofVec3f(0, -10, 0));
	emitter->setRate(20);
	emitter->groupSize = 20;
	emitter->setParticleRadius(0.02);
	emitter->radius = 0.3;
	emitter->setLifespan(0.1);
	emitter->particleColor = ofColor::yellow;

	explosion = new ParticleEmitter(particles);
	explosion->init();
	explosion->type = RadialEmitter;
	explosion->setPosition(ofVec3f(0, 0, 0));
	explosion->setVelocity(ofVec3f(20, 0, 0));
	explosion->groupSize = 100;
	explosion->setParticleRadius(0.04);
	explosion->setLifespan(0.8);
	explosion->particleColor = ofColor::orange;

	landerParticle = new LanderParticle();
	landerParticle->damping = 0.999;
	landerParticle->mass = 40;
	landerParticle->position = ofVec3f(ofRandom(-35, 35), 25, ofRandom(-35, 35));
	lander.setPosition(landerParticle->position.x, landerParticle->position.y, landerParticle->position.z);

	fuel = INIT_FUEL;
	health = INIT_HEALTH;

	tracking.lookAt(landerParticle->position);
	mobile.setPosition(landerParticle->position + glm::rotate(glm::vec3(0, 5, 8), glm::radians(landerParticle->angle), glm::vec3(0, 1, 0)));
	mobile.lookAt(landerParticle->position);
	onboard.setPosition(landerParticle->position.x, landerParticle->position.y + 1.2, landerParticle->position.z);

	// create the randomly-placed landing zone (goal)
	//
	TreeNode temp;
	octree.intersect(Ray(Vector3(0, 50, 0), Vector3(ofRandom(-1, 1), -1, ofRandom(-1, 1))), octree.root, temp);	// shoot down a ray in a random-ish direction; wherever it hits is the landing zone
	goal = glm::vec3(mesh.getVertex(temp.points[0]));
	ofVec3f goalMin = lander.getSceneMin() + goal;
	ofVec3f goalMax = lander.getSceneMax() + goal;
	goalBox = new Box(Vector3(goalMin.x, goalMin.y, goalMin.z), Vector3(goalMax.x, goalMax.y, goalMax.z));
	
	// setup telemetry readout
	//
	aglRay = new Ray(Vector3(landerParticle->position.x, landerParticle->position.y, landerParticle->position.z), Vector3(0, -1, 0));

	// setup forces
	//
	gravity = new GravityForce(ofVec3f(0, -0.8, 0));		// acceleration due to gravity on the moon is 1.625 m/s^2, but there isn't really scale for a meter in this program, so I just used what felt about right
	turbulence = new TurbulenceForce(ofVec3f(-0.01, -0.01, -0.01), ofVec3f(0.01, 0.01, 0.01));
	thrust = new ThrusterForce();
	rotation = new RotationForce(landerParticle);
	impulse = new ImpulseForce();
}

//--------------------------------------------------------------
// incrementally update scene (animation)
//
void ofApp::update() {

	if (!bStarted) return;	// nothing to update if the game hasn't begun...

	float camRoll = landerParticle->angle;

	if (!debugMode) {	// if we're in debug mode (by pressing tab), put all the forces on pause and let the player control everything
		
		bool colliding = checkCollision(mesh, landerBounds, octree.root);
		
		if (colliding) { 
			turningLeft = turningRight = thrustingRight = thrustingLeft = thrustingBack = thrustingForward = false; 
			landerParticle->velocity.x *= 0.8;
			landerParticle->velocity.z *= 0.8;
			landerParticle->rVelocity *= 0.8;
			if (landerParticle->velocity.length() > 1) {
				health -= landerParticle->velocity.length() - 1;		// subtract health for a hard landing
				if (health <= 0) {
					explosion->setPosition(landerParticle->position);
					for(int i = 0; i < explosion->groupSize; i++)
					explosion->spawn(ofGetElapsedTimeMillis());									// explode some particles out when the ship crashes
				}
			}
		}

		else turbulence->updateForce(landerParticle);

		if (fuel <= 0 || health <= 0) turningRight = turningLeft = thrustingRight = thrustingLeft = thrustingUp = thrustingDown = thrustingBack = thrustingForward = false;		// no more flying if the ship is broken or out of fuel

		if (turningLeft ^ turningRight)	// cancel out if both are pressed at once
		{
			rotation->incrementRot(turningLeft);	// increments or decrements the rotational velocity float stored in the particle
		}

		thrust->incrementThrust(ofVec3f(thrustingRight * (-1) + thrustingLeft * (1),		// increment x axis thrust; cancel if both keys are pressed
			thrustingUp * (1.5) + thrustingDown * (-1.5),									// increment y axis thrust; slightly faster than the others, because why not
			thrustingBack * (1) + thrustingForward * (-1)),									// increment z axis thrust
			landerParticle->angle															// input angle of thrust
		);

		if (thrust->isThrusting()) fuel -= 1 / ofGetFrameRate();

		thrust->updateForce(landerParticle);
		gravity->updateForce(landerParticle);

		if (landerParticle->velocity.y < 0 && colliding) {
			ofVec3f norm = ofVec3f(0, 1, 0);
			impulse->force = (0.15 + 1.0)*((-landerParticle->velocity.dot(norm))*norm);	// deflect with a restitution of 0.15
			impulse->force *= ofGetFrameRate() * landerParticle->mass;
			impulse->updateForce(landerParticle);
			if (landerParticle->forces.y < 0) landerParticle->forces.y = 0.01f;			// for whatever reason, impulse->force.y would sometimes be slightly less than landerParticle->forces.y, causing the ship to slowly sink into the ground; I suspect it's a result of imprecision somehwere, but I'm just going to fix it manually here
		}
		landerParticle->integrate();
		camRoll = landerParticle->angle - camRoll;		// keep the onboard camera aligned with the lander's rotation
	}

	lander.setRotation(0, landerParticle->angle, 0, 1, 0);
	lander.setPosition(landerParticle->position.x, landerParticle->position.y, landerParticle->position.z);
	emitter->setPosition(landerParticle->position - ofVec3f(0, -0.5, 0));
	if (fuel <= 0 || health <= 0 || debugMode) { 
		emitter->stop();
		exhaust.stop();
	}
	emitter->update();
	explosion->update();

	thrusterLight.setPosition(landerParticle->position);

	// update any cameras that need it
	//
	if (!bLanderSelected) {
		tracking.lookAt(landerParticle->position);
		mobile.setPosition(landerParticle->position + glm::rotate(glm::vec3(0, 5, 8), glm::radians(landerParticle->angle), glm::vec3(0, 1, 0)));
		mobile.lookAt(landerParticle->position);
		if (!debugMode) onboard.rollDeg(camRoll);
		onboard.setPosition(landerParticle->position.x, landerParticle->position.y + 1.2, landerParticle->position.z);
	}
	
	// get altitude reading
	//
	aglRay->origin = Vector3(landerParticle->position.x, landerParticle->position.y, landerParticle->position.z);
	aglIntersect = &octree.root;
	if (!aglIntersect->box.intersect(*aglRay, 0, 500)) agl = -1;
	else {
		TreeNode below;
		octree.intersect(*aglRay, octree.root, below);
		agl = landerParticle->position.y - below.box.max().y();
	}

	// check for win
	//
	glm::vec3 distance = landerParticle->position - goal;
	if (landerParticle->velocity.length() < 1.5 && distance.y <= 0.8 && distance.x <= 1.5 && distance.z <= 2 && !bWin && health > 0) {	// a little more forgiving with lateral accuracy than vertical
		bWin = true; 
		fuelFinal = fuel;
		beep.play();
	}
}


//--------------------------------------------------------------
void ofApp::draw() {

	loadVbo();

	// draw background image
	//
	if (bBackgroundLoaded) {
		ofPushMatrix();
		ofSetColor(50, 50, 50);
		ofScale(2, 2);
		backgroundImage.draw(-200, -100);
		ofPopMatrix();
	}

	ofEnableDepthTest();

	theCam->begin();

	ofPushMatrix();

	if (bWireframe) {                    // wireframe mode  (include axis)
		ofDisableLighting();
		ofSetColor(ofColor::slateGray);
		terrain.drawWireframe();
		if (bLanderLoaded) {
			lander.drawWireframe();
			if (!bTerrainSelected) drawAxis(lander.getPosition());
		}
		if (bTerrainSelected) drawAxis(ofVec3f(0, 0, 0));
	}
	else {
		ofEnableLighting();              // shaded mode
		terrain.drawFaces();

		// shader section
		//	
		glDepthMask(GL_FALSE);
		ofEnableBlendMode(OF_BLENDMODE_ADD);
		ofEnablePointSprites();

		shader.begin();
		//emitter->draw();
		ofSetColor(ofColor::darkorange);
		particleTex.bind();
		vbo.draw(GL_POINTS, 0, (int)emitter->sys->particles.size());		// draw particles with shader
		particleTex.unbind();
		shader.end();

		ofSetColor(255);
		ofDisablePointSprites();
		ofDisableBlendMode();
		ofEnableAlphaBlending();
		glDepthMask(GL_TRUE);

		// draw the lander, and its bounding box if it's in debug mode
		if (bLanderLoaded) {
			lander.drawFaces();

			ofVec3f min = lander.getSceneMin() + lander.getPosition();
			ofVec3f max = lander.getSceneMax() + lander.getPosition();

			Box bounds = Box(Vector3(min.x, min.y, min.z), Vector3(max.x, max.y, max.z));

			landerBounds = bounds;

			if (bLanderSelected) ofSetColor(ofColor::red);
			if (debugMode) drawBox(bounds);
			ofSetColor(255);
		}
		if (bTerrainSelected && debugMode) drawAxis(ofVec3f(0, 0, 0));
	}


	if (bDisplayPoints) {                // display points as an option    
		glPointSize(3);
		ofSetColor(ofColor::green);
		terrain.drawVertices();
	}


	ofNoFill();
	ofSetColor(ofColor::white);
	if (debugMode) octree.draw(octree.root, depth, 0);		// draw the terrain-splitting octree to a depth given by the gui slider
	//octree.drawLeafNodes(octree.root);

	ofSetColor(ofColor::green);
	drawBox(*goalBox);			// the spot for the player to land on

	if (debugMode && isHighlighted) {
		ofSetColor(ofColor::blue);
		drawBox(highlighted.box);
	}

	ofPopMatrix();

	theCam->end();

	ofDisableDepthTest();	
	
	if (debugMode) gui.draw();


	// draw screen data
	//

	// framerate in top left
	string str;
	str += "Frame Rate: " + std::to_string(ofGetFrameRate());
	ofSetColor(ofColor::white);
	if (debugMode) ofDrawBitmapString(str, ofGetWindowWidth() - 170, 15);		// player probably doesnt care about the frame rate unless they're debugging

	// live altitude in top right
	string str2;
	if (agl < 0 || agl > 500)	str2 += "Altitide (AGL): >500";
	else						str2 += "Altitide (AGL): " + std::to_string(agl);
	ofSetColor(ofColor::white);
	ofDrawBitmapString(str2, 5, 15);

	// current fuel shown below altitude readout
	string str3;
	if (fuel <= 0) str3 += "FUEL: 0.0s";
	else { 
		char fuelSeconds[16];
		sprintf(fuelSeconds, "FUEL: %.1fs", fuel);				// only show one digit after the decimal
		str3 += fuelSeconds;
	}
	ofSetColor(ofColor::white);
	ofDrawBitmapString(str3, 5, 30);

	// message at start of game
	if (!bStarted) {
		string str4 = "click anywhere to begin";
		ofDrawBitmapString(str4, (ofGetWindowWidth() / 2) - 100, 15);
	}

	// message after win
	else if (bWin) {
		string str4 = "Mission Accomplished";
		ofSetColor(ofColor::green);
		ofDrawBitmapString(str4, (ofGetWindowWidth()/2) - 100, 15);
		string str5;
		char finalFuelSeconds[24];
		sprintf(finalFuelSeconds, "fuel at landing: %.1fs", fuelFinal);
		str5 += finalFuelSeconds;
		ofDrawBitmapString(str5, (ofGetWindowWidth() / 2) - 105, 30);
	}

	// let the player know they've damaged the lander
	if (health < INIT_HEALTH) {
		string str6;
		if (health > 0) {
			str6 = "damage sustained";
			int col = ofMap(health, INIT_HEALTH, 0, 255, 0);	// the message will be more red as the lander suffers more damage
			ofSetColor(ofColor(255, col, col));
		}
		else {
			str6 = "damage sustained - LEM inoperable";
			ofSetColor(ofColor(255, 0, 0));
		}
		ofDrawBitmapString(str6, 5, ofGetWindowHeight() - 15);
	}

	ofSetColor(255);
}

// 

// Draw an XYZ axis in RGB at world (0,0,0) for reference.
//
void ofApp::drawAxis(ofVec3f location) {

	ofPushMatrix();
	ofTranslate(location);

	ofSetLineWidth(1.0);

	// X Axis
	ofSetColor(ofColor(255, 0, 0));
	ofDrawLine(ofPoint(0, 0, 0), ofPoint(1, 0, 0));
	

	// Y Axis
	ofSetColor(ofColor(0, 255, 0));
	ofDrawLine(ofPoint(0, 0, 0), ofPoint(0, 1, 0));

	// Z Axis
	ofSetColor(ofColor(0, 0, 255));
	ofDrawLine(ofPoint(0, 0, 0), ofPoint(0, 0, 1));

	ofPopMatrix();
}


void ofApp::keyPressed(int key) {

	switch (key) {
	case 'C':
	case 'c':
		if (cam.getMouseInputEnabled()) cam.disableMouseInput();
		else cam.enableMouseInput();
		break;
	case 'F':
	case 'f':
		ofToggleFullscreen();
		break;
	case 'H':
	case 'h':
		break;
	case 'r':
		cam.reset();
		break;
	case 't':
		setCameraTarget();
		break;
	case 'u':
		break;
	case 'v':
		if (debugMode) togglePointsDisplay();
		break;
	case 'V':
		break;
	case 'w':     // spacecraft thrust UP
		thrustingUp = true;
		emitter->start();
		thrusterLight.enable();
		if (!exhaust.isPlaying()) exhaust.play();
		break;
	case 's':     // spacecraft thrust DOWN
		thrustingDown = true;
		emitter->start();
		thrusterLight.enable();
		if (!exhaust.isPlaying()) exhaust.play();
		break;
	case 'a':     // rotate spacecraft counter-clockwise (about Y (UP) axis)
		turningLeft = true;
		break;
	case 'd':     // rotate spacecraft clockwise (about Y (UP) axis)
		turningRight = true;
		break;
	case OF_KEY_ALT:
		cam.enableMouseInput();
		bAltKeyDown = true;
		break;
	case OF_KEY_CONTROL:
		bCtrlKeyDown = true;
		break;
	case OF_KEY_SHIFT:
		if (debugMode) toggleWireframeMode();
		break;
	case OF_KEY_DEL:
		break;
	case OF_KEY_F1:		// freecam
		theCam = &cam;
		bTargetCam = true;
		cam.setTarget(camTarget);
		break;
	case OF_KEY_F2:		// tracking cam
		theCam = &tracking;
		bTargetCam = false;
		break;
	case OF_KEY_F3:		// over the shoulder cam
		theCam = &mobile;
		bTargetCam = false;
		break;
	case OF_KEY_F4:		// onboard cam
		theCam = &onboard;
		bTargetCam = false;
		break;
	case OF_KEY_F5:		// top down cam
		theCam = &top;
		bTargetCam = false;
		break;
	case OF_KEY_UP:    // move forward
		thrustingForward = true;
		break;
	case OF_KEY_DOWN:   // move backward
		thrustingBack = true;
		break;
	case OF_KEY_LEFT:   // move left
		thrustingLeft = true;
		break;
	case OF_KEY_RIGHT:   // move right
		thrustingRight = true;
		break;
	case OF_KEY_TAB:	// toggle debug mode
		debugMode = !debugMode;
		break;
	default:
		break;
	}
}

void ofApp::toggleWireframeMode() {
	bWireframe = !bWireframe;
}

void ofApp::toggleSelectTerrain() {
	bTerrainSelected = !bTerrainSelected;
}

void ofApp::togglePointsDisplay() {
	bDisplayPoints = !bDisplayPoints;
}

void ofApp::keyReleased(int key) {

	switch (key) {
	case 'w':
		thrustingUp = false;
		emitter->stop();
		thrusterLight.disable();
		if (exhaust.isPlaying()) exhaust.stop();
		break;
	case 's':
		thrustingDown = false;
		emitter->stop();
		thrusterLight.disable();
		if (exhaust.isPlaying()) exhaust.stop();
		break;
	case 'd':
		turningRight = false;
		break;
	case 'a':
		turningLeft = false;
		break;
	case OF_KEY_ALT:
		cam.disableMouseInput();
		bAltKeyDown = false;
		break;
	case OF_KEY_CONTROL:
		bCtrlKeyDown = false;
		break;
	case OF_KEY_SHIFT:
		break;
	case OF_KEY_UP:
		thrustingForward = false;
		break;
	case OF_KEY_DOWN:
		thrustingBack = false;
		break;
	case OF_KEY_LEFT:
		thrustingLeft = false;
		break;
	case OF_KEY_RIGHT:
		thrustingRight = false;
		break;
	default:
		break;
	}
}



//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

	

}


//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button) {

	if (!bStarted) bStarted = true;

	// if moving camera, don't allow mouse interaction
	//
	if (cam.getMouseInputEnabled()) return;

	// check if the player has clicked on the lander
	glm::vec3 p = theCam->screenToWorld(glm::vec3(mouseX, mouseY, 0));
	glm::vec3 rayDir = glm::normalize(p - theCam->getPosition());

	glm::vec3 min = lander.getSceneMin() + lander.getPosition();
	glm::vec3 max = lander.getSceneMax() + lander.getPosition();
	Box bounds = Box(Vector3(min.x, min.y, min.z), Vector3(max.x, max.y, max.z));
	Ray mouseClickRay = Ray(Vector3(p.x, p.y, p.z), Vector3(rayDir.x, rayDir.y, rayDir.z));

	if (debugMode && bounds.intersect(mouseClickRay, 0, 1000)) {
		bLanderSelected = true;
		orthVector = rayDir;
	}
	else {
		bLanderSelected = false;

		if (octree.intersect(mouseClickRay, octree.root, highlighted)) { 
			isHighlighted = true;
			if (bTargetCam) { 
				glm::vec3 diff = cam.getPosition() - camTarget;
				camTarget = glm::vec3(highlighted.box.max().x(), highlighted.box.max().y(), highlighted.box.max().z()); 
				cam.setPosition(camTarget + diff);
				cam.setTarget(camTarget);
			}
		}
		else isHighlighted = false;	
	}

	mouseDownPos = p;
}


//draw a box from a "Box" class  
//
void ofApp::drawBox(const Box &box) {
	Vector3 min = box.parameters[0];
	Vector3 max = box.parameters[1];
	Vector3 size = max - min;
	Vector3 center = size / 2 + min;
	ofVec3f p = ofVec3f(center.x(), center.y(), center.z());
	float w = size.x();
	float h = size.y();
	float d = size.z();
	ofDrawBox(p, w, h, d);
}

// return a Mesh Bounding Box for the entire Mesh
//
Box ofApp::meshBounds(const ofMesh & mesh) {
	int n = mesh.getNumVertices();
	ofVec3f v = mesh.getVertex(0);
	ofVec3f max = v;
	ofVec3f min = v;
	for (int i = 1; i < n; i++) {
		ofVec3f v = mesh.getVertex(i);

		if (v.x > max.x) max.x = v.x;
		else if (v.x < min.x) min.x = v.x;

		if (v.y > max.y) max.y = v.y;
		else if (v.y < min.y) min.y = v.y;

		if (v.z > max.z) max.z = v.z;
		else if (v.z < min.z) min.z = v.z;
	}
	return Box(Vector3(min.x, min.y, min.z), Vector3(max.x, max.y, max.z));
}



//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button) {

	if (cam.getMouseInputEnabled()) return;

	// move the lander under the mouse
	glm::vec3 p = theCam->screenToWorld(glm::vec3(mouseX, mouseY, 0));
	glm::vec3 rayDir = glm::normalize(p - theCam->getPosition());

	if (debugMode && bLanderSelected) {
		if (glm::dot(rayDir, orthVector) == 0) return;	// check that the ray cast from the mouse drag isnt perpendicular to the plane of the lander
		// test for ray-plane intersection:

		// orthVector = the orthogonal vector to the plane
		// lander.getPosition() = the point on the plane

		// rayDir = direction of the ray cast from the mouse
		// theCam->getPosition() = the origin of the ray
		// t = the length of the ray when it intersects the plane

		float t = (glm::dot(glm::vec3(lander.getPosition() - theCam->getPosition()), glm::vec3(orthVector))) / (glm::dot(rayDir, orthVector));
		glm::vec3 newPos = theCam->getPosition() + (rayDir * t);

		landerParticle->position = newPos;
	}

	else if (octree.intersect(Ray(Vector3(p.x, p.y, p.z), Vector3(rayDir.x, rayDir.y, rayDir.z)), octree.root, highlighted)) isHighlighted = true;
}


//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button) {

	bLanderSelected = false;
	isHighlighted = false;
}



// Set the camera to use the selected point as it's new target
//  
void ofApp::setCameraTarget() {

}


//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}



//--------------------------------------------------------------
// setup basic ambient lighting in GL  (for now, enable just 1 light)
//
void ofApp::initLightingAndMaterials() {

	static float ambient[] =
	{ .5f, .5f, .5, 1.0f };
	static float diffuse[] =
	{ 1.0f, 1.0f, 1.0f, 1.0f };

	static float position[] =
	{5.0, 5.0, 5.0, 0.0 };

	static float lmodel_ambient[] =
	{ 1.0f, 1.0f, 1.0f, 1.0f };

	static float lmodel_twoside[] =
	{ GL_TRUE };


	glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
	glLightfv(GL_LIGHT0, GL_POSITION, position);

	glLightfv(GL_LIGHT1, GL_AMBIENT, ambient);
	glLightfv(GL_LIGHT1, GL_DIFFUSE, diffuse);
	glLightfv(GL_LIGHT1, GL_POSITION, position);


	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, lmodel_ambient);
	glLightModelfv(GL_LIGHT_MODEL_TWO_SIDE, lmodel_twoside);

	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
//	glEnable(GL_LIGHT1);
	glShadeModel(GL_SMOOTH);
} 

void ofApp::savePicture() {
	ofImage picture;
	picture.grabScreen(0, 0, ofGetWidth(), ofGetHeight());
	picture.save("screenshot.png");
	cout << "picture saved" << endl;
}

//--------------------------------------------------------------
//
// support drag-and-drop of model (.obj) file loading.  when
// model is dropped in viewport, place origin under cursor
//
void ofApp::dragEvent(ofDragInfo dragInfo) {
	if (lander.loadModel(dragInfo.files[0])) {
		bLanderLoaded = true;
		lander.setScaleNormalization(false);
	//	lander.setScale(.5, .5, .5);
		lander.setPosition(0, 0, 0);
//		lander.setRotation(1, 180, 1, 0, 0);

		// We want to drag and drop a 3D object in space so that the model appears 
		// under the mouse pointer where you drop it !
		//
		// Our strategy: intersect a plane parallel to the camera plane where the mouse drops the model
		// once we find the point of intersection, we can position the lander/lander
		// at that location.
		//

		// Setup our rays
		//
		glm::vec3 origin = theCam->getPosition();
		glm::vec3 camAxis = theCam->getZAxis();
		glm::vec3 mouseWorld = theCam->screenToWorld(glm::vec3(mouseX, mouseY, 0));
		glm::vec3 mouseDir = glm::normalize(mouseWorld - origin);
		float distance;

		bool hit = glm::intersectRayPlane(origin, mouseDir, glm::vec3(0, 0, 0), camAxis, distance);
		if (hit) {
			// find the point of intersection on the plane using the distance 
			// We use the parameteric line or vector representation of a line to compute
			//
			// p' = p + s * dir;
			//
			glm::vec3 intersectPoint = origin + distance * mouseDir;

			// Now position the lander's origin at that intersection point
			//
		    glm::vec3 min = lander.getSceneMin();
			glm::vec3 max = lander.getSceneMax();
			float offset = (max.y - min.y) / 2.0;
			lander.setPosition(intersectPoint.x, intersectPoint.y - offset, intersectPoint.z);

			// set up bounding box for lander while we are at it
			//
			landerBounds = Box(Vector3(min.x, min.y, min.z), Vector3(max.x, max.y, max.z));
		}
	}
	

}

// load vertex buffer in preparation for rendering (from example code)
//
void ofApp::loadVbo() {
	if (emitter->sys->particles.size() < 1) return;

	vector<ofVec3f> sizes;
	vector<ofVec3f> points;
	for (int i = 0; i < emitter->sys->particles.size(); i++) {
		points.push_back(emitter->sys->particles[i].position);
		sizes.push_back(ofVec3f(5));
	}
	// upload the data to the vbo
	//
	int total = (int)points.size();
	vbo.clear();
	vbo.setVertexData(&points[0], total, GL_STATIC_DRAW);
	vbo.setNormalData(&sizes[0], total, GL_STATIC_DRAW);
}


//  intersect the mouse ray with the plane normal to the camera 
//  return intersection point.   (package code above into function)
//
glm::vec3 ofApp::getMousePointOnPlane() {
	// Setup our rays
	//
	glm::vec3 origin = theCam->getPosition();
	glm::vec3 camAxis = theCam->getZAxis();
	glm::vec3 mouseWorld = theCam->screenToWorld(glm::vec3(mouseX, mouseY, 0));
	glm::vec3 mouseDir = glm::normalize(mouseWorld - origin);
	float distance;
	
	bool hit = glm::intersectRayPlane(origin, mouseDir, glm::vec3(0, 0, 0), camAxis, distance);

	if (hit) {
		// find the point of intersection on the plane using the distance 
		// We use the parameteric line or vector representation of a line to compute
		//
		// p' = p + s * dir;
		//
		glm::vec3 intersectPoint = origin + distance * mouseDir;
		
		return intersectPoint;
	}
	else return glm::vec3(0, 0, 0);
}


// check for collision between the lander and the terrain using the kd tree
//
bool ofApp::checkCollision(ofMesh &mesh, Box &landerBounds, TreeNode &node){
	if (!(node.box.collide(landerBounds))) return false;

	if (node.children.size() == 0) {
		 ofVec3f vertex;
		for (int i = 0; i < node.points.size(); i++) {
			vertex = mesh.getVertex(node.points[i]);
			if (landerBounds.contains(Vector3(vertex.x, vertex.y, vertex.z)))  return true; 
		}
		return false; 
	}

	for (int i = 0; i < node.children.size(); i++) {
		if (checkCollision(mesh, landerBounds, node.children[i])) return true;
	}
	return false;
}


