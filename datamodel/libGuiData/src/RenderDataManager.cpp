#include "RenderData/RenderDataManager.h"

namespace raco::guiData {
RenderDataManager &RenderDataManager::GetInstance() {
    static RenderDataManager Instance;
    return Instance;
}

RenderDataManager::RenderDataManager() {

}

}
