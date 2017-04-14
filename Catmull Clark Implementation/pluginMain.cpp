#include <maya\MObject.h>
#include <maya\MStatus.h>
#include <maya\MFnPlugin.h>

#include "KSubdivisionCmd.h"
#include "KSubdivisionNode.h"

MStatus initializePlugin(MObject obj) {
	MStatus status;
	MFnPlugin pluginFn(obj, "ernesto", "1.0", "Any");

	// Regsitration of the command
	status = pluginFn.registerCommand(KSubdivisionCmd::melCommand, KSubdivisionCmd::creator, KSubdivisionCmd::newSyntax);
	CHECK_MSTATUS(status);

	// Registration of the node
	status = pluginFn.registerNode(
		KSubdivisionNode::name,
		KSubdivisionNode::id,
		&KSubdivisionNode::creator,
		&KSubdivisionNode::initialize
	);
	CHECK_MSTATUS(status);

	std::cout.rdbuf(std::cerr.rdbuf()); // redirect output to the console
	return status;
}

MStatus uninitializePlugin(MObject obj) {
	MStatus status;
	MFnPlugin pluginFn(obj);

	// Deregister command
	status = pluginFn.deregisterCommand(KSubdivisionCmd::melCommand);
	CHECK_MSTATUS(status);

	// Deregister of the node
	status = pluginFn.deregisterNode(KSubdivisionNode::id);
	CHECK_MSTATUS(status);

	return status;
}
