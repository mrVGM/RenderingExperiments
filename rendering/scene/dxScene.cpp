#include "scene/dxScene.h"

#include "nativeFunc.h"
#include "utils.h"
#include "dxBuffer.h"
#include "api.h"
#include "utils.h"

#include "d3dx12.h"

#include <list>

#define THROW_ERROR(hRes, error) \
if (FAILED(hRes)) {\
    errorMessage = error;\
    return false;\
}

void rendering::scene::DXScene::InitProperties(interpreter::NativeObject & nativeObject)
{
	using namespace interpreter;

#define THROW_EXCEPTION(error)\
scope.SetProperty("exception", Value(error));\
return Value();

	Value& p_instanceBuffers = GetOrCreateProperty(nativeObject, "instanceBuffers");
	Value& p_colladaBuffersCache = GetOrCreateProperty(nativeObject, "colladaBuffersCache");

	Value& addObject = GetOrCreateProperty(nativeObject, "addObject");
	addObject = CreateNativeMethod(nativeObject, 2, [](Value scope) {
		Value selfValue = scope.GetProperty("self");
		DXScene* self = static_cast<DXScene*>(NativeObject::ExtractNativeObject(selfValue));

		Value objectNameValue = scope.GetProperty("param0");
		if (objectNameValue.GetType() != ScriptingValueType::String) {
			THROW_EXCEPTION("Please supply object name!")
		}

		
		Value meshNameValue = scope.GetProperty("param1");
		if (meshNameValue.GetType() != ScriptingValueType::String) {
			THROW_EXCEPTION("Please supply mesh name!")
		}

		Object3D obj;
		obj.m_mesh = meshNameValue.GetString();
		self->m_objects.insert(std::pair<std::string, Object3D>(objectNameValue.GetString(), obj));

		return Value();
	});

	Value& addObjectMaterial = GetOrCreateProperty(nativeObject, "addObjectMaterial");
	addObjectMaterial = CreateNativeMethod(nativeObject, 2, [](Value scope) {
		Value selfValue = scope.GetProperty("self");
		DXScene* self = static_cast<DXScene*>(NativeObject::ExtractNativeObject(selfValue));

		Value objectNameValue = scope.GetProperty("param0");
		if (objectNameValue.GetType() != ScriptingValueType::String) {
			THROW_EXCEPTION("Please supply object name!")
		}

		std::map<std::string, Object3D>::iterator it = self->m_objects.find(objectNameValue.GetString());
		if (it == self->m_objects.end()) {
			THROW_EXCEPTION("Can't find object!")
		}

		Object3D& obj = it->second;

		Value materialNameValue = scope.GetProperty("param1");
		if (materialNameValue.GetType() != ScriptingValueType::String) {
			THROW_EXCEPTION("Please supply material name!")
		}

		obj.m_materials.push_back(materialNameValue.GetString());
		return Value();
	});

	Value& setObjectPosition = GetOrCreateProperty(nativeObject, "setObjectPosition");
	setObjectPosition = CreateNativeMethod(nativeObject, 2, [](Value scope) {
		Value selfValue = scope.GetProperty("self");
		DXScene* self = static_cast<DXScene*>(NativeObject::ExtractNativeObject(selfValue));

		Value objectNameValue = scope.GetProperty("param0");
		if (objectNameValue.GetType() != ScriptingValueType::String) {
			THROW_EXCEPTION("Please supply object name!")
		}

		std::map<std::string, Object3D>::iterator it = self->m_objects.find(objectNameValue.GetString());
		if (it == self->m_objects.end()) {
			THROW_EXCEPTION("Can't find object!")
		}

		Object3D& obj = it->second;

		Value positionValue = scope.GetProperty("param1");
		std::list<Value> positionValueList;

		positionValue.ToList(positionValueList);
		if (positionValueList.size() != 3) {
			THROW_EXCEPTION("Please supply position vector!")
		}

		std::list<Value>::iterator pit = positionValueList.begin();
		float x = (*pit).GetNum();
		++pit;
		float y = (*pit).GetNum();
		++pit;
		float z = (*pit).GetNum();

		obj.m_transform.m_position[0] = x;
		obj.m_transform.m_position[1] = y;
		obj.m_transform.m_position[2] = z;

		return Value();
	});

	Value& setObjectRotation = GetOrCreateProperty(nativeObject, "setObjectRotation");
	setObjectRotation = CreateNativeMethod(nativeObject, 2, [](Value scope) {
		Value selfValue = scope.GetProperty("self");
		DXScene* self = static_cast<DXScene*>(NativeObject::ExtractNativeObject(selfValue));

		Value objectNameValue = scope.GetProperty("param0");
		if (objectNameValue.GetType() != ScriptingValueType::String) {
			THROW_EXCEPTION("Please supply object name!")
		}

		std::map<std::string, Object3D>::iterator it = self->m_objects.find(objectNameValue.GetString());
		if (it == self->m_objects.end()) {
			THROW_EXCEPTION("Can't find object!")
		}

		Object3D& obj = it->second;

		Value rotationValue = scope.GetProperty("param1");
		std::list<Value> rotationValueList;

		rotationValue.ToList(rotationValueList);
		if (rotationValueList.size() != 4) {
			THROW_EXCEPTION("Please supply rotation quaternion!")
		}

		std::list<Value>::iterator rit = rotationValueList.begin();
		float x = (*rit).GetNum();
		++rit;
		float y = (*rit).GetNum();
		++rit;
		float z = (*rit).GetNum();
		++rit;
		float w = (*rit).GetNum();

		obj.m_transform.m_rotation[0] = x;
		obj.m_transform.m_rotation[1] = y;
		obj.m_transform.m_rotation[2] = z;
		obj.m_transform.m_rotation[3] = w;
		return Value();
	});

	Value& setObjectScale = GetOrCreateProperty(nativeObject, "setObjectScale");
	setObjectScale = CreateNativeMethod(nativeObject, 2, [](Value scope) {
		Value selfValue = scope.GetProperty("self");
		DXScene* self = static_cast<DXScene*>(NativeObject::ExtractNativeObject(selfValue));

		Value objectNameValue = scope.GetProperty("param0");
		if (objectNameValue.GetType() != ScriptingValueType::String) {
			THROW_EXCEPTION("Please supply object name!")
		}

		std::map<std::string, Object3D>::iterator it = self->m_objects.find(objectNameValue.GetString());
		if (it == self->m_objects.end()) {
			THROW_EXCEPTION("Can't find object!")
		}

		Object3D& obj = it->second;

		Value scaleValue = scope.GetProperty("param1");
		std::list<Value> scaleValueList;

		scaleValue.ToList(scaleValueList);
		if (scaleValueList.size() != 3) {
			THROW_EXCEPTION("Please supply scale vector!")
		}

		std::list<Value>::iterator sit = scaleValueList.begin();
		float x = (*sit).GetNum();
		++sit;
		float y = (*sit).GetNum();
		++sit;
		float z = (*sit).GetNum();

		obj.m_transform.m_scale[0] = x;
		obj.m_transform.m_scale[1] = y;
		obj.m_transform.m_scale[2] = z;
		
		return Value();
	});

	Value& getInstanceBuffersData = GetOrCreateProperty(nativeObject, "getInstanceBuffersData");
	getInstanceBuffersData = CreateNativeMethod(nativeObject, 0, [](Value scope) {
		Value selfValue = scope.GetProperty("self");
		DXScene* self = static_cast<DXScene*>(NativeObject::ExtractNativeObject(selfValue));
		
		self->ConstructInstanceBuffersData();

		std::list<Value> tmp;
		for (std::map<int, InstanceBuffer>::iterator it = self->m_instanceBuffers.begin(); it != self->m_instanceBuffers.end(); ++it) {
			Value v = interpreter::utils::GetEmptyObject();
			IManagedValue* managedValue = v.GetManagedValue();
			managedValue->SetProperty("id", Value(it->first));
			managedValue->SetProperty("count", Value(it->second.m_objectsCount));

			tmp.push_back(v);
		}

		return Value::FromList(tmp);
	});

	Value& setInstanceBuffers = GetOrCreateProperty(nativeObject, "setInstanceBuffers");
	setInstanceBuffers = CreateNativeMethod(nativeObject, 1, [&](Value scope) {
		Value selfValue = scope.GetProperty("self");
		DXScene* self = static_cast<DXScene*>(NativeObject::ExtractNativeObject(selfValue));

		Value instanceBuffersValue = scope.GetProperty("param0");

		p_instanceBuffers = instanceBuffersValue;

		std::list<Value> instanceBuffers;
		instanceBuffersValue.ToList(instanceBuffers);

		for (std::list<Value>::iterator it = instanceBuffers.begin(); it != instanceBuffers.end(); ++it) {
			Value cur = *it;
			int id = static_cast<int>(cur.GetManagedValue()->GetProperty("id").GetNum());
			Value buffValue = cur.GetManagedValue()->GetProperty("buff");

			DXBuffer* buff = static_cast<DXBuffer*>(NativeObject::ExtractNativeObject(buffValue));

			self->m_instanceBuffers[id].m_buffer = buff->GetBuffer();
			self->m_instanceBuffers[id].m_size = buff->GetBufferWidth();
			self->m_instanceBuffers[id].m_stride = buff->GetStride();
		}

		return Value();
	});

	Value& readColladaScene = GetOrCreateProperty(nativeObject, "readColladaScene");
	readColladaScene = CreateNativeMethod(nativeObject, 1, [&](Value scope) {
		Value selfValue = scope.GetProperty("self");
		DXScene* self = static_cast<DXScene*>(NativeObject::ExtractNativeObject(selfValue));

		Value colladaFileValue = scope.GetProperty("param0");
		if (colladaFileValue.GetType() != ScriptingValueType::String) {
			THROW_EXCEPTION("Please supply Collada File!")
		}
		std::string error;
		bool res = self->ReadColladaScene(colladaFileValue.GetString(), error);

		if (!res) {
			THROW_EXCEPTION(error)
		}

		for (std::map<std::string, collada::Geometry>::const_iterator it = self->m_colladaScene.m_geometries.begin();
			it != self->m_colladaScene.m_geometries.end(); ++it) {
			self->m_colladaGeometryBuffers.insert(
				std::pair<std::string, ColladaGeometryBuffers>(it->first, ColladaGeometryBuffers()));
		}

		for (std::map<std::string, collada::InstanceBuffer>::const_iterator it = self->m_colladaScene.m_instanceBuffers.begin();
			it != self->m_colladaScene.m_instanceBuffers.end(); ++it) {
			self->m_colladaInstanceBuffers.insert(
				std::pair<std::string, ID3D12Resource*>(it->first, nullptr));
		}

		return Value();
	});

	Value& getColladaBufferInfo = GetOrCreateProperty(nativeObject, "getColladaBufferInfo");
	getColladaBufferInfo = CreateNativeMethod(nativeObject, 0, [](Value scope) {
		Value selfValue = scope.GetProperty("self");
		DXScene* self = static_cast<DXScene*>(NativeObject::ExtractNativeObject(selfValue));

		Value res = interpreter::utils::GetEmptyObject();
		std::list<Value> geometries;
		std::list<Value> instanceBuffers;

		for (std::map<std::string, collada::Geometry>::const_iterator it = self->m_colladaScene.m_geometries.begin();
			it != self->m_colladaScene.m_geometries.end(); ++it) {
			Value geo = interpreter::utils::GetEmptyObject();

			geo.SetProperty("name", Value(it->first));
			geo.SetProperty("indexSize", Value(it->second.m_indices.size() * sizeof(int)));
			geo.SetProperty("vertexSize", Value(it->second.m_vertices.size() * sizeof(collada::Vertex)));
			geo.SetProperty("vertexStride", Value(sizeof(collada::Vertex)));

			geometries.push_back(geo);
		}

		for (std::map<std::string, collada::InstanceBuffer>::const_iterator it = self->m_colladaScene.m_instanceBuffers.begin();
			it != self->m_colladaScene.m_instanceBuffers.end(); ++it) {
			Value ib = interpreter::utils::GetEmptyObject();
			
			ib.SetProperty("name", Value(it->first));
			ib.SetProperty("size", Value(it->second.m_data.size() * sizeof(collada::GeometryInstanceData)));
			ib.SetProperty("stride", Value(sizeof(collada::GeometryInstanceData)));

			instanceBuffers.push_back(ib);
		}

		res.SetProperty("geometries", Value::FromList(geometries));
		res.SetProperty("instanceBuffers", Value::FromList(instanceBuffers));

		return res;
	});

	Value& setColladaGeometryVertexBuffer = GetOrCreateProperty(nativeObject, "setColladaGeometryVertexBuffer");
	setColladaGeometryVertexBuffer = CreateNativeMethod(nativeObject, 2, [](Value scope) {
		Value selfValue = scope.GetProperty("self");
		DXScene* self = static_cast<DXScene*>(NativeObject::ExtractNativeObject(selfValue));

		Value nameValue = scope.GetProperty("param0");
		if (nameValue.GetType() != ScriptingValueType::String) {
			THROW_EXCEPTION("Please supply geometry name!")
		}

		Value bufferValue = scope.GetProperty("param1");
		DXBuffer* buffer = dynamic_cast<DXBuffer*>(NativeObject::ExtractNativeObject(bufferValue));

		if (!buffer) {
			THROW_EXCEPTION("Please supply vertex buffer!")
		}

		std::map<std::string, ColladaGeometryBuffers>::iterator it =
			self->m_colladaGeometryBuffers.find(nameValue.GetString());

		if (it == self->m_colladaGeometryBuffers.end()) {
			THROW_EXCEPTION("Invalid Geometry Name!")
		}
		ColladaGeometryBuffers& cgb = it->second;

		cgb.m_vertexBuffer = buffer->GetBuffer();

		return Value();
	});

	Value& setColladaGeometryIndexBuffer = GetOrCreateProperty(nativeObject, "setColladaGeometryIndexBuffer");
	setColladaGeometryIndexBuffer = CreateNativeMethod(nativeObject, 2, [](Value scope) {
		Value selfValue = scope.GetProperty("self");
		DXScene* self = static_cast<DXScene*>(NativeObject::ExtractNativeObject(selfValue));

		Value nameValue = scope.GetProperty("param0");
		if (nameValue.GetType() != ScriptingValueType::String) {
			THROW_EXCEPTION("Please supply geometry name!")
		}

		Value bufferValue = scope.GetProperty("param1");
		DXBuffer* buffer = dynamic_cast<DXBuffer*>(NativeObject::ExtractNativeObject(bufferValue));

		if (!buffer) {
			THROW_EXCEPTION("Please supply index buffer!")
		}

		std::map<std::string, ColladaGeometryBuffers>::iterator it =
			self->m_colladaGeometryBuffers.find(nameValue.GetString());

		if (it == self->m_colladaGeometryBuffers.end()) {
			THROW_EXCEPTION("Invalid Geometry Name!")
		}
		ColladaGeometryBuffers& cgb = it->second;

		cgb.m_indexBuffer = buffer->GetBuffer();

		return Value();
	});

	Value& setColladaInstanceBuffer = GetOrCreateProperty(nativeObject, "setColladaInstanceBuffer");
	setColladaInstanceBuffer = CreateNativeMethod(nativeObject, 2, [](Value scope) {
		Value selfValue = scope.GetProperty("self");
		DXScene* self = static_cast<DXScene*>(NativeObject::ExtractNativeObject(selfValue));

		Value nameValue = scope.GetProperty("param0");
		if (nameValue.GetType() != ScriptingValueType::String) {
			THROW_EXCEPTION("Please supply geometry name!")
		}

		Value bufferValue = scope.GetProperty("param1");
		DXBuffer* buffer = dynamic_cast<DXBuffer*>(NativeObject::ExtractNativeObject(bufferValue));

		if (!buffer) {
			THROW_EXCEPTION("Please supply instance buffer!")
		}

		std::map<std::string, ID3D12Resource*>::iterator it =
			self->m_colladaInstanceBuffers.find(nameValue.GetString());

		if (it == self->m_colladaInstanceBuffers.end()) {
			THROW_EXCEPTION("Invalid Geometry Name!")
		}

		self->m_colladaInstanceBuffers[nameValue.GetString()] = buffer->GetBuffer();
		return Value();
	});

	Value& setColladaBuffersCache = GetOrCreateProperty(nativeObject, "setColladaBuffersCache");
	setColladaBuffersCache = CreateNativeMethod(nativeObject, 1, [&](Value scope) {
		Value selfValue = scope.GetProperty("self");
		DXScene* self = static_cast<DXScene*>(NativeObject::ExtractNativeObject(selfValue));

		p_colladaBuffersCache = scope.GetProperty("param0");

		return Value();
	});

	Value& updateColladaBuffers = GetOrCreateProperty(nativeObject, "updateColladaBuffers");
	updateColladaBuffers = CreateNativeMethod(nativeObject, 0, [](Value scope) {
		Value selfValue = scope.GetProperty("self");
		DXScene* self = static_cast<DXScene*>(NativeObject::ExtractNativeObject(selfValue));

		std::string error;
		bool res = self->UpdateColladaBuffers(error);
		if (!res) {
			THROW_EXCEPTION("Can't update collada buffers!")
		}
		return Value();
	});

#undef THROW_EXCEPTION
}

