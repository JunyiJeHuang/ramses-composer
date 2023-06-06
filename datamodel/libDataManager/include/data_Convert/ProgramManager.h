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
#include <QMessageBox>
#include "PropertyData/PropertyData.h"
#include "NodeData/nodeManager.h"
#include "PropertyData/PropertyType.h"
#include "AnimationData/animationData.h"
#include "CurveData/CurveManager.h"
#include "MaterialData/materialManager.h"
#include "signal/SignalProxy.h"
#include "MeshData/MeshDataManager.h"
#include "openctm.h"
#include "openctmpp.h"

//- #include "assets/OutputAssets.h"
//- #include "assets/AssetsLogic.h"

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
	bool writeBMWAssets(QString filePath);
	bool writeProgram2Json(QString filePath);
    bool readProgramFromJson(QString filePath);
    bool updateUIFromJson(QString filePath);

    void setRelativePath(QString path);
	void setOpenedProjectPath(QString path);
    void initFolderData();

Q_SIGNALS:
    void selectObject(const QString &objectId);
	void createNode(NodeData* node, const std::vector<MaterialData>& materials);

private:
    QString file_;
    QString relativePath_;
    QString openedProjectPath_;
    QMap<QString, QJsonArray> aryMap_;

//-   assets::OutputPtx outputPtx_;
//-   assets::OutputPtw outputPtw_;
//-   assets::OutputCTM outputCTM_;
};
}

#endif // PROGRAMMANAGER_H
