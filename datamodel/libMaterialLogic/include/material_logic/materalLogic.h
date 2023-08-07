#ifndef MATERALLOGIC_H
#define MATERALLOGIC_H

#include <QObject>
#include "core/CommandInterface.h"
#include "MaterialData/materialManager.h"
#include <map>

using namespace raco::guiData;
namespace raco::material_logic {

class MateralLogic : public QObject
{
    Q_OBJECT
public:
    explicit MateralLogic(QObject *parent = nullptr);
	void setMaterialResourcesHandleReMap(std::map<std::string, core::ValueHandle> map) {
		materialResourcesHandleReMap_ = map;
	}

	void setTextureResourcesHandleReMap(std::map<std::string, core::ValueHandle> map) {
		textureResourcesHandleReMap_ = map;
	}

    void setCubeMapResourcesHandleReMap(std::map<std::string, core::ValueHandle> map) {
        cubeMapResourcesHandleReMap_ = map;
    }
	
	std::map<std::string, core::ValueHandle>& getResourcesHandleReMap() {
		return materialResourcesHandleReMap_;
	}
	void setPtxName(Shader &shader);
    void analyzing();
    void analyzingMaterialData();
	void initNodeMaterialProperty(core::ValueHandle valueHandle, NodeMaterial &nodeMaterial);
    void analyzingNodeMaterial();
	void setOneTexture(core::ValueHandle valueHandle, TextureData &textureData);

    void analyzingTexture();
    void initMaterialProperty(core::ValueHandle valueHandle, MaterialData &materialData, Shader &shader);
    void setOptionsProperty(core::ValueHandle valueHandle, MaterialData &materialData);
    void setUniformsProperty(core::ValueHandle valueHandle, MaterialData &materialData);
    void setTexturePorperty(core::ValueHandle valueHandle, MaterialData &materialData, TextureData &textureData);
    void setUniformsMultiElementProperty(core::ValueHandle valueHandle, MaterialData &materialData, UniformType type);

    void analyzingCubeMap();
    bool setCubeMapData(core::ValueHandle valueHandle, CubeMapData &cubemap);

private:
	std::map<std::string, core::ValueHandle> materialResourcesHandleReMap_;
	std::map<std::string, core::ValueHandle> textureResourcesHandleReMap_;
    std::map<std::string, core::ValueHandle> cubeMapResourcesHandleReMap_;
};

}

#endif // MATERALLOGIC_H