void rendering::scene::DXScene::ConstructInstanceBuffersData()
{
	m_instanceBuffers.clear();

	std::list<Object3D*> objectList;

	for (std::map<std::string, Object3D>::iterator it = m_objects.begin(); it != m_objects.end(); ++it) {
		objectList.push_back(&it->second);

		it->second.m_instanceBufferID = -1;
		it->second.m_instanceBufferOffset = -1;
	}

	for (std::list<Object3D*>::iterator testIt = objectList.begin(); testIt != objectList.end(); ++testIt) {
		Object3D* tested = *testIt;

		for (std::list<Object3D*>::iterator processedIt = objectList.begin(); processedIt != testIt; ++processedIt) {
			Object3D* processed = *processedIt;

			if (tested->Similar(*processed)) {
				tested->m_instanceBufferID = processed->m_instanceBufferID;

				tested->m_instanceBufferOffset = m_instanceBuffers[tested->m_instanceBufferID].m_objectsCount;
				++m_instanceBuffers[tested->m_instanceBufferID].m_objectsCount;
				break;
			}
		}

		if (tested->m_instanceBufferID < 0) {
			int id = m_instanceBufferCount++;

			InstanceBuffer instanceBuff;
			instanceBuff.m_objectsCount = 1;

			m_instanceBuffers.insert(std::pair<int, InstanceBuffer>(id, instanceBuff));
			tested->m_instanceBufferID = id;
			tested->m_instanceBufferOffset = 0;
		}
	}
}

