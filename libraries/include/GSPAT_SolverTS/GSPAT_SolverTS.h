#pragma once
#include <GSPAT_Solver.h>

namespace GSPAT_TS {
	/**
		Configuration parameters for BEM Solver: 
		- NUM_ITERATIONS: Integer, defining how many iterations the algorithm should use. 
		- TRANSDUCER_DIFFERENCE_THRESHOLD:
		- PONT_DIFFERENCE_THRESHOLD:
		- REFLECTOR_NUM_ELEMENTS: Integer, defining how many elements (patches) our reflector has.
		- REFLECTOR_ELEMENT_POSITIONS: Array of vertices (vec3) defining the location of each element.  
		- REFLECTOR_ELEMENT_NORMALS: Array of normals (one vec3 per element).
		- REFLECTOR_ELEMENT_AREAS: Array of floats, describing the area of each element.
		- TRANSDUCER_TO_REFLECTOR_MATRIX: Propagation matrix from each transducer (T) to each element in the mesh (M). Complex matrix of size T x M 
		- REFLECTOR_FILE: File (.bin) containing the definition of all reflector related parameters.  
	*/
	enum ConfigurationParameters {
		NUM_ITERATIONS=0,
		TRANSDUCER_DIFFERENCE_THRESHOLD, 
		PONT_DIFFERENCE_THRESHOLD,
		REFLECTOR_NUM_ELEMENTS,
		REFLECTOR_ELEMENT_POSITIONS,
		REFLECTOR_ELEMENT_NORMALS,
		REFLECTOR_ELEMENT_AREAS,
		TRANSDUCER_TO_REFLECTOR_MATRIX, 
		REFLECTOR_FILE
	};
	/**
		Create a Solver. Width and Height specify the number of transducers, but currently only 32x16 transducers is supported.

	*/
	_GSPAT_Export GSPAT::Solver* createSolver(int numTransducers);

	void printMessage_GSPAT(const char*);
	void printError_GSPAT(const char*);
	void printWarning_GSPAT(const char*);

	_GSPAT_Export void RegisterPrintFuncs(void(*p_Message)(const char*), void(*p_Warning)(const char*), void(*p_Error)(const char*));

	_GSPAT_Export void resetInitialPhases(GSPAT::Solver* solver);

	_GSPAT_Export float* simulateFieldFromHologram(GSPAT::Solver* solver, float* hologram, int numFieldPoints, float* fieldPositions);

	_GSPAT_Export void computeMatrixAandB(GSPAT::Solver* solver, int numMeshes, float* meshPositions, float* meshAreas, float* meshNormals, float*& matrixA, float*& matrixB);
};