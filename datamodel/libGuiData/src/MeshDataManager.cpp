#include "MeshData/MeshDataManager.h"

namespace raco::guiData {
MeshData::MeshData() {

}

void MeshData::setMeshName(std::string name) {
    meshName_ = name;
}

std::string MeshData::getMeshName() {
    return meshName_;
}

void MeshData::setMeshUri(std::string uri) {
    meshUri_ = uri;
}

std::string MeshData::getMeshUri() {
    return meshUri_;
}

void MeshData::setNumTriangles(int num) {
    numTriangles_ = num;
}

int MeshData::getNumTriangles() {
    return numTriangles_;
}

void MeshData::setNumVertices(int num) {
    numVertices_ = num;
}

int MeshData::getNumVertices() {
    return numVertices_;
}

int MeshData::getAttributeSize() {
    return attributes_.size();
}

void MeshData::addAttribute(Attribute attr) {
    attributes_.push_back(attr);
}

std::vector<Attribute> MeshData::getAttributes() {
    return attributes_;
}

void MeshData::setIndices(std::vector<uint32_t> indices) {
    indexBuffer_ = indices;
}

std::vector<uint32_t> MeshData::getIndices() {
    return indexBuffer_;
}

void MeshData::setModelMatrix(QMatrix4x4 matrix) {
    modelMatrix_ = matrix;
}

QMatrix4x4 MeshData::getModelMatrix() {
    return modelMatrix_;
}

MeshDataManager &MeshDataManager::GetInstance() {
    static MeshDataManager Instance;
    return Instance;
}

MeshDataManager::MeshDataManager() {

}

void MeshDataManager::clearMesh() {
    meshDataMap_.clear();
}

bool MeshDataManager::hasMeshData(std::string id) {
    auto iter = meshDataMap_.find(id);
    if (iter != meshDataMap_.end()) {
        return true;
    }
    return false;
}

void MeshDataManager::addMeshData(std::string id, MeshData mesh) {
    meshDataMap_.emplace(id, mesh);
}

bool MeshDataManager::getMeshData(std::string id, MeshData& meshdata) {
	auto iter = meshDataMap_.find(id);
	if (iter != meshDataMap_.end()) {
		meshdata = iter->second;
		return true;
    }
    return false;
}

void MeshDataManager::setMeshModelMatrix(std::string id, QMatrix4x4 matrix) {
    auto iter = meshDataMap_.find(id);
    if (iter != meshDataMap_.end()) {
        iter->second.setModelMatrix(matrix);
    }
}

std::map<std::string, MeshData> MeshDataManager::getMeshDataMap() {
    return meshDataMap_;
}

int MeshDataManager::attriIndex(std::vector<Attribute> attrs, std::string aName) {
    for (int i{0}; i < attrs.size(); i++) {
        auto attrIt = attrs.at(i);
        if (attrIt.name.compare(aName) == 0) {
            return i;
        }
    }
    return -1;
}
}
