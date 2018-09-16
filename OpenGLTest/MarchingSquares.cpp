#include "MarchingSquares.h"

using std::vector;
using glm::vec3;

const static float _squares[16][18] = {
	{ -1.0f },
	{ 0.0f, 0.5f, 0.5f, 0.0f, 0.0f, 0.0f, -1.0f },
	{ 0.5f, 0.0f, 1.0f, 0.5f, 1.0f, 0.0f, -1.0f },
	{ 0.0f, 0.5f, 1.0f, 0.5f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.5f, 1.0f, 0.0f, -1.0f },
	{ 0.5f, 1.0f, 1.0f, 1.0f, 1.0f, 0.5f, -1.0f },
	{ 0.0f, 0.5f, 0.5f, 0.0f, 0.0f, 0.0f, 0.5f, 1.0f, 1.0f, 1.0f, 1.0f, 0.5f, -1.0f },
	{ 0.5f, 1.0f, 1.0f, 1.0f, 0.5f, 0.0f, 0.5f, 0.0f, 1.0f, 1.0f, 1.0f, 0.0f, -1.0f },
	{ 0.0f, 0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.5f, 0.5f, 1.0f, 1.0f, 0.0f, 0.5f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f },
	{ 0.0f, 1.0f, 0.5f, 1.0f, 0.0f, 0.5f, -1.0f },
	{ 0.0f, 1.0f, 0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.5f, 1.0f, 0.5f, 0.0f, -1.0f },
	{ 0.5f, 0.0f, 1.0f, 0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 0.5f, 1.0f, 0.0f, 0.5f, -1.0f },
	{ 0.0f, 1.0f, 0.5f, 1.0f, 0.0f, 0.0f, 0.5f, 1.0f, 1.0f, 0.5f, 0.0f, 0.0f, 1.0f, 0.5f, 1.0f, 0.0f, 0.0f, 0.0f },
	{ 0.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.5f, 0.0f, 0.5f, 1.0f, 1.0f, 1.0f, 0.5f, -1.0f },
	{ 0.0f, 1.0f, 0.5f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.5f },
	{ 0.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.5f, 1.0f, 1.0f, 0.5f, 0.0f, 0.0f, 0.5f, 1.0f, 1.0f, 1.0f, 0.0f, 0.5f, 0.0f },
	{ 0.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 0.0f, -1.0f }
};

vector<vec3> MarchingSquares::GenerateMesh(Map m) {
	vector<vec3> vertexVector;
	for (int x = 0; x < m.width() - 1; x++) {
		for (int y = 0; y < m.height() - 1; y++) {
			int index = 0;
			if (m.isSolid(x, y)) { index += 1; }
			if (m.isSolid(x + 1, y)) { index += 2; }
			if (m.isSolid(x + 1, y + 1)) { index += 4; }
			if (m.isSolid(x, y + 1)) { index += 8; }
			for (int i = 0; i < 18; i += 2) {
				if (_squares[index][i] == -1.0f) { break; }
				vec3 vertex = vec3(
					_squares[index][i] + x,
					_squares[index][i + 1] + y,
					0.0f
				);
				vertexVector.push_back(vertex);
			}
		}
	}
	return vertexVector;
}