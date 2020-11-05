#pragma once

#include "ofMain.h"
#include  "ofxAssimpModelLoader.h"
#include "ofxGui.h"
#include "box.h"
#include "ray.h"
#include "KdTree.h"
#include "ParticleSystem.h"
#include "ParticleEmitter.h"

class ofApp : public ofBaseApp{

	public:
		void setup();
		void update();
		void draw();

		void keyPressed(int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void mouseEntered(int x, int y);
		void mouseExited(int x, int y);
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);
		void drawAxis(ofVec3f);
		void initLightingAndMaterials();
		void savePicture();
		void toggleWireframeMode();
		void togglePointsDisplay();
		void toggleSelectTerrain();
		void setCameraTarget();
		void loadVbo();
		glm::vec3 getMousePointOnPlane();
		bool checkCollision(ofMesh &, Box &, TreeNode &);
		void drawBox(const Box &box);
		Box meshBounds(const ofMesh &);
		
		ParticleSystem *particles;
		ParticleEmitter *emitter;		// exhaust particles
		ParticleEmitter *explosion;		// crash particles
		LanderParticle *landerParticle;	// the particle that controls the lander
		GravityForce *gravity;
		ThrusterForce *thrust;
		TurbulenceForce *turbulence;
		RotationForce *rotation;
		ImpulseForce *impulse;

		ofEasyCam cam;					// default freecam
		ofCamera tracking;				// tracking camera
		ofCamera mobile;				// over the shoulder camera
		ofCamera onboard;				// perspective camera
		ofCamera top;					// top down camera
		ofCamera *theCam;
		ofxAssimpModelLoader terrain, lander;
		ofMesh mesh;
		ofLight topLight;
		ofLight sideLight;
		ofLight thrusterLight;
		Box boundingBox;
		Box landerBounds;
		ofImage backgroundImage;
		ofSoundPlayer exhaust;			// when thrusting
		ofSoundPlayer beep;				// after successful landing

		glm::vec3 goal;					// target landing zone
		Box *goalBox;

		ofxFloatSlider depth;			// depth of the octree to draw
		ofxPanel gui;

		bool bAltKeyDown;
		bool bCtrlKeyDown;
		bool bWireframe;
		bool bDisplayPoints;
		bool bPointSelected;
		bool bTargetCam;
		
		bool bLanderLoaded = false;
		bool bTerrainSelected;
		bool bLanderSelected = false;
		bool bBackgroundLoaded = false;
		bool bStarted = false;
		bool bWin = false;
		bool debugMode = false;
	
		ofVec3f selectedPoint;
		ofVec3f intersectPoint;
		
		glm::vec3 mouseDownPos;
		glm::vec3 orthVector;
		glm::vec3 camTarget;

		bool thrustingUp = false;			// thrust controls
		bool thrustingDown = false;			//
		bool thrustingForward = false;		//
		bool thrustingBack = false;			//
		bool thrustingLeft = false;			//
		bool thrustingRight = false;		//

		bool turningLeft = false;			//
		bool turningRight = false;			//

		float agl;							// current altitude
		Ray *aglRay;						// ray to get agl
		TreeNode *aglIntersect;				// the node that the ray intersects with when testing agl

		float fuel;							// current fuel
		float fuelFinal;					// fuel at time of landing

		float health;						// lander integrity

		const float selectionRange = 4.0;

		KdTree octree;						// the kdtree is now an octree, even though the class is still called kdtree. Why? ==> I don't know how to rename classes on visual studio
		TreeNode highlighted;
		bool isHighlighted = false;

		ofTexture  particleTex;				// shader components
		ofVbo vbo;							//
		ofShader shader;					//
};
