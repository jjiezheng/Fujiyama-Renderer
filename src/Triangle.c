/*
Copyright (c) 2011-2012 Hiroshi Tsubokawa
See LICENSE and README
*/

#include "Triangle.h"
#include "Numeric.h"
#include "Vector.h"
#include "Box.h"
#include <float.h>

#define EPSILON 1e-6

double TriComputeArea(const double *vert0, const double *vert1, const double *vert2)
{
	double a[3];
	double b[3];
	double cross[3];

	VEC3_SUB(a, vert1, vert0);
	VEC3_SUB(b, vert2, vert0);
	VEC3_CROSS(cross, a, b);

	return .5 * VEC3_LEN(cross);
}

void TriComputeBounds(
		double *box,
		const double *vert0, const double *vert1, const double *vert2)
{
	BOX3_SET(box, FLT_MAX, FLT_MAX, FLT_MAX, -FLT_MAX, -FLT_MAX, -FLT_MAX);

	BoxAddPoint(box, vert0);
	BoxAddPoint(box, vert1);
	BoxAddPoint(box, vert2);
}

void TriComputeFaceNormal(
		double *N,
		const double *vert0, const double *vert1, const double *vert2)
{
	double edge1[3], edge2[3];
	VEC3_SUB(edge1, vert1, vert0);
	VEC3_SUB(edge2, vert2, vert0);

	VEC3_CROSS(N, edge1, edge2);
	VEC3_NORMALIZE(N);
}

void TriComputeNormal(
		double *N,
		const double *N0, const double *N1, const double *N2,
		double u, double v)
{
	/* N = (1-u-v) * N0 + u * N1 + v * N2 */
	double t = 1-u-v;
	N[0] = t * N0[0] + u * N1[0] + v * N2[0];
	N[1] = t * N0[1] + u * N1[1] + v * N2[1];
	N[2] = t * N0[2] + u * N1[2] + v * N2[2];
}

/* Codes from
 * Fast, minimum storage ray-triangle intersection.
 * Tomas Möller and Ben Trumbore.
 * Journal of Graphics Tools, 2(1):21--28, 1997.
 */
int TriRayIntersect(
		const double *vert0, const double *vert1, const double *vert2,
		const double *orig, const double *dir, int cull_backfaces,
		double *t, double *u, double *v)
{
	double edge1[3], edge2[3], tvec[3], pvec[3], qvec[3];
	double det, inv_det;

	/* find vectors for two edges sharing vert0 */
	VEC3_SUB(edge1, vert1, vert0);
	VEC3_SUB(edge2, vert2, vert0);

	/* begin calculating determinant - also used to calculate U parameter */
	VEC3_CROSS(pvec, dir, edge2);

	/* if determinant is near zero, ray lies in plane of triangle */
	det = VEC3_DOT(edge1, pvec);

/*#ifdef TEST_CULL*/           /* define TEST_CULL if culling is desired */
	if (cull_backfaces) {
		if (det < EPSILON)
			return 0;

		/* calculate distance from vert0 to ray origin */
		VEC3_SUB(tvec, orig, vert0);

		/* calculate U parameter and test bounds */
		*u = VEC3_DOT(tvec, pvec);
		if (*u < 0.0 || *u > det)
			return 0;

		/* prepare to test V parameter */
		VEC3_CROSS(qvec, tvec, edge1);

		/* calculate V parameter and test bounds */
		*v = VEC3_DOT(dir, qvec);
		if (*v < 0.0 || *u + *v > det)
			return 0;

		/* calculate t, scale parameters, ray intersects triangle */
		*t = VEC3_DOT(edge2, qvec);
		inv_det = 1.0 / det;
		*t *= inv_det;
		*u *= inv_det;
		*v *= inv_det;
/*#else*/                    /* the non-culling branch */
	} else {
		if (det > -EPSILON && det < EPSILON)
			return 0;
		inv_det = 1.0 / det;

		/* calculate distance from vert0 to ray origin */
		VEC3_SUB(tvec, orig, vert0);

		/* calculate U parameter and test bounds */
		*u = VEC3_DOT(tvec, pvec) * inv_det;
		if (*u < 0.0 || *u > 1.0)
			return 0;

		/* prepare to test V parameter */
		VEC3_CROSS(qvec, tvec, edge1);

		/* calculate V parameter and test bounds */
		*v = VEC3_DOT(dir, qvec) * inv_det;
		if (*v < 0.0 || *u + *v > 1.0)
			return 0;

		/* calculate t, ray intersects triangle */
		*t = VEC3_DOT(edge2, qvec) * inv_det;
/*#endif*/
	}
	return 1;
}

