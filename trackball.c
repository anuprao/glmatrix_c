/* Trackball helpers
 *
 * Copyright (c)  2020 Anup Jayapal Rao
 * 				  - Minor rewrite to port to glmatrix datatypes
 * 
 * Copyright (c) 2014 Nicolas Rougier
 *               2008 Roger Allen
 *               1993, 1994, Silicon Graphics, Inc.
 *
 * Implementation of a virtual trackball.
 * Implemented by Gavin Bell, lots of ideas from Thant Tessman and
 * the August '88 issue of Siggraph's "Computer Graphics," pp. 121-129.
 *
 * Original code from:
 * David M. Ciemiewicz, Mark Grossman, Henry Moreton, and Paul Haeberli
*/
#include <stdlib.h>
#include <math.h>

#include "gl-matrix.h"

WARNING: PORT-IN-PROGRESS

// Given an axis and angle, compute quaternion.
def axis_to_quat(a, phi, q)
{
	normalised_a = vec3_create(NULL);
	vec3_normalize(a, normalised_a);
	
	q[0] = normalised_a[0];
	q[1] = normalised_a[1];
	q[2] = normalised_a[2];
	
	vec3_scale(q, sin(phi/2.0), NULL);
	
	q[3] = cos(phi/2.0);
}


// Project an x,y pair onto a sphere of radius r OR a hyperbolic sheet
// if we are away from the center of the sphere.
def tb_project_to_sphere(r, x, y)
{
	d = sqrt(x*x + y*y);

	if d < (r * 0.70710678118654752440)
	{
		// Inside sphere 
		z = sqrt(r*r - d*d);
	}
	else 
	{
		// On hyperbola 
		t = r / 1.41421356237309504880;
		z = t*t / d;
	}

	return z;
}
   
// Given two rotations, e1 and e2, expressed as quaternion rotations,
// figure out the equivalent single rotation and stuff it into dest.
// 
// This routine also normalizes the result every RENORMCOUNT times it is
// called, to keep error from creeping in.
// 
// NOTE: This routine is written so that q1 or q2 may be the same
// as dest (or each other).
// 
RENORMCOUNT=97;

def add_quats(q1, q2, dest, renormcounter)
{
    t1 = quat_create(q1);
    t2 = quat_create(q2);
    t3 = quat_create(NULL);
    tf = quat_create(NULL);
    
    // Note: using vec3 fxn on quat
    vec3_scale(t1, q2[3], NULL);
    
    // Note: using vec3 fxn on quat
    vec3_scale(t2, q1[3], NULL);
    
    // Note: using vec3 fxn on quat
    vec3_cross(q2, q1, t3);
    
    // Note: using vec3 fxn on quat
    vec3_add(t1, t2, tf);
    
    // Note: using vec3 fxn on quat
    vec3_add(t3, tf, tf);
    
    d1_dot_q2_using_vec3_dot = vec3_dot(q1, q2);
    tf[3] = q1[3] * q2[3] - d1_dot_q2_using_vec3_dot;

    dest[0] = tf[0];
    dest[1] = tf[1];
    dest[2] = tf[2];
    dest[3] = tf[3];

    if (++renormcounter > RENORMCOUNT)
    {
        renormcounter = 0;
        quat_normalize(dest, NULL);
	}
}

// This size should really be based on the distance from the center of
// rotation to the point on the object underneath the mouse.  That
// point would then track the mouse as closely as possible.  This is a
// simple example, though, so that is left as an Exercise for the
// Programmer.
TRACKBALLSIZE=0.8;

// Ok, simulate a track-ball.  Project the points onto the virtual
// trackball, then figure out the axis of rotation, which is the cross
// product of P1 P2 and O P1 (O is the center of the ball, 0,0,0)
// Note:  This is a deformed trackball-- is a trackball in the center,
// but is deformed into a hyperbolic sheet of rotation away from the
// center.  This particular function was chosen after trying out
// several variations.
// 
// It is assumed that the arguments to this routine are in the range
// (-1.0 ... 1.0)
// 

def trackball(q, p1x, p1y, p2x, p2y)
{
	p1 = vec3_create(NULL); 
	p2 = vec3_create(NULL);
	
	// Axis of rotation 
	a = vec3_create(NULL);
	d = vec3_create(NULL);

	if ((p1x == p2x) && (p1y == p2y))
	{
		// Zero rotation 
		q[0] = 0.0;
		q[1] = 0.0;
		q[2] = 0.0;
		q[3] = 1.0;
		
		return;
	}

	// First, figure out z-coordinates for projection of P1 and P2 to
	// deformed sphere 
	p1[0] = p1x;
	p1[1] = p1y;
	p1[2] = tb_project_to_sphere(TRACKBALLSIZE, p1x, p1y);

	p2[0] = p2x;
	p2[1] = p2y;
	p2[2] = tb_project_to_sphere(TRACKBALLSIZE, p2x, p2y);

	// Now, we want the cross product of P1 and P2
	vec3_cross(p2, p1, a);

	// Figure out how much to rotate around that axis.
	vec3_subtract(p1, p2, d);
	t = vec3_length(d) / (2.0*TRACKBALLSIZE);

	// Avoid problems with out-of-control values...
	if (t > 1.0)
	{
		t = 1.0;
	}
	
	if (t < -1.0)
	{
		t = -1.0;
	}

	// how much to rotate about axis
	phi = 2.0 * asin(t);

	axis_to_quat(a, phi, q);
}		
