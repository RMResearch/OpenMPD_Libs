#include "ReflectionSurface.h"

void ReflectionSurface::addMesh(glm::vec4 A, glm::vec4 B, glm::vec4 C) {
	glm::vec4 AB = B - A;
	glm::vec4 AC = C - A;
	float area = 0.5f * sqrt(glm::length2(AB)*glm::length2(AC) - glm::dot(AB, AC) * glm::dot(AB, AC));
	if (!isnan(area)) {
		vecCentres.push_back((A + B + C) / 3.f);
		vecNormals.push_back(glm::vec4(glm::normalize(glm::cross(glm::vec3(AB.x, AB.y, AB.z), glm::vec3(AC.x, AC.y, AC.z))), 0));
		vecAreas.push_back(area);
	}
}
void ReflectionSurface::addScanMesh(glm::vec4 A, glm::vec4 B, glm::vec4 C) {
	glm::vec4 AB = B - A;
	glm::vec4 AC = C - A;
	float area = 0.5f * sqrt(glm::length2(AB)*glm::length2(AC) - glm::dot(AB, AC) * glm::dot(AB, AC));
	if (!isnan(area)) {
		// meshes used for both computing scattering and ray-tracing
		vecCentres.push_back((A + B + C) / 3.f);
		vecNormals.push_back(glm::vec4(glm::normalize(glm::cross(glm::vec3(AB.x, AB.y, AB.z), glm::vec3(AC.x, AC.y, AC.z))), 0));
		vecAreas.push_back(area);
		// meshes used only for ray-tracing
		vecAs.push_back(A);
		vecABs.push_back(AB);
		vecACs.push_back(AC);
	}
}

void ReflectionSurface::clearReflector() {
	vecCentres.clear();
	vecNormals.clear();
	vecAreas.clear();
}
void ReflectionSurface::addFlatReflector(int numCols, int numRows, float cellPitch, float height) {
	float area = cellPitch * cellPitch / 2.f;
	for (int y = 0; y < numRows; y++) {
		for (int x = 0; x < numCols; x++) {
			glm::vec4 position(1.f);
			position.x = cellPitch * (x - (numCols - 1.0f) / 2.0f);
			position.y = cellPitch * (y - (numRows - 1.0f) / 2.0f);
			position.z = 0.0;

			glm::vec4 A = position + glm::vec4(-cellPitch / 2.f, -cellPitch / 2.f, 0, 0);
			glm::vec4 B = position + glm::vec4(+cellPitch / 2.f, -cellPitch / 2.f, 0, 0);
			glm::vec4 C = position + glm::vec4(+cellPitch / 2.f, +cellPitch / 2.f, 0, 0);
			glm::vec4 D = position + glm::vec4(-cellPitch / 2.f, +cellPitch / 2.f, 0, 0);

			addMesh(A, B, C);
			addMesh(A, C, D);
		}
	}
}
void ReflectionSurface::addTestReflector(int numMeshes) {
	float pitch = 0.001f;
	for (int i = 0; i < numMeshes; i++) {
		glm::vec4 A(-pitch / 2.f, -pitch / 2.f, 0, 0);
		glm::vec4 B(+pitch / 2.f, -pitch / 2.f, 0, 0);
		glm::vec4 C(+pitch / 2.f, +pitch / 2.f, 0, 0);
		addMesh(A, B, C);
	}
}

void ReflectionSurface::addSTLObject(std::string fileName, glm::mat4 transformation) {
	std::ifstream ifs(fileName);
	if (ifs.is_open()) {
		int numVertices = 0;
		std::string fileLine;
		glm::vec4 A, B, C;
		while (getline(ifs, fileLine)) {
			std::stringstream toParse(fileLine);
			std::vector<std::string> lineTokens;
			std::string token;
			while (std::getline(toParse >> std::ws, token, ' ')) {
				lineTokens.push_back(token);
			}
			if (!lineTokens.empty()) {
				if (lineTokens[0] == "facet") {
					numVertices = 0;
				}
				else if (lineTokens[0] == "vertex") {
					static glm::vec4 vertex(0, 0, 0, 1);
					vertex.x = atof(lineTokens[1].c_str());
					vertex.y = atof(lineTokens[2].c_str());
					vertex.z = atof(lineTokens[3].c_str());
					vertex = transformation * vertex;
					if (numVertices == 0) A = vertex;
					if (numVertices == 1) B = vertex;
					if (numVertices == 2) C = vertex;
					numVertices++;
				}
				else if (lineTokens[0] == "endfacet") {
					if(glm::length(glm::cross(glm::vec3(B - A), glm::vec3(C - A))) > 0)
						addScanMesh(A, B, C);
				}
			}
		}
	}
	else {
		std::cout << "ERROR: File " << fileName << "cannot be opened!" << std::endl;
	}
}
void ReflectionSurface::addOBJObject(std::string fileName, glm::mat4 transformation) {
	std::vector<glm::vec4> vertices;

	std::ifstream ifs(fileName);
	if (ifs.is_open()) {
		std::string fileLine;
		while (getline(ifs, fileLine)) {
			std::stringstream toParse(fileLine);
			std::vector<std::string> lineTokens;
			std::string token;
			while (std::getline(toParse >> std::ws, token, ' ')) {
				lineTokens.push_back(token);
			}
			if (!lineTokens.empty()) {
				if (lineTokens[0] == "v") {
					static glm::vec4 vertex(0, 0, 0, 1);
					vertex.x = atof(lineTokens[1].c_str());
					vertex.y = atof(lineTokens[2].c_str());
					vertex.z = atof(lineTokens[3].c_str());
					vertex = transformation * vertex;
					vertices.push_back(vertex);
				}
				else if (lineTokens[0] == "f") {
					std::vector<glm::vec4> triangle;
					for (int i = 0; i < 3; i++) {
						std::stringstream split(lineTokens[i + 1]);
						std::vector<std::string> numberTokens;
						std::string numberToken;
						while (std::getline(split >> std::ws, numberToken, '/')) {
							numberTokens.push_back(numberToken);
						}
						int vertexIndex = atoi(numberTokens[0].c_str());
						triangle.push_back(vertices[vertexIndex - 1]);
					}
					addScanMesh(triangle[0], triangle[1], triangle[2]);
				}
			}
		}
	}
	else {
		std::cout << "ERROR: File " << fileName << "cannot be opened!" << std::endl;
	}
}