bool rendering::scene::DXScene::ReadColladaScene(const std::string& colladaFile, std::string& errorMessage)
{
	struct ColladaNodesContainer
	{
		std::list<collada::ColladaNode*> m_nodes;
		~ColladaNodesContainer()
		{
			for (std::list<collada::ColladaNode*>::iterator it = m_nodes.begin();
				it != m_nodes.end(); ++it) {
				delete* it;
			}
			m_nodes.clear();
		}
	};

	interpreter::Value api = GetAPI();
	interpreter::Value appContext = api.GetProperty("app_context");
	std::string filePath = appContext.GetProperty("root_dir").GetString() +  colladaFile;

	collada::IColladaReader* reader = collada::GetReader();

	scripting::ISymbol* symbol = reader->ReadColladaFile(filePath);
	if (!symbol) {
		errorMessage = "Can't parse Collada File!";
		return false;
	}

	std::list<collada::ColladaNode*> nodes;
	ColladaNodesContainer container;
	bool res = reader->ConstructColladaTree(symbol, nodes, container.m_nodes);

	if (!res) {
		errorMessage = "Can't construct Collada Tree!";
		return false;
	}

	res = collada::ConvertToScene(nodes, m_colladaScene);
	if (!res) {
		errorMessage = "Can't convert to Collada Scene!";
		return false;
	}

	return true;
}

