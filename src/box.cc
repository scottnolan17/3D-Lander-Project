#include "vector3.h"
#include "ray.h"
#include "box.h"
  
/*
 * Ray-box intersection using IEEE numerical properties to ensure that the
 * test is both robust and efficient, as described in:
 *
 *      Amy Williams, Steve Barrus, R. Keith Morley, and Peter Shirley
 *      "An Efficient and Robust Ray-Box Intersection Algorithm"
 *      Journal of graphics tools, 10(1):49-54, 2005
 *
 */

bool Box::intersect(const Ray &r, float t0, float t1) const {
  float tmin, tmax, tymin, tymax, tzmin, tzmax;

  tmin = (parameters[r.sign[0]].x() - r.origin.x()) * r.inv_direction.x();
  tmax = (parameters[1-r.sign[0]].x() - r.origin.x()) * r.inv_direction.x();
  tymin = (parameters[r.sign[1]].y() - r.origin.y()) * r.inv_direction.y();
  tymax = (parameters[1-r.sign[1]].y() - r.origin.y()) * r.inv_direction.y();
  if ( (tmin > tymax) || (tymin > tmax) ) 
    return false;
  if (tymin > tmin)
    tmin = tymin;
  if (tymax < tmax)
    tmax = tymax;
  tzmin = (parameters[r.sign[2]].z() - r.origin.z()) * r.inv_direction.z();
  tzmax = (parameters[1-r.sign[2]].z() - r.origin.z()) * r.inv_direction.z();
  if ( (tmin > tzmax) || (tzmin > tmax) ) 
    return false;
  if (tzmin > tmin)
    tmin = tzmin;
  if (tzmax < tmax)
    tmax = tzmax;
  return ( (tmin < t1) && (tmax > t0) );
}

bool Box::collide(Box &other) {
	return	(this->min().x() <= other.max().x() && this->max().x() >= other.min().x()) &&
			(this->min().y() <= other.max().y() && this->max().y() >= other.min().y()) &&
			(this->min().z() <= other.max().z() && this->max().z() >= other.min().z());
}			

bool Box::contains(Vector3 &point) {
	return this->min().x() <= point.x() && point.x() <= this->max().x()
		&& this->min().y() <= point.y() && point.y() <= this->max().y()
		&& this->min().z() <= point.z() && point.z() <= this->max().z();
}
																					