float* ReflectionSurface::getCentres() {
	int numMeshes = vecCentres.size();
	float *centres = new float[numMeshes * 4];
	int meshInd = 0;
	for (int m = 0; m < numMeshes; m++) {
		centres[meshInd++] = vecCentres[m].x;
		centres[meshInd++] = vecCentres[m].y;
		centres[meshInd++] = vecCentres[m].z;
		centres[meshInd++] = 1;
	}
	return centres;
}
float* ReflectionSurface::getNormals() {
	int numMeshes = vecNormals.size();
	float *normals = new float[numMeshes * 4];
	int meshInd = 0;
	for (int m = 0; m < numMeshes; m++) {
		normals[meshInd++] = vecNormals[m].x;
		normals[meshInd++] = vecNormals[m].y;
		normals[meshInd++] = vecNormals[m].z;
		normals[meshInd++] = 1;
	}
	return normals;
}
float* ReflectionSurface::getAreas() {
	int numMeshes = vecAreas.size();
	float *areas = new float[numMeshes];
	for (int m = 0; m < numMeshes; m++) {
		areas[m] = vecAreas[m];
	}
	return areas;
}
float* ReflectionSurface::getAs() {
	int numMeshes = vecAs.size();
	float *As = new float[numMeshes * 4];
	int meshInd = 0;
	for (int m = 0; m < numMeshes; m++) {
		As[meshInd++] = vecAs[m].x;
		As[meshInd++] = vecAs[m].y;
		As[meshInd++] = vecAs[m].z;
		As[meshInd++] = 1;
	}
	return As;
}
float* ReflectionSurface::getABs() {
	int numMeshes = vecABs.size();
	float *ABs = new float[numMeshes * 4];
	int meshInd = 0;
	for (int m = 0; m < numMeshes; m++) {
		ABs[meshInd++] = vecABs[m].x;
		ABs[meshInd++] = vecABs[m].y;
		ABs[meshInd++] = vecABs[m].z;
		ABs[meshInd++] = 1;
	}
	return ABs;
}
float* ReflectionSurface::getACs() {
	int numMeshes = vecACs.size();
	float *ACs = new float[numMeshes * 4];
	int meshInd = 0;
	for (int m = 0; m < numMeshes; m++) {
		ACs[meshInd++] = vecACs[m].x;
		ACs[meshInd++] = vecACs[m].y;
		ACs[meshInd++] = vecACs[m].z;
		ACs[meshInd++] = 1;
	}
	return ACs;
}

void ReflectionSurface::saveAsCSV(std::string fileName) {
	std::ofstream f(fileName);
	f << "index," << "x," << "y," << "z," << "area," << std::endl;
	for (int m = 0; m < vecCentres.size(); m++)
		f << m << "," << vecCentres[m].x << "," << vecCentres[m].y << "," << vecCentres[m].z << "," << vecAreas[m] << "," << std::endl;
	f.close();

}

float* ReflectionSurface::importTransducer2MeshMatrix(std::string fileName, int numTransducers, int numMeshes) {
	float* transducer2MeshMatrix = new float[2 * numMeshes * numTransducers];
	FILE* f;
	fopen_s(&f, fileName.c_str(), "rb");
	if (f == NULL) printf("File cannot open\n");
	else {
		size_t numMem;
		numMem = fread(transducer2MeshMatrix, sizeof(float), 2 * numMeshes * numTransducers, f);
		fclose(f);
	}
	return transducer2MeshMatrix;
}