#include "include/collada.h"

#include <queue>
#include <functional>
#include <sstream>
#include <vector>

namespace
{
	using namespace collada;
	
	void FindChildNodes(const ColladaNode* rootNode, std::function<bool(const ColladaNode*)> predicate, std::list<const ColladaNode*>& nodesFound)
	{
		std::queue<const ColladaNode*> nodesToCheck;
		nodesToCheck.push(rootNode);

		while (!nodesToCheck.empty()) {
			const ColladaNode* cur = nodesToCheck.front();
			nodesToCheck.pop();

			if (predicate(cur)) {
				nodesFound.push_back(cur);
			}

			for (std::list<ColladaNode*>::const_iterator it = cur->m_children.begin();
				it != cur->m_children.end(); ++it) {
				nodesToCheck.push(*it);
			}
		}
	}

	const ColladaNode* FindChildTagByName(const std::string& name, const ColladaNode* rootNode)
	{
		std::list<const ColladaNode*> found;
		FindChildNodes(rootNode, [name](const ColladaNode* node) {
			return node->m_tagName == name;
		},
		found);

		if (found.size() == 0) {
			return nullptr;
		}

		return *found.begin();
	}

	void FindChildTagsByName(
		const std::string& name,
		const ColladaNode* rootNode,
		std::list<const ColladaNode*>& found)
	{
		FindChildNodes(rootNode, [name](const ColladaNode* node) {
			return node->m_tagName == name;
		},
		found);
	}

	const ColladaNode* FindChildTagByID(
		const std::string& id, const ColladaNode* rootNode)
	{
		std::list<const ColladaNode*> found;
		FindChildNodes(rootNode, [id](const ColladaNode* node) {
			std::map<std::string, std::string>::const_iterator it = node->m_tagProps.find("id");
			if (it == node->m_tagProps.end()) {
				return false;
			}
			return it->second == id;
		},
		found);

		if (found.size() == 0) {
			return nullptr;
		}

		return *found.begin();
	}

	bool ReadMaterialTriangles(const ColladaNode* triangles, const ColladaNode* geometry, Object& object);

	bool ReadObjectFromNode(const ColladaNode* node, const ColladaNode* rootDataNode, Object& object)
	{
		const ColladaNode* instanceGeometry = FindChildTagByName("instance_geometry", node);
		if (!instanceGeometry) {
			return false;
		}

		std::string geometryURL;
		{
			std::map<std::string, std::string>::const_iterator urlProp =
				instanceGeometry->m_tagProps.find("url");

			if (urlProp == instanceGeometry->m_tagProps.end()) {
				return false;
			}

			geometryURL = urlProp->second.substr(1);
		}

		const ColladaNode* geometry = FindChildTagByID(geometryURL, rootDataNode);
		if (!geometry) {
			return false;
		}

		std::list<const ColladaNode*> trianglesTags;
		FindChildTagsByName("triangles", geometry, trianglesTags);

		int firstFreeIndex = 0;
		for (std::list<const ColladaNode*>::const_iterator it = trianglesTags.begin();
			it != trianglesTags.end(); ++it) {
			const ColladaNode* trianglesTag = *it;

			const std::string& materialName = trianglesTag->m_tagProps.find("material")->second;

			if (!ReadMaterialTriangles(trianglesTag, geometry, object)) {
				return false;
			}
			
			MaterialIndexRange mir;
			mir.m_name = materialName;
			mir.indexOffset = firstFreeIndex;
			mir.indexCount = object.m_indices.size() - firstFreeIndex;
			object.m_materials.push_back(mir);

			firstFreeIndex = object.m_indices.size();
		}

		return true;
	}

	struct Vector3
	{
		float m_values[3];
	};

	struct Vector2
	{
		float m_values[2];
	};

	bool ReadVectors3D(const ColladaNode* verts, std::vector<Vector3>& vectors)
	{
		const ColladaNode* arr = FindChildTagByName("float_array", verts);
		const ColladaNode* acc = FindChildTagByName("accessor", verts);

		std::list<const ColladaNode*> params;
		FindChildTagsByName("param", acc, params);

		int offsets[3] = {-1, -1, -1};

		int index = 0;
		for (std::list<const ColladaNode*>::const_iterator it = params.begin();
			it != params.end(); ++it) {
			
			const ColladaNode* cur = *it;
			const std::string& name = cur->m_tagProps.find("name")->second;
			if (name == "X") {
				offsets[0] = index;
			}
			if (name == "Y") {
				offsets[1] = index;
			}
			if (name == "Z") {
				offsets[2] = index;
			}
			++index;
		}

		for (int i = 0; i < 3; ++i) {
			if (offsets[i] < 0) {
				return false;
			}
		}

		const std::string strideStr = acc->m_tagProps.find("stride")->second;

		std::stringstream ss(strideStr);
		int stride;
		ss >> stride;

		std::vector<float> data;
		for (std::list<scripting::ISymbol*>::const_iterator it = arr->m_data.begin();
			it != arr->m_data.end(); ++it) {
			data.push_back((*it)->m_symbolData.m_number);
		}

		for (int i = 0; i < data.size(); i += stride) {
			vectors.push_back(Vector3{ 
				data[i + offsets[0]],
				data[i + offsets[1]],
				data[i + offsets[2]] 
			});
		}

		return true;
	}

