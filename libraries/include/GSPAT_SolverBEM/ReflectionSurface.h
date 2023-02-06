#pragma once
#include <iostream>
#include <sstream>
#include <fstream>
#include <vector>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/norm.hpp>

class ReflectionSurface {

public:
	// meshes used for both computing scattering and ray-tracing
	std::vector<glm::vec4> vecCentres;
	std::vector<glm::vec4> vecNormals;
	std::vector<float> vecAreas;
	// meshes used only for ray-tracing
	std::vector<glm::vec4> vecAs;
	std::vector<glm::vec4> vecABs;
	std::vector<glm::vec4> vecACs;

	void clearReflector();
	void addFlatReflector(int numCols, int numRows, float cellPitch, float height = 0.f);
	void addTestReflector(int numMeshes);
	void addSTLObject(std::string fileName, glm::mat4 transformation = glm::mat4(1.f));
	void addOBJObject(std::string fileName, glm::mat4 transformation = glm::mat4(1.f));

	int getNumMeshes() { return vecCentres.size(); }
	int getScanNumMeshes() { return vecAs.size(); }

	float* getCentres();
	float* getNormals();
	float* getAreas();
	float* getAs();
	float* getABs();
	float* getACs();

	void saveAsCSV(std::string fileName);

	static float* importTransducer2MeshMatrix(std::string fileName, int numTransducers, int numMeshes);
protected:
	void addMesh(glm::vec4 A, glm::vec4 B, glm::vec4 C);
	void addScanMesh(glm::vec4 A, glm::vec4 B, glm::vec4 C);
};
