/*
Copyright (c) 2011-2012 Hiroshi Tsubokawa
See LICENSE and README
*/

#ifndef MESHIO_H
#define MESHIO_H

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

struct Mesh;

enum MshErrorNo {
	MSH_ERR_NONE = 0,
	MSH_ERR_FILE_NOT_EXIST,
	MSH_ERR_BAD_MAGIC_NUMBER,
	MSH_ERR_BAD_FILE_VERSION,
	MSH_ERR_LONG_ATTRIB_NAME,
	MSH_ERR_NO_MEMORY
};

struct MeshInput {
	FILE *file;
	int version;
	int nverts;
	int nvert_attrs;
	int nfaces;
	int nface_attrs;

	double *P;
	double *N;
	int *indices;

	char **attr_names;
};

struct MeshOutput {
	FILE *file;
	int version;
	int nverts;
	int nvert_attrs;
	int nfaces;
	int nface_attrs;

	double *P;
	double *N;
	float *Cd;
	float *uv;
	int *indices;
};

/* mesh input file interfaces */
extern struct MeshInput *MshOpenInputFile(const char *filename);
extern void MshCloseInputFile(struct MeshInput *in);
extern int MshReadHeader(struct MeshInput *in);
extern int MshReadAttribute(struct MeshInput *in, void *data);

/* mesh output file interfaces */
extern struct MeshOutput *MshOpenOutputFile(const char *filename);
extern void MshCloseOutputFile(struct MeshOutput *out);
extern void MshWriteFile(struct MeshOutput *out);

/* high level interface for loading mesh file */
extern int MshLoadFile(struct Mesh *mesh, const char *filename);

/* error no interfaces */
extern int MshGetErrorNo(void);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* XXX_H */

