/*
Copyright (c) 2011-2012 Hiroshi Tsubokawa
See LICENSE and README
*/

#include "ObjectInstance.h"
#include "Intersection.h"
#include "Accelerator.h"
#include "Interval.h"
#include "Property.h"
#include "Numeric.h"
#include "Vector.h"
#include "Volume.h"
#include "Matrix.h"
#include "Array.h"
#include "Box.h"
#include "Ray.h"

#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <float.h>

struct ObjectInstance {
	/* geometric properties */
	const struct Accelerator *acc;
	const struct Volume *volume;
	double bounds[6];

	/* transformation properties */
	struct TransformSampleList transform_samples;

	/* non-geometric properties */
	const struct Shader *shader;
	const struct Light **target_lights;
	int n_target_lights;
	const struct ObjectGroup *reflection_target;
	const struct ObjectGroup *refraction_target;
};

static void update_object_bounds(struct ObjectInstance *obj);
static void merge_sampled_bounds(struct ObjectInstance *obj);
static const struct Transform *get_interpolated_transform(const struct ObjectInstance *obj,
		double time);

/* TODO remove acc argument */
/* ObjectInstance interfaces */
struct ObjectInstance *ObjNew(const struct Accelerator *acc)
{
	struct ObjectInstance *obj;

	obj = (struct ObjectInstance *) malloc(sizeof(struct ObjectInstance));
	if (obj == NULL)
		return NULL;

	obj->acc = acc;
	obj->volume = NULL;

	XfmInitTransformSampleList(&obj->transform_samples);
	update_object_bounds(obj);

	obj->shader = NULL;
	obj->target_lights = NULL;
	obj->n_target_lights = 0;

	obj->reflection_target = NULL;
	obj->refraction_target = NULL;

	return obj;
}

void ObjFree(struct ObjectInstance *obj)
{
	if (obj == NULL)
		return;
	free(obj);
}

int ObjSetVolume(struct ObjectInstance *obj, const struct Volume *volume)
{
	if (obj->acc != NULL)
		return -1;

	if (obj->volume != NULL)
		return -1;

	obj->volume = volume;
	update_object_bounds(obj);

	assert(obj->acc == NULL && obj->volume != NULL);
	return 0;
}

int ObjIsSurface(const struct ObjectInstance *obj)
{
	if (obj->acc == NULL)
		return 0;

	assert(obj->volume == NULL);
	return 1;
}

int ObjIsVolume(const struct ObjectInstance *obj)
{
	if (obj->volume == NULL)
		return 0;

	assert(obj->acc == NULL);
	return 1;
}

void ObjSetTranslate(struct ObjectInstance *obj,
		double tx, double ty, double tz, double time)
{
	XfmPushTranslateSample(&obj->transform_samples, tx, ty, tz, time);
	update_object_bounds(obj);
}

void ObjSetRotate(struct ObjectInstance *obj,
		double rx, double ry, double rz, double time)
{
	XfmPushRotateSample(&obj->transform_samples, rx, ry, rz, time);
	update_object_bounds(obj);
}

void ObjSetScale(struct ObjectInstance *obj,
		double sx, double sy, double sz, double time)
{
	XfmPushScaleSample(&obj->transform_samples, sx, sy, sz, time);
	update_object_bounds(obj);
}

void ObjSetTransformOrder(struct ObjectInstance *obj, int order)
{
	XfmSetSampleTransformOrder(&obj->transform_samples, order);
	update_object_bounds(obj);
}

void ObjSetRotateOrder(struct ObjectInstance *obj, int order)
{
	XfmSetSampleRotateOrder(&obj->transform_samples, order);
	update_object_bounds(obj);
}

void ObjSetShader(struct ObjectInstance *obj, const struct Shader *shader)
{
	obj->shader = shader;
}

void ObjSetLightList(struct ObjectInstance *obj, const struct Light **lights, int count)
{
	obj->target_lights = lights;
	obj->n_target_lights = count;
}

void ObjSetReflectTarget(struct ObjectInstance *obj, const struct ObjectGroup *grp)
{
	assert(grp != NULL);
	obj->reflection_target = grp;
}

void ObjSetRefractTarget(struct ObjectInstance *obj, const struct ObjectGroup *grp)
{
	assert(grp != NULL);
	obj->refraction_target = grp;
}

const struct ObjectGroup *ObjGetReflectTarget(const struct ObjectInstance *obj)
{
	return obj->reflection_target;
}

const struct ObjectGroup *ObjGetRefractTarget(const struct ObjectInstance *obj)
{
	return obj->refraction_target;
}

const struct Shader *ObjGetShader(const struct ObjectInstance *obj)
{
	return obj->shader;
}

const struct Light **ObjGetLightList(const struct ObjectInstance *obj)
{
	return obj->target_lights;
}

int ObjGetLightCount(const struct ObjectInstance *obj)
{
	return obj->n_target_lights;
}

void ObjGetBounds(const struct ObjectInstance *obj, double *bounds)
{
	BOX3_COPY(bounds, obj->bounds);
}

int ObjIntersect(const struct ObjectInstance *obj, double time,
		const struct Ray *ray, struct Intersection *isect)
{
	const struct Transform *transform_interp = NULL;
	struct Ray ray_object_space;
	int hit = 0;

	if (!ObjIsSurface(obj))
		return 0;

	transform_interp = get_interpolated_transform(obj, time);

	/* transform ray to object space */
	ray_object_space = *ray;
	XfmTransformPointInverse(transform_interp, ray_object_space.orig);
	XfmTransformVectorInverse(transform_interp, ray_object_space.dir);

	hit = AccIntersect(obj->acc, time, &ray_object_space, isect);
	if (!hit)
		return 0;

	/* transform intersection back to world space */
	XfmTransformPoint(transform_interp, isect->P);
	XfmTransformVector(transform_interp, isect->N);
	VEC3_NORMALIZE(isect->N);

	/* TODO should make TransformLocalGeometry? */
	XfmTransformVector(transform_interp, isect->dPds);
	XfmTransformVector(transform_interp, isect->dPdt);

	isect->object = obj;

	return 1;
}