bool rendering::scene::DXScene::UpdateColladaBuffers(std::string& errorMessage)
{
	for (std::map<std::string, collada::Geometry>::const_iterator it = m_colladaScene.m_geometries.begin();
		it != m_colladaScene.m_geometries.end(); ++it) {

		const collada::Geometry& curGeo = it->second;

		CD3DX12_RANGE readRange(0, 0);

		void* dst = nullptr;
		{
			ID3D12Resource* curBuff = m_colladaGeometryBuffers[it->first].m_vertexBuffer;
			THROW_ERROR(
				curBuff->Map(0, &readRange, &dst),
				"Can't map Vertex Buffer!")

				collada::Vertex* vertexArr = static_cast<collada::Vertex*>(dst);

			for (std::list<collada::Vertex>::const_iterator vertIt = curGeo.m_vertices.begin();
				vertIt != curGeo.m_vertices.end(); ++vertIt) {
				*vertexArr = *vertIt;
				++vertexArr;
			}

			curBuff->Unmap(0, nullptr);
		}

		{
			ID3D12Resource* curBuff = m_colladaGeometryBuffers[it->first].m_indexBuffer;
			THROW_ERROR(
				curBuff->Map(0, &readRange, &dst),
				"Can't map Vertex Buffer!")

			int* indexArr = static_cast<int*>(dst);

			for (std::list<int>::const_iterator indIt = curGeo.m_indices.begin();
				indIt != curGeo.m_indices.end(); ++indIt) {
				*indexArr = *indIt;
				++indexArr;
			}

			curBuff->Unmap(0, nullptr);
		}

	}
	return true;
}

rendering::scene::DXScene::~DXScene()
{
	collada::ReleaseColladaReader();
}


#undef THROW_ERROR

bool rendering::scene::Object3D::Similar(const Object3D& other) const
{
	if (m_mesh != other.m_mesh) {
		return false;
	}

	if (m_materials.size() != other.m_materials.size()) {
		return false;
	}

	std::list<std::string>::const_iterator matIt1 = m_materials.begin();
	std::list<std::string>::const_iterator matIt2 = other.m_materials.begin();

	while (matIt1 != m_materials.end()) {
		if (*matIt1 != *matIt2) {
			return false;
		}

		++matIt1;
		++matIt2;
	}
	return true;
}
