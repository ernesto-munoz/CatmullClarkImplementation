#include "KSubdivisionNode.h"

MTypeId KSubdivisionNode::id(0x00333);
MString KSubdivisionNode::name("KSubdivisionNode");

MObject KSubdivisionNode::inputMesh;
MObject KSubdivisionNode::outputMesh;
MObject KSubdivisionNode::numIterations;

MStatus KSubdivisionNode::compute(const MPlug & plug, MDataBlock & data)
{
	MStatus status;

	if (plug == outputMesh) {
		cout << "Begin Catmull Clark" << endl;
		// Get Input
		MDataHandle inputDataHandle = data.inputValue(numIterations, &status); CHECK_MSTATUS(status);
		int numIterationsValue = inputDataHandle.asInt();

		inputDataHandle = data.inputValue(inputMesh, &status); CHECK_MSTATUS(status);
		MObject inputMeshObj = inputDataHandle.asMesh();

		// Copy the input mesh
		MFnMeshData meshDataFn;
		MObject outputMeshObj = meshDataFn.create();
		MFnMesh outputMeshFn;
		outputMeshFn.copy(inputMeshObj, outputMeshObj);

		// Apply the catmull clark subdivision numIterations times
		for (int i = 0; i < numIterationsValue; ++i) {
			applyCatmullClark(outputMeshObj, outputMeshObj);
		}
		
		// Set output
		MDataHandle outputDataHandle = data.outputValue(outputMesh, &status);
		CHECK_MSTATUS(status);
		status = outputDataHandle.set(outputMeshObj);
		CHECK_MSTATUS(status);

		data.setClean(plug);
	}
	else {
		return MS::kUnknownParameter;
	}

	return status;
}

void * KSubdivisionNode::creator()
{
	return new KSubdivisionNode();;
}

MStatus KSubdivisionNode::initialize()
{
	MStatus status;

	MFnNumericAttribute nAttr;
	MFnTypedAttribute meshAttr;

	numIterations = nAttr.create("numInterations", "numIterations", MFnNumericData::kInt, 1, &status);
	CHECK_MSTATUS(status);
	nAttr.setKeyable(true);
	nAttr.setStorable(true);

	inputMesh = meshAttr.create("inputMesh", "inputMesh", MFnData::kMesh, &status);
	CHECK_MSTATUS(status);
	meshAttr.setStorable(false);

	outputMesh = meshAttr.create("outputMesh", "outputMesh", MFnData::kMesh, &status);
	CHECK_MSTATUS(status);
	meshAttr.setStorable(false);
	meshAttr.setWritable(false);

	status = addAttribute(numIterations); CHECK_MSTATUS(status);
	status = addAttribute(inputMesh); CHECK_MSTATUS(status);
	status = addAttribute(outputMesh); CHECK_MSTATUS(status);
	
	status = attributeAffects(numIterations, outputMesh); CHECK_MSTATUS(status);
	status = attributeAffects(inputMesh, outputMesh); CHECK_MSTATUS(status);

	return status;
}

