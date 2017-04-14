#pragma once

#include <maya\MGlobal.h>
#include <maya\MPxNode.h>
#include <maya\MFnTypedAttribute.h>
#include <maya\MFnMesh.h>
#include <maya\MPointArray.h>
#include <maya\MItMeshPolygon.h>
#include <maya\MItMeshEdge.h>
#include <maya\MItMeshVertex.h>
#include <maya\MFnNumericAttribute.h>
#include <maya\MFnMeshData.h>

class KSubdivisionNode : public MPxNode
{
public:
	virtual MStatus compute(const MPlug & plug, MDataBlock & data);

	static void * creator();
	static MStatus initialize();

	static MTypeId id;
	static MString name;

	static MObject numIterations;
	static MObject inputMesh;
	static MObject outputMesh;

private:
	void applyCatmullClark(MObject & inputMeshObj, MObject & outputMeshObj);

};