	bool ReadUVs(const ColladaNode* uvs, std::vector<Vector2>& vectors)
	{
		const ColladaNode* arr = FindChildTagByName("float_array", uvs);
		const ColladaNode* acc = FindChildTagByName("accessor", uvs);

		std::list<const ColladaNode*> params;
		FindChildTagsByName("param", acc, params);

		int offsets[2] = { -1, -1 };

		int index = 0;
		for (std::list<const ColladaNode*>::const_iterator it = params.begin();
			it != params.end(); ++it) {

			const ColladaNode* cur = *it;
			const std::string& name = cur->m_tagProps.find("name")->second;
			if (name == "S") {
				offsets[0] = index;
			}
			if (name == "T") {
				offsets[1] = index;
			}
			++index;
		}

		for (int i = 0; i < 2; ++i) {
			if (offsets[i] < 0) {
				return false;
			}
		}

		const std::string strideStr = acc->m_tagProps.find("stride")->second;

		std::stringstream ss(strideStr);
		int stride;
		ss >> stride;

		std::vector<float> data;
		for (std::list<scripting::ISymbol*>::const_iterator it = arr->m_data.begin();
			it != arr->m_data.end(); ++it) {
			data.push_back((*it)->m_symbolData.m_number);
		}

		for (int i = 0; i < data.size(); i += stride) {
			vectors.push_back(Vector2{
				data[i + offsets[0]],
				data[i + offsets[1]]
				});
		}

		return true;
	}

	int FindVertexIndex(const Vertex& vertex, const Object& object)
	{
		int index = 0;
		for (std::list<Vertex>::const_iterator it = object.m_vertices.begin();
			it != object.m_vertices.end(); ++it) {
			if (vertex.Equals(*it)) {
				return index;
			}
			++index;
		}

		return -1;
	}