void KSubdivisionNode::applyCatmullClark(MObject & inputMeshObj, MObject & outputMeshObj)
{
	MStatus status;

	// Create function for the input and output mesh
	MFnMesh inputMeshFn(inputMeshObj);
	MFnMesh	outputMeshFn(outputMeshObj);

	// Save information about each polygon in the input mesh
	MIntArray* polygonsEdges = new MIntArray[inputMeshFn.numPolygons()];
	MIntArray* polygonsVertices = new MIntArray[inputMeshFn.numPolygons()];

	// Calculate Face Points
	MPointArray facePoints;
	MItMeshPolygon polygonIt(inputMeshObj, &status); CHECK_MSTATUS(status);
	for (; !polygonIt.isDone(); polygonIt.next()) {
		MPoint averagePosition = polygonIt.center(MSpace::kObject, &status); CHECK_MSTATUS(status);
		status = facePoints.append(averagePosition); CHECK_MSTATUS(status);

		// Save information about each polygon
		status = polygonIt.getVertices(polygonsVertices[polygonIt.index()]); CHECK_MSTATUS(status);
		status = polygonIt.getEdges(polygonsEdges[polygonIt.index()]); CHECK_MSTATUS(status);
	}

	// Calculate Edge Points
	MPointArray edgePoints;
	MItMeshEdge edgeIt(inputMeshObj, &status); CHECK_MSTATUS(status);
	for (; !edgeIt.isDone(); edgeIt.next()) {
		// Vertex in the edge
		MPoint firstPoint = edgeIt.point(0);
		MPoint secondPoint = edgeIt.point(1);

		// Conected faces
		MIntArray facesConnected;
		edgeIt.getConnectedFaces(facesConnected, &status);
		CHECK_MSTATUS(status);

		// Calculate the average of 3 or 4 points
		unsigned int numPoints = 3;
		MPoint edgePoint(0.0, 0.0, 0.0);
		edgePoint += firstPoint;
		edgePoint += secondPoint;
		edgePoint += facePoints[facesConnected[0]];
		if (facesConnected.length() == 2) {
			edgePoint += facePoints[facesConnected[1]];
			numPoints++;
		}

		edgePoint *= 1.0f / numPoints;
		edgePoints.append(edgePoint);
	}

	// New position of the vertex
	// Barycenter (Q + 2R + (n-3)P) / n
	// Q averta of surrounding faces points
	// R average if all surroding edge MIDpoints
	// P the point itself
	MPointArray newControlVertexPoints;
	MItMeshVertex vertexIt(inputMeshObj, &status);
	for (; !vertexIt.isDone(); vertexIt.next()) {
		// Connected faces and edges for this vertex
		MIntArray connectedFaces;
		MIntArray connectedEdges;
		unsigned int numConnectedFaces;
		unsigned int numConnectedEdges;
		status = vertexIt.getConnectedEdges(connectedEdges); CHECK_MSTATUS(status);
		status = vertexIt.getConnectedFaces(connectedFaces); CHECK_MSTATUS(status);

		numConnectedEdges = connectedEdges.length();
		numConnectedFaces = connectedFaces.length();

		// valence = num of edges connected to this point (n)
		unsigned valence = numConnectedEdges;

		if (valence < 3) {
			newControlVertexPoints.append(vertexIt.position(MSpace::kObject, &status));
			continue;
		}

		// Average of the surrounding face points
		MPoint averageFacePoint(0.0, 0.0, 0.0);
		for (unsigned int i = 0; i < numConnectedFaces; ++i) {
			averageFacePoint += facePoints[connectedFaces[i]];
		}
		averageFacePoint *= 1.0f / numConnectedFaces;

		// Average of the surrounding edge MID points
		MPoint averageEdgePoint(0.0, 0.0, 0.0);
		for (unsigned int i = 0; i < numConnectedEdges; ++i) {
			int2 vertices;
			inputMeshFn.getEdgeVertices(connectedEdges[i], vertices);
			MPoint f1, f2;
			inputMeshFn.getPoint(vertices[0], f1);
			inputMeshFn.getPoint(vertices[1], f2);
			averageEdgePoint += (f1 + f2) / 2;
		}
		averageEdgePoint *= 2.0 / numConnectedEdges;

		MPoint controlPoint = vertexIt.position(MSpace::kObject, &status) * (valence - 3); CHECK_MSTATUS(status);

		// Calculate new point position
		MPoint newControlVertex = averageFacePoint + averageEdgePoint + controlPoint;
		newControlVertex *= (1.0 / valence);

		cout << "New control point: " << vertexIt.index() << " " << newControlVertex.x << " " << newControlVertex.y << " " << newControlVertex.z << endl;

		newControlVertexPoints.append(newControlVertex);
	}

	// Create the new polygon
	unsigned int originalPolygonNum = inputMeshFn.numPolygons();
	for (unsigned int polygonIndex = 0; polygonIndex < originalPolygonNum; ++polygonIndex) {
		for (int v = 0, previousV = polygonsVertices[polygonIndex].length() - 1; v < polygonsVertices[polygonIndex].length(); ++v, ++previousV) {
			MPointArray newPolygonPoints;
			MPoint vertex0, vertex1, vertex2, vertex3;
			vertex0 = newControlVertexPoints[polygonsVertices[polygonIndex][v]];
			vertex1 = edgePoints[polygonsEdges[polygonIndex][v]];
			vertex2 = facePoints[polygonIndex];
			vertex3 = edgePoints[polygonsEdges[polygonIndex][previousV % polygonsVertices[polygonIndex].length()]];
			newPolygonPoints.append(vertex0); newPolygonPoints.append(vertex1); newPolygonPoints.append(vertex2); newPolygonPoints.append(vertex3);
			outputMeshFn.addPolygon(newPolygonPoints, true, 1.0e-10, MObject::kNullObj, &status); CHECK_MSTATUS(status);
		}
		// Delete old face
		status = outputMeshFn.deleteFace(0);
		CHECK_MSTATUS(status);
	}

	delete[] polygonsVertices;
	delete[] polygonsEdges;
}
