/*
Copyright (c) 2011-2012 Hiroshi Tsubokawa
See LICENSE and README
*/

#ifndef TRANSFORM_H
#define TRANSFORM_H

#include "Property.h"

#ifdef __cplusplus
extern "C" {
#endif

enum TransformOrder {
	ORDER_SRT = 0,
	ORDER_STR,
	ORDER_RST,
	ORDER_RTS,
	ORDER_TRS,
	ORDER_TSR,
	ORDER_XYZ,
	ORDER_XZY,
	ORDER_YXZ,
	ORDER_YZX,
	ORDER_ZXY,
	ORDER_ZYX
};

struct Transform {
	double matrix[16];
	double inverse[16];

	int transform_order;
	int rotate_order;

	double translate[3];
	double rotate[3];
	double scale[3];
};

extern void XfmReset(struct Transform *transform);

extern void XfmTransformPoint(const struct Transform *transform, double *point);
extern void XfmTransformVector(const struct Transform *transform, double *vector);
extern void XfmTransformBounds(const struct Transform *transform, double *bounds);

extern void XfmTransformPointInverse(const struct Transform *transform, double *point);
extern void XfmTransformVectorInverse(const struct Transform *transform, double *vector);
extern void XfmTransformBoundsInverse(const struct Transform *transform, double *bounds);

extern void XfmSetTranslate(struct Transform *transform, double tx, double ty, double tz);
extern void XfmSetRotate(struct Transform *transform, double rx, double ry, double rz);
extern void XfmSetScale(struct Transform *transform, double sx, double sy, double sz);
extern void XfmSetTransformOrder(struct Transform *transform, int order);
extern void XfmSetRotateOrder(struct Transform *transform, int order);
extern void XfmSetTransform(struct Transform *transform,
		int transform_order, int rotate_order,
		double tx, double ty, double tz,
		double rx, double ry, double rz,
		double sx, double sy, double sz);

extern int XfmIsTransformOrder(int order);
extern int XfmIsRotateOrder(int order);

/* TransformSampleList */
struct TransformSampleList {
	struct PropertySampleList translate;
	struct PropertySampleList rotate;
	struct PropertySampleList scale;
	int transform_order;
	int rotate_order;

	struct Transform transform_sample;
	double last_sample_time;
};

extern void XfmInitTransformSampleList(struct TransformSampleList *list);
extern void XfmLerpTransformSample(struct TransformSampleList *list, double time);

extern void XfmPushTranslateSample(struct TransformSampleList *list,
		double tx, double ty, double tz, double time);
extern void XfmPushRotateSample(struct TransformSampleList *list,
		double rx, double ry, double rz, double time);
extern void XfmPushScaleSample(struct TransformSampleList *list,
		double sx, double sy, double sz, double time);

extern void XfmSetSampleTransformOrder(struct TransformSampleList *list, int order);
extern void XfmSetSampleRotateOrder(struct TransformSampleList *list, int order);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* XXX_H */