int ObjVolumeIntersect(const struct ObjectInstance *obj, double time,
			const struct Ray *ray, struct Interval *interval)
{
	const struct Transform *transform_interp = NULL;
	struct Ray ray_object_space;
	double volume_bounds[6] = {0};
	double boxhit_tmin = 0;
	double boxhit_tmax = 0;
	int hit = 0;

	if (!ObjIsVolume(obj))
		return 0;

	VolGetBounds(obj->volume, volume_bounds);

	transform_interp = get_interpolated_transform(obj, time);

	/* transform ray to object space */
	ray_object_space = *ray;
	XfmTransformPointInverse(transform_interp, ray_object_space.orig);
	XfmTransformVectorInverse(transform_interp, ray_object_space.dir);

	hit = BoxRayIntersect(volume_bounds,
			ray_object_space.orig,
			ray_object_space.dir,
			ray_object_space.tmin,
			ray_object_space.tmax,
			&boxhit_tmin, &boxhit_tmax);

	if (!hit) {
		return 0;
	}

	interval->tmin = boxhit_tmin;
	interval->tmax = boxhit_tmax;
	interval->object = obj;

	return 1;
}

int ObjGetVolumeSample(const struct ObjectInstance *obj, double time,
			const double *point, struct VolumeSample *sample)
{
	const struct Transform *transform_interp = NULL;
	double point_in_objspace[3] = {0};
	int hit = 0;

	if (!ObjIsVolume(obj))
		return 0;

	transform_interp = get_interpolated_transform(obj, time);

	VEC3_COPY(point_in_objspace, point);
	XfmTransformPointInverse(transform_interp, point_in_objspace);

	hit = VolGetSample(obj->volume, point_in_objspace, sample);

	return hit;
}

static const struct Transform *get_interpolated_transform(const struct ObjectInstance *obj,
		double time)
{
	struct TransformSampleList *mutable_transform_list =
			(struct TransformSampleList *) &obj->transform_samples;

	XfmLerpTransformSample(mutable_transform_list, time);
	return &obj->transform_samples.transform_sample;
}

static void update_object_bounds(struct ObjectInstance *obj)
{
	if (ObjIsSurface(obj)) {
		AccGetBounds(obj->acc, obj->bounds);
	}
	else if (ObjIsVolume(obj)) {
		VolGetBounds(obj->volume, obj->bounds);
	}
	else {
		/* TODO TEST allowing state where neither surface nor volume */
		/*
		printf("fatal error: object is neither surface nor volume\n");
		abort();
		*/
		BOX3_SET(obj->bounds, FLT_MAX, FLT_MAX, FLT_MAX, -FLT_MAX, -FLT_MAX, -FLT_MAX);
	}
	merge_sampled_bounds(obj);
}

static void merge_sampled_bounds(struct ObjectInstance *obj)
{
	double merged_bounds[6] = {FLT_MAX, FLT_MAX, FLT_MAX, -FLT_MAX, -FLT_MAX, -FLT_MAX};
	double original_bounds[6] = {FLT_MAX, FLT_MAX, FLT_MAX, -FLT_MAX, -FLT_MAX, -FLT_MAX};
	double S[3] = {1, 1, 1};
	int i;

	/* assumes obj->bounds is the same as acc->bounds or vol->bounds */
	ObjGetBounds(obj, original_bounds);

	/* extend bounds when rotated to ensure object is always inside bounds */
	if (obj->transform_samples.rotate.sample_count > 1) {
		const double diagonal = BoxDiagonal(original_bounds);
		double centroid[3] = {0};

		BoxCentroid(original_bounds, centroid);
		BOX3_SET(original_bounds,
				centroid[0] - diagonal,
				centroid[1] - diagonal,
				centroid[2] - diagonal,
				centroid[0] + diagonal,
				centroid[1] + diagonal,
				centroid[2] + diagonal);
	}

	/* compute maximum scale over sampling time */
	VEC3_COPY(S, obj->transform_samples.scale.samples[0].vector);
	for (i = 1; i < obj->transform_samples.scale.sample_count; i++) {
		S[0] = MAX(S[0], obj->transform_samples.scale.samples[i].vector[0]);
		S[1] = MAX(S[1], obj->transform_samples.scale.samples[i].vector[1]);
		S[2] = MAX(S[2], obj->transform_samples.scale.samples[i].vector[2]);
	}

	/* accumulate all bounds over sampling time */
	for (i = 0; i < obj->transform_samples.translate.sample_count; i++) {
		double sample_bounds[6] = {FLT_MAX, FLT_MAX, FLT_MAX, -FLT_MAX, -FLT_MAX, -FLT_MAX};
		const double *T = obj->transform_samples.translate.samples[i].vector;
		const double *R = obj->transform_samples.rotate.samples[i].vector;
		struct Transform transform;

		XfmSetTransform(&transform,
			obj->transform_samples.transform_order, obj->transform_samples.rotate_order,
			T[0], T[1], T[2],
			R[0], R[1], R[2],
			S[0], S[1], S[2]);

		BOX3_COPY(sample_bounds, original_bounds);
		XfmTransformBounds(&transform, sample_bounds);
		BoxAddBox(merged_bounds, sample_bounds);
	}

	BOX3_COPY(obj->bounds, merged_bounds);
}

