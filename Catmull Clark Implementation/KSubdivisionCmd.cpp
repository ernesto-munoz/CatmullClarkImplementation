#include "KSubdivisionCmd.h"

const char *numIterationsFlag = "-n", *numIterationsLongFlag = "-numIterations";

const MString KSubdivisionCmd::melCommand = "ksubdivision";

MStatus KSubdivisionCmd::doIt(const MArgList &args)
{
	MStatus status;

	MArgDatabase argData(syntax(), args);
	unsigned int numIterations = 1;
	if (argData.isFlagSet(numIterationsFlag)) {
		argData.getFlagArgument(numIterationsFlag, 0, numIterations);
	}

	MSelectionList selection;
	MGlobal::getActiveSelectionList(selection);


	MDagPath dagPath;
	MFnMesh meshFn;
	MString name;

	MItSelectionList iter(selection, MFn::kMesh);
	for (; !iter.isDone(); iter.next()) {

		// Get where the In Mesh attribute is connected
		MObject meshNode;
		iter.getDependNode(meshNode);

		MFnDependencyNode meshDependencyNodeFn(meshNode);
		MPlug inputMeshPlug = meshDependencyNodeFn.findPlug("inMesh", &status);
		CHECK_MSTATUS(status);

		MPlugArray sourcePlugs;
		inputMeshPlug.connectedTo(sourcePlugs, true, false, &status);
		CHECK_MSTATUS(status);
		if (sourcePlugs.length() <= 0) continue;
		MPlug sourcePlug = sourcePlugs[0];  // only needed the first

		// Create the node
		MObject subdivisionNode = dgModifier.createNode(KSubdivisionNode::name, &status);
		dgModifier.doIt();
		CHECK_MSTATUS(status);
		MFnDependencyNode subdivisionNodeDependencyFn;
		status = subdivisionNodeDependencyFn.setObject(subdivisionNode);

		
		MPlug nNumIterationsPlug = subdivisionNodeDependencyFn.findPlug("numIterations", &status);
		CHECK_MSTATUS(status);
		MPlug nInputMeshPlug = subdivisionNodeDependencyFn.findPlug("inputMesh", &status);
		CHECK_MSTATUS(status);
		MPlug nOutputMeshPlug = subdivisionNodeDependencyFn.findPlug("outputMesh", &status);
		CHECK_MSTATUS(status);

		// Disconnect and connect
		status = dgModifier.disconnect(sourcePlug, inputMeshPlug);
		CHECK_MSTATUS(status);


		nNumIterationsPlug.setInt(numIterations);
		status = dgModifier.connect(sourcePlug, nInputMeshPlug);
		CHECK_MSTATUS(status);
		status = dgModifier.connect(nOutputMeshPlug, inputMeshPlug);
		CHECK_MSTATUS(status);
	}

	return redoIt();
}

MStatus KSubdivisionCmd::undoIt()
{
	return dgModifier.undoIt();
}

MStatus KSubdivisionCmd::redoIt()
{
	return dgModifier.doIt();
}

void * KSubdivisionCmd::creator()
{
	return new KSubdivisionCmd();
}

MSyntax KSubdivisionCmd::newSyntax()
{
	MSyntax syntax;
	syntax.addFlag(numIterationsFlag, numIterationsLongFlag, MSyntax::kLong);
	return syntax;
}
