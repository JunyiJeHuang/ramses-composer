#ifndef PROGRAMMANAGER_H
#define PROGRAMMANAGER_H

#include <QObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QJsonValue>
#include <QString>
#include <QDebug>
#include <QFile>
#include <QDateTime>
#include <QDir>
#include <QThread>
#include <QMap>
#include <mutex>
#include "PropertyData/PropertyData.h"
#include "NodeData/nodeManager.h"
#include "PropertyData/PropertyType.h"
#include "AnimationData/animationData.h"
#include "CurveData/CurveManager.h"
#include "MaterialData/materialManager.h"
#include "signal/SignalProxy.h"
#include "data_Convert/OutputAssets.h"
#include "data_Convert/AssetsLogic.h"
#include "MeshData/MeshDataManager.h"
#include "openctm.h"
#include "openctmpp.h"

using namespace raco::guiData;
namespace raco::dataConvert {

enum EDATAYPE {
    ETYPE_FLOAT = 1,
    ETYPE_VEC2,
    ETYPE_VEC3,
    ETYPE_VEC4
};

class ProgramManager : public QObject {
    Q_OBJECT
public:
	bool writeProgram(QString filePath);
	bool writeBMWAssets(QString filePath);
	bool readBMWAssets(QString filePath);
	bool writeProgram2Json(QString filePath);
    void setRelativePath(QString path);
	void setOpenedProjectPath(QString path);
	bool writeCTMFile(std::string filePathStr);
    bool readProgramFromJson(QString filePath);
	bool updateUIFromJson(QString filePath);

Q_SIGNALS:
    void selectObject(const QString &objectId);
	void createNode(NodeData* node, const std::vector<MaterialData>& materials);

private:
    QString file_;
    QString relativePath_;
    QString openedProjectPath_;
    QMap<QString, QJsonArray> aryMap_;
    OutputPtx outputPtx_;
	OutputPtw outputPtw_;

    AssetsLogic assetsLogic_;
};
}

#endif // PROGRAMMANAGER_H