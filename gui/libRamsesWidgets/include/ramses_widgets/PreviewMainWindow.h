/*
 * SPDX-License-Identifier: MPL-2.0
 *
 * This file is part of Ramses Composer
 * (see https://github.com/bmwcarit/ramses-composer).
 *
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
 * If a copy of the MPL was not distributed with this file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "components/DataChangeDispatcher.h"
#include "components/DebugInstanceCounter.h"
#include "ramses_widgets/RendererBackend.h"
#include "core/CommandInterface.h"
#include "user_types/PerspectiveCamera.h"
#include "ramses_base/RamsesHandles.h"
#include "style/Icons.h"
#include <QLabel>
#include <QMainWindow>
#include <QToolButton>
#include <memory>
#include <QEvent>
#include <QVector3D>
#include <QVector4D>
#include <QMatrix4x4>
#include <QtMath>
#include <QtAlgorithms>
#include <QMap>
#include <QCursor>

namespace raco::user_types {

class BaseCamera;
using SBaseCamera = std::shared_ptr<BaseCamera>;
}


namespace Ui {
class PreviewMainWindow;
}

namespace raco::ramses_adaptor {
class SceneBackend;
}

namespace raco::ramses_widgets {

enum KEY_BOARD_TYPE {
    KEY_BOARD_NONE,
    KEY_BOARD_TRANSLATION,
    KEY_BOARD_ROTATION,
    KEY_BOARD_SCALING
};

enum MODEL_MOVE_DIRECT {
    MODEL_MOVE_DEFAULT,
    MODEL_MOVE_X,
    MODEL_MOVE_Y,
    MODEL_MOVE_Z
};

class PreviewContentWidget;
class PreviewScrollAreaWidget;

class PreviewMainWindow final : public QMainWindow {
	DEBUG_INSTANCE_COUNTER(PreviewMainWindow);

public:
	PreviewMainWindow(RendererBackend& rendererBackend,
		raco::ramses_adaptor::SceneBackend* sceneBackend,
		const QSize& sceneSize,
		raco::core::Project* project,
		raco::components::SDataChangeDispatcher dispatcher,
		raco::core::CommandInterface* commandInterface,
		QWidget* parent = nullptr);
	~PreviewMainWindow();
	void displayScene(ramses::sceneId_t sceneId, core::Vec4f const& backgroundColor);
	void setAxesIconLabel(QLabel * axesIcon);
    void setAxesIcon(const bool& z_up);
	void sceneScaleUpdate(bool zup, float scaleValue, bool scaleUp);

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;

public Q_SLOTS:
	void setViewport(const QSize& sceneSize);
	void commit(bool forceUpdate);
	void setAxes(const bool& z_up);
	void updateAxesIconLabel();
	void setEnableDisplayGrid(bool enable);
	void sceneUpdate(bool z_up);
private:
    QMatrix4x4 getViewMatrix(QVector3D position, float rX, float rY, float rZ);
    QMatrix4x4 getViewMatrix2(QVector3D posistion, float rX, float rY, float rZ);
    QVector<float> checkTriCollision(QVector3D ray, QVector3D camera, QVector<QVector<QVector3D>> triangles);
    float checkSingleTriCollision(QVector3D ray, QVector3D camera, QVector<QVector3D> triangle);
    QMap<std::string, QVector<QVector<QVector3D> >> getMeshTriangles();
    std::string caculateRayIntersection(QVector3D ray, QVector3D cameraPos);
    void mouseMove(QPoint position);
    void translationMovement(QPoint position);
    void rotationMovement(QPoint position);
    void scalingMovement(QPoint position);
    QVector3D calculateModelTranslation(QPoint position);

private:
	std::unique_ptr<Ui::PreviewMainWindow> ui_;
	PreviewContentWidget* previewWidget_;
	PreviewScrollAreaWidget* scrollAreaWidget_;
	QLabel* sceneIdLabel_;
	raco::core::Project* project_;
	raco::core::CommandInterface* commandInterface_;
	raco::ramses_adaptor::SceneBackend* sceneBackend_;
	bool zUp_;
	QLabel* axesIcon_;
	QSize sceneSize_;
	bool updateAxesIconLabel_;
	float scaleValue_;
	int mode_;
	bool haveInited_;
    QLabel *upLabel_{nullptr};
    QLabel *modelLabel_{nullptr};
    std::string selModelID_;
    KEY_BOARD_TYPE keyBoardType_ = KEY_BOARD_NONE;
    MODEL_MOVE_DIRECT modelMoveDirect_ = MODEL_MOVE_DEFAULT;
    QPoint selModelPos_;
    QVector3D selModelTranslation_;
};

}  // namespace raco::ramses_widgets
