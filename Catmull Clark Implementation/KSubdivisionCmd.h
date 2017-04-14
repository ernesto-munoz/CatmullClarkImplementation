#pragma once

#include <maya\MGlobal.h>
#include <maya\MPxCommand.h>
#include <maya\MArgList.h>
#include <maya\MDGModifier.h>
#include <maya\MSelectionList.h>
#include <maya\MFnMesh.h>
#include <maya\MItSelectionList.h>
#include <maya\MDagPath.h>
#include <maya\MFnDependencyNode.h>
#include <maya\MPlug.h>
#include <maya\MPlugArray.h>
#include <maya\MSyntax.h>
#include <maya\MArgDatabase.h>


#include "KSubdivisionNode.h"

class KSubdivisionCmd : public MPxCommand
{
public:
	virtual MStatus doIt(const MArgList&);
	virtual MStatus undoIt();
	virtual MStatus redoIt();
	virtual bool isUndoable() const { return true; }

	static void *creator();
	static MSyntax newSyntax();

	static const MString melCommand;

private:
	MDGModifier dgModifier;
	
};