	bool ReadMaterialTriangles(const ColladaNode* triangles, const ColladaNode* geometry, Object& object)
	{
		std::list<const ColladaNode*> inputs;
		FindChildTagsByName("input", triangles, inputs);

		int vertexOffset = -1;
		int normalOffset = -1;
		int uvOffset = -1;

		std::vector<Vector3> vertices;
		std::vector<Vector3> normals;
		std::vector<Vector2> uvs;

		for (std::list<const ColladaNode*>::const_iterator it = inputs.begin();
			it != inputs.end(); ++it) {
			const ColladaNode* cur = *it;

			const std::string& semantic = cur->m_tagProps.find("semantic")->second;
			const std::string& source = cur->m_tagProps.find("source")->second;
			const std::string& offsetStr = cur->m_tagProps.find("offset")->second;
			
			std::stringstream ss(offsetStr);

			if (semantic == "VERTEX") {
				ss >> vertexOffset;

				const ColladaNode* vert = FindChildTagByID(source.substr(1), geometry);
				if (!vert || vert->m_tagName != "vertices") {
					return false;
				}

				std::list<const ColladaNode*> tmp;
				FindChildNodes(vert, [](const ColladaNode* node) {
					if (node->m_tagName != "input") {
						return false;
					}
					std::map<std::string, std::string>::const_iterator semanticProp = node->m_tagProps.find("semantic");
					if (semanticProp == node->m_tagProps.end()) {
						return false;
					}
					if (semanticProp->second != "POSITION") {
						return false;
					}

					return true;
				}, tmp);
				
				if (tmp.size() == 0) {
					return false;
				}
				const ColladaNode* input = *tmp.begin();

				std::map<std::string, std::string>::const_iterator sourceUrlIt = input->m_tagProps.find("source");
				const std::string& sourceUrl = sourceUrlIt->second;

				const ColladaNode* vertsSource = FindChildTagByID(sourceUrl.substr(1), geometry);

				if (!ReadVectors3D(vertsSource, vertices)) {
					return false;
				}
			}

			if (semantic == "NORMAL") {
				ss >> normalOffset;

				const ColladaNode* norm = FindChildTagByID(source.substr(1), geometry);
				if (!norm || norm->m_tagName != "source") {
					return false;
				}

				if (!ReadVectors3D(norm, normals)) {
					return false;
				}
			}

			if (semantic == "TEXCOORD") {
				ss >> uvOffset;

				const ColladaNode* uvNode = FindChildTagByID(source.substr(1), geometry);
				if (!uvNode || uvNode->m_tagName != "source") {
					return false;
				}

				if (!ReadUVs(uvNode, uvs)) {
					return false;
				}
			}
		}

		if (vertexOffset < 0 || normalOffset < 0 || uvOffset < 0) {
			return false;
		}

		const ColladaNode* pTag = FindChildTagByName("p", triangles);

		std::vector<int> indices;
		for (std::list<scripting::ISymbol*>::const_iterator it = pTag->m_data.begin();
			it != pTag->m_data.end(); ++it) {
			indices.push_back(static_cast<int>((*it)->m_symbolData.m_number));
		}
		
		int stride = inputs.size();

		Vertex triangle[3];
		int vertRead = 0;

		for (int i = 0; i < indices.size(); i += stride) {
			Vertex& cur = triangle[vertRead];
			int vertIndex = indices[i + vertexOffset];
			int normalIndex = indices[i + normalOffset];
			int uvIndex = indices[i + uvOffset];

			cur.m_position[0] = vertices[vertIndex].m_values[0];
			cur.m_position[1] = vertices[vertIndex].m_values[1];
			cur.m_position[2] = vertices[vertIndex].m_values[2];

			cur.m_normal[0] = normals[normalIndex].m_values[0];
			cur.m_normal[1] = normals[normalIndex].m_values[1];
			cur.m_normal[2] = normals[normalIndex].m_values[2];

			cur.m_uv[0] = uvs[uvIndex].m_values[0];
			cur.m_uv[1] = uvs[uvIndex].m_values[1];

			++vertRead;

			if (vertRead < 3) {
				continue;
			}

			vertRead = 0;

			for (int j = 0; j < 3; ++j) {
				int vertIndex = FindVertexIndex(triangle[j], object);
				if (vertIndex < 0) {
					vertIndex = object.m_vertices.size();
					object.m_vertices.push_back(triangle[j]);
				}
				object.m_indices.push_back(vertIndex);
			}
		}

		return true;
	}

}

bool collada::ConvertToScene(const std::list<collada::ColladaNode*>& nodes, collada::Scene& scene)
{
	const ColladaNode* sceneTag = nullptr;
	const ColladaNode* dataContainerTag = nullptr;

	for (std::list<ColladaNode*>::const_iterator it = nodes.begin(); it != nodes.end(); ++it) {
		sceneTag = FindChildTagByName("scene", *it);
		if (sceneTag) {
			dataContainerTag = *it;
			break;
		}
	}

	if (!sceneTag) {
		return false;
	}

	const ColladaNode* instanceVisualScene = FindChildTagByName("instance_visual_scene", sceneTag);

	if (!instanceVisualScene) {
		return false;
	}

	std::string visualSceneURL;
	{
		std::map<std::string, std::string>::const_iterator urlIt = instanceVisualScene->m_tagProps.find("url");
		if (urlIt == instanceVisualScene->m_tagProps.end()) {
			return false;
		}

		visualSceneURL = urlIt->second.substr(1);
	}

	const ColladaNode* visualScene = FindChildTagByID(visualSceneURL, dataContainerTag);
	if (!visualScene) {
		return false;
	}

	std::list<const ColladaNode*> objectNodes;
	FindChildTagsByName("node", visualScene, objectNodes);

	for (std::list<const ColladaNode*>::const_iterator it = objectNodes.begin();
		it != objectNodes.end(); ++it) {
		scene.m_objects.push_back(Object());
		Object& obj = scene.m_objects.back();
		if (!ReadObjectFromNode(*it, dataContainerTag, obj)) {
			return false;
		}
	}

	return true;
}

bool collada::Vertex::Equals(const Vertex& other) const
{
	static const float EPS = 0.000001;
	for (int i = 0; i < 3; ++i) {
		if (abs(m_position[i] - other.m_position[i]) >= EPS) {
			return false;
		}
	}

	for (int i = 0; i < 3; ++i) {
		if (abs(m_normal[i] - other.m_normal[i]) >= EPS) {
			return false;
		}
	}

	for (int i = 0; i < 2; ++i) {
		if (abs(m_uv[i] - other.m_uv[i]) >= EPS) {
			return false;
		}
	}

	return true;
}