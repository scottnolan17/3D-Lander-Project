//  KdTree Template - Simple KdTree class
//
//  SJSU  - CS134 Game Development
//
//  Kevin M. Smith   04/19/20

//  **Important:  Vertices (x, y, z) in the mesh are stored in the Tree node as an integer index.
//  to read the 3D vertex point from the mesh given index i,  use the function ofMesh::getVertex(i);  See
//  KdTree::meshBounds() for an example of usage;
//
//

#include "KdTree.h"
 
// draw KdTree (recursively)
//
void KdTree::draw(TreeNode & node, int numLevels, int level) {
	if (level >= numLevels) return;
	drawBox(node.box);
	level++;
	for (int i = 0; i < node.children.size(); i++) {
		draw(node.children[i], numLevels, level);
	}
}

// draw only leaf Nodes
//
void KdTree::drawLeafNodes(TreeNode & node) {
	if (node.children.size() == 0) drawBox(node.box);
	else {
		for (int i = 0; i < node.children.size(); i++)
			drawLeafNodes(node.children[i]);
	}
}


//draw a box from a "Box" class  
//
void KdTree::drawBox(const Box &box) {
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
Box KdTree::meshBounds(const ofMesh & mesh) {
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
	cout << "vertices: " << n << endl;
//	cout << "min: " << min << "max: " << max << endl;
	return Box(Vector3(min.x, min.y, min.z), Vector3(max.x, max.y, max.z));
}

// getMeshPointsInBox:  return an array of indices to points in mesh that are contained 
//                      inside the Box.  Return count of points found;
//
int KdTree::getMeshPointsInBox(const ofMesh & mesh, const vector<int>& points,
	Box & box, vector<int> & pointsRtn)
{
	int result = 0;
	ofVec3f point;
	for (int i = 0; i < points.size(); i++) {
		point = mesh.getVertex(points[i]);
		if (box.inside(Vector3(point.x, point.y, point.z))) {
			pointsRtn.push_back(points[i]);
			result++;
		}
	}
	return result;
}



//  Subdivide a Box; return children in  boxList
//
void KdTree::subDivideBox(const Box &box, vector<Box> & boxList) {
	/*
	if (axis == "x") {
		boxList.push_back(Box(box.parameters[0], Vector3(halfPoint, box.parameters[1].y(), box.parameters[1].z())));
		boxList.push_back(Box(Vector3(halfPoint, box.parameters[0].y(), box.parameters[0].z()), box.parameters[1]));
	}
	else if (axis == "y") {
		boxList.push_back(Box(box.parameters[0], Vector3(box.parameters[1].x(), halfPoint, box.parameters[1].z())));
		boxList.push_back(Box(Vector3(box.parameters[0].x(), halfPoint, box.parameters[0].z()), box.parameters[1]));
	}
	else if (axis == "z") {
		boxList.push_back(Box(box.parameters[0], Vector3(box.parameters[1].x(), box.parameters[1].y(), halfPoint)));
		boxList.push_back(Box(Vector3(box.parameters[0].x(), box.parameters[0].y(), halfPoint), box.parameters[1]));
	}
	*/
	float width = box.parameters[1].x() - box.parameters[0].x();
	float height = box.parameters[1].y() - box.parameters[0].y();
	float depth = box.parameters[1].z() - box.parameters[0].z();
	Vector3 min = box.parameters[0];
	Vector3 max = box.parameters[1];
	boxList.push_back(Box(box.parameters[0], Vector3(min.x() + width / 2, min.y() + height / 2, min.z() + depth / 2)));
	boxList.push_back(Box(Vector3(min.x() + width / 2, min.y(), min.z()), Vector3(max.x(), min.y() + height / 2, min.z() + depth / 2)));
	boxList.push_back(Box(Vector3(min.x(), min.y(), min.z() + depth / 2), Vector3(min.x() + width / 2, min.y() + height / 2, max.z())));
	boxList.push_back(Box(Vector3(min.x() + width / 2, min.y(), min.z() + depth / 2), Vector3(max.x(), min.y() + height / 2, max.z())));
	boxList.push_back(Box(Vector3(min.x(), min.y() + height / 2, min.z()), Vector3(min.x() + width / 2, max.y(), min.z() + depth / 2)));
	boxList.push_back(Box(Vector3(min.x() + width / 2, min.y() + height / 2, min.z()), Vector3(max.x(), max.y(), min.z() + depth / 2)));
	boxList.push_back(Box(Vector3(min.x(), min.y() + height / 2, min.z() + depth / 2), Vector3(min.x() + width / 2, max.y(), max.z())));
	boxList.push_back(Box(Vector3(min.x() + width / 2, min.y() + height / 2, min.z() + depth / 2), box.parameters[1]));
}

void KdTree::create(const ofMesh & geo, int numLevels) {
	mesh = geo;
	int level = 0;

	root.box = meshBounds(mesh);
	for (int i = 0; i < mesh.getNumVertices(); i++) root.points.push_back(i);
	level++;

	subdivide(mesh, root, numLevels, level);
}

void KdTree::subdivide(const ofMesh & mesh, TreeNode & node, int numLevels, int level) {
	if (level > numLevels || node.points.size() <= 1) return;

	// subdivide this node
	vector<Box> childBoxes;
	vector<int> childPoints1;
	vector<int> childPoints2;
	vector<int> childPoints3;
	vector<int> childPoints4;
	vector<int> childPoints5;
	vector<int> childPoints6;
	vector<int> childPoints7;
	vector<int> childPoints8;

	float halfPoint;
	/*
	if (node.box.parameters[1].x() - node.box.parameters[0].x() >= node.box.parameters[1].z() - node.box.parameters[0].z()) {		// if x dimension is bigger than z dimension...
		if (node.box.parameters[1].x() - node.box.parameters[0].x() >= node.box.parameters[1].y() - node.box.parameters[0].y()) {	// and if the x dimension is also bigger than the y dimension...

			// divide along x axis into 2 equal sized boxes
//			halfPoint = node.box.parameters[0].x() + (node.box.parameters[1].x() - node.box.parameters[0].x()) / 2;

			// divide along mean of data points on the x axis
			halfPoint = 0;
			for (int i = 0; i < node.points.size(); i++) halfPoint += mesh.getVertex(node.points[i]).x;
			halfPoint = halfPoint / node.points.size();

			subDivideBox(node.box, childBoxes, halfPoint, "x");
		}
		else {																														// or if the y dimension is bigger than the x dimension...

			// divide along y axis into 2 equal sized boxes
			halfPoint = node.box.parameters[0].y() + (node.box.parameters[1].y() - node.box.parameters[0].y()) / 2;

			// divide along mean of data points on the y axis
//			halfPoint = 0;
//			for (int i = 0; i < node.points.size(); i++) halfPoint += mesh.getVertex(node.points[i]).y;
//			halfPoint = halfPoint / node.points.size();

			subDivideBox(node.box, childBoxes, halfPoint, "y");
		}
	}
	else {																															// or if the z dimension is bigger than the x dimension...
		if (node.box.parameters[1].z() - node.box.parameters[0].z() >= node.box.parameters[1].y() - node.box.parameters[0].y()) {	// and if the z dimension is also bigger than the y dimension...

			// divide along z axis into 2 equal sized boxes
//			halfPoint = node.box.parameters[0].z() + (node.box.parameters[1].z() - node.box.parameters[0].z()) / 2;

			// divide along mean of data points on the z axis
			halfPoint = 0;
			for (int i = 0; i < node.points.size(); i++) halfPoint += mesh.getVertex(node.points[i]).z;
			halfPoint = halfPoint / node.points.size();

			subDivideBox(node.box, childBoxes, halfPoint, "z");
		}
		else {																														// or if the y dimension is bigger than the z dimension.
					
			// divide along y axis into 2 equal sized boxes
			halfPoint = node.box.parameters[0].y() + (node.box.parameters[1].y() - node.box.parameters[0].y()) / 2;

			// divide along mean of data points on the y axis
//			halfPoint = 0;
//			for (int i = 0; i < node.points.size(); i++) halfPoint += mesh.getVertex(node.points[i]).y;
//			halfPoint = halfPoint / node.points.size();

			subDivideBox(node.box, childBoxes, halfPoint, "y");
		}
	}
	*/

	subDivideBox(node.box, childBoxes);

	getMeshPointsInBox(mesh, node.points, childBoxes[0], childPoints1);
	getMeshPointsInBox(mesh, node.points, childBoxes[1], childPoints2);
	getMeshPointsInBox(mesh, node.points, childBoxes[2], childPoints3);
	getMeshPointsInBox(mesh, node.points, childBoxes[3], childPoints4);
	getMeshPointsInBox(mesh, node.points, childBoxes[4], childPoints5);
	getMeshPointsInBox(mesh, node.points, childBoxes[5], childPoints6);
	getMeshPointsInBox(mesh, node.points, childBoxes[6], childPoints7);
	getMeshPointsInBox(mesh, node.points, childBoxes[7], childPoints8);

	if (childPoints1.size() > 0) {
		node.children.push_back(TreeNode());
		node.children.back().box = childBoxes[0];
		node.children.back().points = childPoints1;
	}
	if (childPoints2.size() > 0) {
		node.children.push_back(TreeNode());
		node.children.back().box = childBoxes[1];
		node.children.back().points = childPoints2;
	}
	if (childPoints3.size() > 0) {
		node.children.push_back(TreeNode());
		node.children.back().box = childBoxes[2];
		node.children.back().points = childPoints3;
	}
	if (childPoints4.size() > 0) {
		node.children.push_back(TreeNode());
		node.children.back().box = childBoxes[3];
		node.children.back().points = childPoints4;
	}
	if (childPoints5.size() > 0) {
		node.children.push_back(TreeNode());
		node.children.back().box = childBoxes[4];
		node.children.back().points = childPoints5;
	}
	if (childPoints6.size() > 0) {
		node.children.push_back(TreeNode());
		node.children.back().box = childBoxes[5];
		node.children.back().points = childPoints6;
	}
	if (childPoints7.size() > 0) {
		node.children.push_back(TreeNode());
		node.children.back().box = childBoxes[6];
		node.children.back().points = childPoints7;
	}
	if (childPoints8.size() > 0) {
		node.children.push_back(TreeNode());
		node.children.back().box = childBoxes[7];
		node.children.back().points = childPoints8;
	}

	//recursively subdivide the child nodes
	for (int i  = 0; i < node.children.size(); i++) subdivide(mesh, node.children[i], numLevels, level + 1);
}

bool KdTree::intersect(const Ray &ray, const TreeNode & node, TreeNode & nodeRtn) {
	// if the ray doesn't pass through this node, just return now
	if (!node.box.intersect(ray, 0, 1000)) return false;

	// test if this is the leaf node that the mouse is clicking
	if (node.children.size() == 0) {
		nodeRtn = node;
		return true;
	}

	bool result = false;
	for (int i = 0; i < node.children.size(); i++) {
		result = result || intersect(ray, node.children[i], nodeRtn);
	}

	return result;
}


bool KdTree::checkCollision(const Ray &ray, const TreeNode & node, TreeNode & nodeRtn) {
	// if the ray doesn't pass through this node, just return now
	if (!node.box.intersect(ray, 0, 1000)) return false;

	// test if this is the leaf node that the mouse is clicking
	if (node.children.size() == 0) {
		nodeRtn = node;
		return true;
	}

	bool result = false;
	for (int i = 0; i < node.children.size(); i++) {
		result = result || intersect(ray, node.children[i], nodeRtn);
	}

	return result;
}
