/*
 * SPDX-License-Identifier: MPL-2.0
 *
 * This file is part of Ramses Composer
 * (see https://github.com/bmwcarit/ramses-composer).
 *
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
 * If a copy of the MPL was not distributed with this file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include "ramses_widgets/PreviewMainWindow.h"

#include "components/RaCoPreferences.h"
#include "ramses_adaptor/SceneAdaptor.h"
#include "ramses_adaptor/SceneBackend.h"
#include "ramses_widgets/PreviewContentWidget.h"
#include "ramses_widgets/PreviewScrollAreaWidget.h"
#include <ramses-renderer-api/RamsesRenderer.h>
#include "user_types/BaseCamera.h"
#include "user_types/MeshNode.h"
#include "user_types/Node.h"
#include "user_types/OrthographicCamera.h"
#include "user_types/PerspectiveCamera.h"
#include "user_types/RenderPass.h"
#include "user_types/Material.h"
#include "ramses_adaptor/NodeAdaptor.h"
#include "core/Queries.h"
#include "signal/SignalProxy.h"
#include "PropertyData/PropertyType.h"
#include "ramses_widgets/SceneStateEventHandler.h"
#include "style/Icons.h"

#include "ui_PreviewMainWindow.h"
#include <QLabel>
#include <QMenu>
#include <QMessageBox>
#include <QPushButton>
#include <QToolButton>
#include <windows.h>
#include <QMatrix4x4>
#include <log_system/log.h>
#include <QHBoxLayout>

namespace raco::ramses_widgets {

using namespace raco::core;

float saasin(float fac) {
    if (fac <= -1.0f) {
    return (float)-M_PI_2;
    }
    else if (fac >= 1.0f) {
    return (float)M_PI_2;
    }
    else {
    return asinf(fac);
    }
}

float len_v2v2(const float v1[2], const float v2[2]) {
    float x, y;

    x = v1[0] - v2[0];
    y = v1[1] - v2[1];
    return sqrtf(x * x + y * y);
}

float normalize_v2(float n[2]) {
    float dot = n[0] * n[0] + n[1] * n[1];
    if (dot > 1.0e-35f) {
        dot = sqrtf(dot);
    } else {
        dot = 0.0f;
    }
    return dot;
}

float angle_normalize_v2v2(float a[2], float b[2]) {
    float dot = a[0] * b[0] + a[1] * b[1];

    if (dot >= 0.0f) {
        return 2.0f * saasin(len_v2v2(a, b) / 2.0f);
    }

    float v2_n[2];
    v2_n[0] = -b[0];
    v2_n[1] = -b[1];
    return (float)M_PI - 2.0f * saasin(len_v2v2(a, v2_n) / 2.0f);
}

QMatrix4x4 rotationEuler(float x, float y, float z) {
    const float rotX = qDegreesToRadians(x);
    const float rotY = qDegreesToRadians(y);
    const float rotZ = qDegreesToRadians(z);

    // Save some sin and cos values for reuse in the computations
    const float sx = std::sin(rotX);
    const float cx = std::cos(rotX);
    const float sy = std::sin(rotY);
    const float cy = std::cos(rotY);
    const float sz = std::sin(rotZ);
    const float cz = std::cos(rotZ);

    return QMatrix4x4(cz * cy                 , cz * sy * sx - sz * cx    , sz * sx + cz * sy * cx  ,   0.0f,
                      sz * cy                 , cz * cx + sz * sy * sx    , sz * sy * cx - cz * sx  ,   0.0f,
                      -sy                     , cy * sx                   , cy * cx                 ,   0.0f,
                      0.0f                    , 0.0f                      , 0.0f                    ,   1.0f);
}

std::array<double, 3> XYZEul_FromHMatrix(float M[][4]) {
    std::array<double, 3> ea;
    int i = 0, j = 1, k = 2;
    double cy = sqrtf(M[i][i] * M[i][i] + M[j][i] * M[j][i]);
    if (cy > 16 * FLT_EPSILON) {
        ea[0] = atan2f(M[k][j], M[k][k]);
        ea[1] = atan2f(-M[k][i], cy);
        ea[2] = atan2f(M[j][i], M[i][i]);
    } else {
        ea[0] = atan2f(-M[j][k], M[j][j]);
        ea[1] = atan2f(-M[k][i], cy);
        ea[2] = 0;
    }

    ea[0] = ea[0] * 180.0 / M_PI;
    ea[1] = ea[1] * 180.0 / M_PI;
    ea[2] = ea[2] * 180.0 / M_PI;

    return ea;
}

PreviewMainWindow::PreviewMainWindow(RendererBackend& rendererBackend, raco::ramses_adaptor::SceneBackend* sceneBackend, const QSize& sceneSize, raco::core::Project* project,
    raco::components::SDataChangeDispatcher dispatcher, raco::core::CommandInterface* commandInterface, QWidget* parent)
	: QMainWindow{parent},
	  ui_{new Ui::PreviewMainWindow()},
      commandInterface_(commandInterface),
      sceneBackend_(sceneBackend),
      project_(project) {
    ui_->setupUi(this);

    sceneIdLabel_ = new QLabel{"scene id: -", ui_->statusbar};
    auto* pixelLabel = new QLabel{"x: - y: -", ui_->statusbar};
    auto* scaleLabel = new QLabel{"scale: 1.0", ui_->statusbar};
    ui_->statusbar->addWidget(sceneIdLabel_);
    ui_->statusbar->addPermanentWidget(pixelLabel);
    ui_->statusbar->addPermanentWidget(scaleLabel);

	haveInited_ = false;
	zUp_ = project_->settings()->axes_.asBool();
	updateAxesIconLabel_ = true;
	scaleValue_ = 1.0f;
	// scroll and zoom logic widget
	scrollAreaWidget_ = new PreviewScrollAreaWidget{sceneSize, this};
    connect(scrollAreaWidget_, &PreviewScrollAreaWidget::scaleChanged, [this, scaleLabel](double scale, bool addvalue) {
		QString content{};
		content.append("scale: ");
		content.append(std::to_string(scale).c_str());
		scaleLabel->setText(content);
        //this->sceneScaleUpdate(this->zUp_, (float)scale, addvalue);
	});
	setCentralWidget(scrollAreaWidget_);

	// Actual preview surface
	previewWidget_ = new PreviewContentWidget{rendererBackend, sceneBackend, scrollAreaWidget_->viewport()};
	connect(scrollAreaWidget_, &PreviewScrollAreaWidget::viewportRectChanged, previewWidget_, &PreviewContentWidget::setViewportRect);
	connect(previewWidget_, &PreviewContentWidget::updateAxesIconLabel, this, &PreviewMainWindow::updateAxesIconLabel);
	connect(previewWidget_, &PreviewContentWidget::newMousePosition, [this, pixelLabel](const QPoint globalPosition) {
        if (auto previewPosition = scrollAreaWidget_->globalPositionToPreviewPosition(globalPosition)) {
            mouseMove(QPoint(previewPosition->x(), previewPosition->y()));
			QString content{};
			content.append("x: ");
			content.append(std::to_string(previewPosition->x()).c_str());
			content.append(" y: ");
			content.append(std::to_string(previewPosition->y()).c_str());
			pixelLabel->setText(content);
		} else {
			pixelLabel->setText("x: - y: -");
		}
	});

	// Size mode tool button
	{
		auto* sizeMenu = new QMenu{ui_->toolBar};
		sizeMenu->addAction(ui_->actionSetSizeModeOff);
		sizeMenu->addAction(ui_->actionSetSizeModeVerticalFit);
		sizeMenu->addAction(ui_->actionSetSizeModeHorizontalFit);
		sizeMenu->addAction(ui_->actionSetSizeModeBestFit);
		sizeMenu->addAction(ui_->actionSetSizeModeOriginalFit);
		ui_->actionSetSizeModeOff->setCheckable(true);
		ui_->actionSetSizeModeVerticalFit->setCheckable(true);
		ui_->actionSetSizeModeHorizontalFit->setCheckable(true);
		ui_->actionSetSizeModeBestFit->setCheckable(true);
		ui_->actionSetSizeModeOriginalFit->setCheckable(true);
		connect(ui_->actionSetSizeModeOff, &QAction::triggered, scrollAreaWidget_, &PreviewScrollAreaWidget::autoSizingOff);
		connect(ui_->actionSetSizeModeVerticalFit, &QAction::triggered, scrollAreaWidget_, &PreviewScrollAreaWidget::autoSizingVerticalFit);
		connect(ui_->actionSetSizeModeHorizontalFit, &QAction::triggered, scrollAreaWidget_, &PreviewScrollAreaWidget::autoSizingHorizontalFit);
		connect(ui_->actionSetSizeModeBestFit, &QAction::triggered, scrollAreaWidget_, &PreviewScrollAreaWidget::autoSizingBestFit);
		connect(ui_->actionSetSizeModeOriginalFit, &QAction::triggered, scrollAreaWidget_, &PreviewScrollAreaWidget::autoSizingOriginalFit);
		auto* sizeMenuButton = new QToolButton{ui_->toolBar};
		sizeMenuButton->setMenu(sizeMenu);
		sizeMenuButton->setPopupMode(QToolButton::InstantPopup);
		connect(scrollAreaWidget_, &PreviewScrollAreaWidget::autoSizingChanged, [this, sizeMenuButton](PreviewScrollAreaWidget::AutoSizing mode) {
			auto action = ui_->actionSetSizeModeOff;
			switch (mode) {
				case PreviewScrollAreaWidget::AutoSizing::VERTICAL_FIT:
					action = ui_->actionSetSizeModeVerticalFit;
					break;
				case PreviewScrollAreaWidget::AutoSizing::HORIZONTAL_FIT:
					action = ui_->actionSetSizeModeHorizontalFit;
					break;
				case PreviewScrollAreaWidget::AutoSizing::BEST_FIT:
					action = ui_->actionSetSizeModeBestFit;
					break;
				case PreviewScrollAreaWidget::AutoSizing::ORIGINAL_FIT:
					action = ui_->actionSetSizeModeOriginalFit;
					break;
			};
			sizeMenuButton->setText(action->text());
			ui_->actionSetSizeModeOff->setChecked(false);
			ui_->actionSetSizeModeVerticalFit->setChecked(false);
			ui_->actionSetSizeModeHorizontalFit->setChecked(false);
			ui_->actionSetSizeModeBestFit->setChecked(false);
			ui_->actionSetSizeModeOriginalFit->setChecked(false);
			action->setChecked(true);
		});
		ui_->toolBar->insertWidget(ui_->actionSelectSizeMode, sizeMenuButton);
	}
	// MSAA button
	{
		auto* msaaMenu = new QMenu{ui_->toolBar};
		msaaMenu->addAction(ui_->actionSetMSAAx0);
		msaaMenu->addAction(ui_->actionSetMSAAx2);
		msaaMenu->addAction(ui_->actionSetMSAAx4);
		msaaMenu->addAction(ui_->actionSetMSAAx8);
		ui_->actionSetMSAAx0->setCheckable(true);
		ui_->actionSetMSAAx0->setChecked(true);
		ui_->actionSetMSAAx2->setCheckable(true);
		ui_->actionSetMSAAx4->setCheckable(true);
		ui_->actionSetMSAAx8->setCheckable(true);

		auto* msaaMenuButton = new QToolButton{ui_->toolBar};
		msaaMenuButton->setMenu(msaaMenu);
		msaaMenuButton->setPopupMode(QToolButton::InstantPopup);

		msaaMenuButton->setText(ui_->actionSetMSAAx0->text());

		auto updateMsaaSelection = [this, msaaMenuButton](QAction* action) {
			msaaMenuButton->setText(action->text());
			ui_->actionSetMSAAx0->setChecked(ui_->actionSetMSAAx0 == action);
			ui_->actionSetMSAAx2->setChecked(ui_->actionSetMSAAx2 == action);
			ui_->actionSetMSAAx4->setChecked(ui_->actionSetMSAAx4 == action);
			ui_->actionSetMSAAx8->setChecked(ui_->actionSetMSAAx8 == action);
		};

        connect(ui_->actionSetMSAAx0, &QAction::triggered, this, [this, msaaMenuButton, updateMsaaSelection]() {
            previewWidget_->setMsaaSampleRate(PreviewMultiSampleRate::MSAA_0X);
            updateMsaaSelection(ui_->actionSetMSAAx0);
        });
        connect(ui_->actionSetMSAAx2, &QAction::triggered, this, [this, msaaMenuButton, updateMsaaSelection]() {
            previewWidget_->setMsaaSampleRate(PreviewMultiSampleRate::MSAA_2X);
            updateMsaaSelection(ui_->actionSetMSAAx2);
        });
        connect(ui_->actionSetMSAAx4, &QAction::triggered, this, [this, msaaMenuButton, updateMsaaSelection]() {
            previewWidget_->setMsaaSampleRate(PreviewMultiSampleRate::MSAA_4X);
            updateMsaaSelection(ui_->actionSetMSAAx4);
        });
        connect(ui_->actionSetMSAAx8, &QAction::triggered, this, [this, msaaMenuButton, updateMsaaSelection]() {
            previewWidget_->setMsaaSampleRate(PreviewMultiSampleRate::MSAA_8X);
            updateMsaaSelection(ui_->actionSetMSAAx8);
        });
		ui_->toolBar->insertWidget(ui_->actionSelectSizeMode, msaaMenuButton);
    }
	{
		auto* modeMenu = new QMenu{this};
		modeMenu->addAction(ui_->actionModeRoam);
		modeMenu->addAction(ui_->actionModePreview);
		connect(ui_->actionModeRoam, &QAction::triggered, scrollAreaWidget_, &PreviewScrollAreaWidget::autoModeRoam);
		connect(ui_->actionModePreview, &QAction::triggered, scrollAreaWidget_, &PreviewScrollAreaWidget::autoModePreview);

		auto* previewModeButton = new QToolButton{this};
		previewModeButton->setMenu(modeMenu);
		previewModeButton->setPopupMode(QToolButton::InstantPopup);
		connect(scrollAreaWidget_, &PreviewScrollAreaWidget::autoPreviewOrRoamModeChanged, [=](PreviewScrollAreaWidget::AutoPreviewMode mode) {
			switch (mode) {
				case PreviewScrollAreaWidget::AutoPreviewMode::ROAM:
					mode_ = 0;
					previewModeButton->setDefaultAction(ui_->actionModeRoam);
					break;
				case PreviewScrollAreaWidget::AutoPreviewMode::PREVIEW:
					mode_ = 1;
					previewModeButton->setDefaultAction(ui_->actionModePreview);
					break;
			};
		});
		ui_->toolBar->insertWidget(ui_->actionPreviewMode, previewModeButton);
    }
    {
        upLabel_ = new QLabel(this);
        upLabel_ ->setStyleSheet("color:yellow;");

        modelLabel_ = new QLabel(this);
        modelLabel_ ->setStyleSheet("color:yellow;");

        QWidget *spacer = new QWidget(this);
        spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        ui_->toolBar->addWidget(spacer);
        ui_->toolBar->addWidget(modelLabel_);
        ui_->toolBar->addWidget(upLabel_);
    }

	// Screenshot button
	{
		auto* screenshotButton = new QPushButton{};
		screenshotButton->setIcon(style::Icons::instance().screenshot);
		screenshotButton->setToolTip("Save Screenshot");

		auto* stretchedWidget = new QWidget(ui_->toolBar);
		auto* layout = new QHBoxLayout(stretchedWidget);
		layout->setContentsMargins(0, 0, 0, 0);
		layout->addStretch();
		layout->addWidget(screenshotButton);
		stretchedWidget->setLayout(layout);
		ui_->toolBar->addWidget(stretchedWidget);

		connect(screenshotButton, &QPushButton::clicked, [this]() {
			saveScreenshot();
		});
    }
}

PreviewMainWindow::~PreviewMainWindow() {
	delete previewWidget_;
}

void PreviewMainWindow::displayScene(ramses::sceneId_t sceneId, core::Vec4f const& backgroundColor) {
	previewWidget_->setBackgroundColor(backgroundColor);
	if (sceneId != previewWidget_->getSceneId()) {
		sceneIdLabel_->setText(QString{"scene id: %1"}.arg(sceneId.getValue()));
		previewWidget_->setSceneId(sceneId);
	}
}

void PreviewMainWindow::sceneScaleUpdate(bool zup, float scaleValue, bool scaleUp) {
	if (mode_ == 1) {
		// preview mode
		return;
	}
	if (scaleValue_ != scaleValue || zup != zUp_) {
		scaleValue_ = scaleValue;
        float x, y, z;
		float cameraScale = (scaleUp == true) ? 0.95 : 1.05;
        auto scene = const_cast<ramses::Scene *>(sceneBackend_->currentScene());
        ramses::RamsesObject* object = scene->findObjectByName("PerspectiveCamera");
        if (object) {
            if (object->getType() == ramses::ERamsesObjectType_PerspectiveCamera) {
                ramses::PerspectiveCamera* camera = static_cast<ramses::PerspectiveCamera*>(object);
                camera->getTranslation(x, y, z);
                camera->setTranslation(x * cameraScale, y, z * cameraScale);
			}
		}
    }
}

void PreviewMainWindow::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        if (!selModelID_.empty() && keyBoardType_ != KEY_BOARD_NONE) {
            keyBoardType_ = KEY_BOARD_NONE;
            this->setCursor(Qt::ArrowCursor);
            modelLabel_->setText("   ");
            return;
        }

        float cx, cy, cz;

        auto scene = const_cast<ramses::Scene *>(sceneBackend_->currentScene());
        ramses::RamsesObject* object = scene->findObjectByName("PerspectiveCamera");
        if (object) {
            if (object->getType() == ramses::ERamsesObjectType_PerspectiveCamera) {
                ramses::PerspectiveCamera* camera = static_cast<ramses::PerspectiveCamera*>(object);
                camera->getTranslation(cx, cy, cz);

                QVector3D position(cx, cy, cz);

                // caculate projection matrix
                QMatrix4x4 projection_matrix;
                projection_matrix.perspective(camera->getVerticalFieldOfView(), camera->getAspectRatio(), camera->getNearPlane(), camera->getFarPlane());

                // caculate view matrix
                float rX{0.0f}, rY{0.0f}, rZ{0.0f};
                ramses::ERotationConvention convention;
                camera->getRotation(rX, rY, rZ, convention);
                QMatrix4x4 view_matrix = getViewMatrix2(position, rX, rY, rZ);

                // get view position
                int width = camera->getViewportWidth();
                int height = camera->getViewportHeight();
                auto previewPosition = scrollAreaWidget_->globalPositionToPreviewPosition(event->globalPos());

                if (!previewPosition) {
                    return;
                }
                float x = 2.0f * previewPosition->x() / width - 1.0f;
                float y = 1.0f - (2.0f * (height - previewPosition->y()) / height);

                // caculate ray direction
                QVector3D ray_nds = QVector3D(x, y, 1.0f);
                QVector4D ray_clip = QVector4D(ray_nds, 1.0f);
                QVector4D ray_eye = projection_matrix.inverted() * ray_clip;
                QVector4D ray_camera = view_matrix.inverted() * ray_eye;

                QVector3D ray_world;
                if (ray_camera.w() != 0.0f) {
                    ray_world = QVector3D(ray_eye.x() / ray_eye.w(), ray_eye.y() / ray_eye.w(), ray_eye.z() / ray_eye.w());
                }

                //QQuaternion rotation = QQuaternion::fromEulerAngles(rX, rY, rZ);

                QQuaternion q1 = QQuaternion::fromAxisAndAngle(QVector3D(0, 0, 1), rZ);
                QQuaternion q2 = QQuaternion::fromAxisAndAngle(QVector3D(0, 1, 0), rY);
                QQuaternion q3 = QQuaternion::fromAxisAndAngle(QVector3D(1, 0, 0), rX);
                QQuaternion rotation = q1 * q2 * q3;

                QVector3D ray = QVector3D(ray_world - position).normalized();
                ray = rotation.rotatedVector(ray);

                selModelID_ = caculateRayIntersection(ray, position);

                // select node
                Q_EMIT signal::signalProxy::GetInstance().sigSwitchObjectNode(QString::fromStdString(selModelID_));
                Q_EMIT signal::signalProxy::GetInstance().sigSwithOutLineModel(QString::fromStdString(selModelID_));
            }
        }
    }
    QWidget::mousePressEvent(event);
}

void PreviewMainWindow::mouseReleaseEvent(QMouseEvent *event) {
    QWidget::mouseReleaseEvent(event);
}

void PreviewMainWindow::keyPressEvent(QKeyEvent *event) {
    if (!selModelID_.empty()) {
        if (event->key() == Qt::Key_Escape) {
            keyBoardType_ = KEY_BOARD_NONE;
            modelMoveDirect_ = MODEL_MOVE_DEFAULT;
            this->setCursor(Qt::ArrowCursor);
            modelLabel_->setText("   ");
            return;
        }
        auto previewPosition = scrollAreaWidget_->globalPositionToPreviewPosition(this->cursor().pos());
        if (event->key() == Qt::Key_G) {
            keyBoardType_ = KEY_BOARD_TRANSLATION;
            modelLabel_->setText("ModelMove:Translation   ");
            modelMoveDirect_ = MODEL_MOVE_DEFAULT;
        } else if (event->key() == Qt::Key_R) {
            keyBoardType_ = KEY_BOARD_ROTATION;
            modelLabel_->setText("ModelMove:Rotation   ");
            modelMoveDirect_ = MODEL_MOVE_DEFAULT;
        } else if (event->key() == Qt::Key_S) {
            keyBoardType_ = KEY_BOARD_SCALING;
            this->setCursor(Qt::SizeHorCursor);
            modelLabel_->setText("ModelMove:Scaling   ");
            selModelPos_ = QPoint(previewPosition->x(), previewPosition->y());
            modelMoveDirect_ = MODEL_MOVE_DEFAULT;
        }
        switch (keyBoardType_) {
        case KEY_BOARD_TRANSLATION: {
            if (event->key() == Qt::Key_X) {
                modelMoveDirect_ = MODEL_MOVE_X;
                this->setCursor(Qt::SizeHorCursor);
                selModelPos_ = QPoint(previewPosition->x(), previewPosition->y());
                selModelTranslation_ = calculateModelTranslation(selModelPos_);
                modelLabel_->setText("ModelMove:Translation_X   ");
            } else if (event->key() == Qt::Key_Y) {
                modelMoveDirect_ = MODEL_MOVE_Y;
                this->setCursor(Qt::SizeVerCursor);
                selModelPos_ = QPoint(previewPosition->x(), previewPosition->y());
                selModelTranslation_ = calculateModelTranslation(selModelPos_);
                modelLabel_->setText("ModelMove:Translation_Y   ");
            } else if (event->key() == Qt::Key_Z) {
                modelMoveDirect_ = MODEL_MOVE_Z;
                this->setCursor(Qt::SizeBDiagCursor);
                selModelPos_ = QPoint(previewPosition->x(), previewPosition->y());
                selModelTranslation_ = calculateModelTranslation(selModelPos_);
                modelLabel_->setText("ModelMove:Translation_Z   ");
            }
            break;
        }
        case KEY_BOARD_ROTATION: {
            if (event->key() == Qt::Key_X) {
                modelMoveDirect_ = MODEL_MOVE_X;
                this->setCursor(Qt::SizeHorCursor);
                selModelPos_ = QPoint(previewPosition->x(), previewPosition->y());
                modelLabel_->setText("ModelMove:Rotation_X   ");
            } else if (event->key() == Qt::Key_Y) {
                modelMoveDirect_ = MODEL_MOVE_Y;
                this->setCursor(Qt::SizeVerCursor);
                selModelPos_ = QPoint(previewPosition->x(), previewPosition->y());
                modelLabel_->setText("ModelMove:Rotation_Y   ");
            } else if (event->key() == Qt::Key_Z) {
                modelMoveDirect_ = MODEL_MOVE_Z;
                this->setCursor(Qt::SizeBDiagCursor);
                selModelPos_ = QPoint(previewPosition->x(), previewPosition->y());
                modelLabel_->setText("ModelMove:Rotation_Y   ");
            }
            break;
        }
        default:
            break;
        }
    }
    QWidget::keyPressEvent(event);
}

void PreviewMainWindow::setAxesIconLabel(QLabel * axesIcon) {
	axesIcon_ = axesIcon;
}

void PreviewMainWindow::setViewport(const QSize& sceneSize) {
	scrollAreaWidget_->setViewport(sceneSize);
}

void PreviewMainWindow::commit(bool forceUpdate) {
	previewWidget_->commit(forceUpdate);

	const QSize areaSize = scrollAreaWidget_->viewport()->size();
	if (sceneSize_ != areaSize) {
		sceneSize_ = areaSize;
		QRect scrollRect = QRect(scrollAreaWidget_->pos(), scrollAreaWidget_->size());
//		axesIcon_->setGeometry(scrollRect.width() - 130, scrollRect.y() + 20, 100, 100);
	}
}

void PreviewMainWindow::updateAxesIconLabel() {
	updateAxesIconLabel_ = true;
}

void PreviewMainWindow::setAxesIcon(const bool& z_up) {
    if (sceneSize_.height() != 0 && sceneSize_.width() != 0) {
        // to be doing
//        previewWidget_->setMask({0, 0, 1, 1});
        previewWidget_->update();
        if (z_up) {
            QPixmap pix = QPixmap(":zUp");
//            axesIcon_->setPixmap(pix);
            upLabel_->setText("+Z_UP");
        } else {
            QPixmap pix = QPixmap(":yUp");
//            axesIcon_->setPixmap(pix);
            upLabel_->setText("+Y_UP");
        }
		scrollAreaWidget_->setForceUpdateFlag(true);
	}
}

/*
** z_up == true,  +Z up, +Y forward
** z_up == false, +Y up, +Z forward
*/
void PreviewMainWindow::setAxes(const bool& z_up) {
	if (!haveInited_) {
        setAxesIcon(z_up);
		haveInited_ = true;
        zUp_ = z_up;
		return;
	}
    if (zUp_ == z_up) {
		if (updateAxesIconLabel_) {
			updateAxesIconLabel_ = false;
			scrollAreaWidget_->setForceUpdateFlag(false);
		}
		return;
	}

	setAxesIcon(z_up);

	for (const auto& object : project_->instances()) {
        if (/*&object->getTypeDescription() == &user_types::PerspectiveCamera::typeDescription ||*/
            &object->getTypeDescription() == &user_types::OrthographicCamera::typeDescription ||
			&object->getTypeDescription() == &user_types::Node::typeDescription ||
			&object->getTypeDescription() == &user_types::MeshNode::typeDescription) {
			if (object->getParent() == nullptr) {
				ValueHandle translation_x{object, &user_types::Node::translation_, &core::Vec3f::x};
				ValueHandle translation_y{object, &user_types::Node::translation_, &core::Vec3f::y};
				ValueHandle translation_z{object, &user_types::Node::translation_, &core::Vec3f::z};
				ValueHandle rotation_x{object, &user_types::Node::rotation_, &core::Vec3f::x};
				double x = translation_x.asDouble();
				double y = translation_y.asDouble();
				double z = translation_z.asDouble();
				if (z_up) {
					commandInterface_->set(translation_y, -z);
					commandInterface_->set(translation_z, y);
				} else {
					commandInterface_->set(translation_y, z);
					commandInterface_->set(translation_z, -y);
				}
				double offset = z_up ? (rotation_x.asDouble() + 90.0) : (rotation_x.asDouble() - 90.0);
				if (offset < -360.0) {
					commandInterface_->set(rotation_x, (offset + 360.0));
				} else if (offset > 360.0) {
					commandInterface_->set(rotation_x, (offset - 360.0));
				} else {
					commandInterface_->set(rotation_x, offset);
				}
			}
		}
	}
	zUp_ = z_up;
	signal::signalProxy::GetInstance().sigInitPropertyBrowserView();
}

void PreviewMainWindow::setEnableDisplayGrid(bool enable) {
	previewWidget_->setEnableDisplayGrid(enable);
}

void PreviewMainWindow::sceneUpdate(bool z_up) {
    previewWidget_->sceneUpdate(z_up, scaleValue_);
}

QMatrix4x4 PreviewMainWindow::getViewMatrix(QVector3D position, float rX, float rY, float rZ) {
    QVector3D worldUp(0.0, 1.0, 0.0);

//    float yawR = qDegreesToRadians(rY);
    float yawR = qDegreesToRadians(-rY - 90.0f);
    float picthR = qDegreesToRadians(rX);
    float rollR = qDegreesToRadians(rZ + 90);

    QVector3D direction = QVector3D(cos(yawR) * cos(picthR), sin(picthR), sin(yawR) * cos(picthR));

    QVector3D front = (direction).normalized();
    QVector3D right = QVector3D::crossProduct(front, worldUp).normalized();
    QVector3D up = QVector3D::crossProduct(right, front).normalized();

    QMatrix4x4 view;
    view.lookAt(position, position + front, up);
    return view;
}

QMatrix4x4 PreviewMainWindow::getViewMatrix2(QVector3D position, float rX, float rY, float rZ) {
//    QQuaternion q = QQuaternion::fromEulerAngles(qDegreesToRadians(rX), qDegreesToRadians(rY), qDegreesToRadians(rZ));
//    QMatrix4x4 R(q.toRotationMatrix());

//    QMatrix4x4 T;
//    T.setToIdentity();
//    T.translate(-QVector3D(position.x(), position.y(), position.z()));

//    QMatrix4x4 ViewMatrix;
//    ViewMatrix = R.transposed() * T;
//    return ViewMatrix;

    QVector3D target(0, 0, 0);
    QVector3D up(0, 1, 0);
    QVector3D direction = target - position;

    QVector3D right = QVector3D::crossProduct(direction, up).normalized();
    QVector3D trueUp = QVector3D::crossProduct(right, direction).normalized();

    QMatrix4x4 ViewMatrix;
    ViewMatrix.setToIdentity();
    ViewMatrix.lookAt(position, position + direction, trueUp);
    return ViewMatrix;
}

QVector<float> PreviewMainWindow::checkTriCollision(QVector3D ray, QVector3D camera, QVector<QVector<QVector3D> > triangles) {
    QVector<float> vec_t;

    for (auto triangle : triangles) {
        float t = checkSingleTriCollision(ray, camera, triangle);
        vec_t.push_back(t);
    }
    return vec_t;
}

float PreviewMainWindow::checkSingleTriCollision(QVector3D ray, QVector3D camera, QVector<QVector3D> triangle) {
    QVector3D E1 =  triangle[1] - triangle[0];
    QVector3D E2 =  triangle[2] - triangle[0];

    QVector3D P = QVector3D::crossProduct(ray, E2);
    float det = QVector3D::dotProduct(P, E1);
    QVector3D T;
    if (det > 0)
       T = camera - triangle[0];
    else {
       T = triangle[0] - camera;
       det = -det;
    }

    if (det < 0.00001f)
      return -1.0f;

    float u = QVector3D::dotProduct(P, T);
    if (u < 0.0f || u > det)
      return -1.0f;

    QVector3D Q = QVector3D::crossProduct(T, E1);
    float v = QVector3D::dotProduct(Q, ray);
    if(v < 0.0f || u+v > det)
      return -1.0f;

    float t = QVector3D::dotProduct(Q, E2);
    if (t < 0.0f)
      return -1.0f;

    return t/det;
}

QMap<std::string, QVector<QVector<QVector3D> > > PreviewMainWindow::getMeshTriangles() {
    QMap<std::string, QVector<QVector<QVector3D> >> meshTriangles;
    for (auto it : guiData::MeshDataManager::GetInstance().getMeshDataMap()) {
        QVector<QVector<QVector3D> > triangles;
        QMatrix4x4 modelMatrix = it.second.getModelMatrix();

        std::vector<uint32_t> indices = it.second.getIndices();
        int triCount = indices.size() / 3;

        int posIndex = guiData::MeshDataManager::GetInstance().attriIndex(it.second.getAttributes(), "a_Position");
        if (posIndex != -1) {
            guiData::Attribute attri = it.second.getAttributes().at(posIndex);
            auto verticesData = reinterpret_cast<float *>(attri.data.data());

            for (int i{0}; i < triCount; ++i) {
                uint32_t pos = i * 3;
                QVector<QVector3D> triangle;
                for (int j{0}; j < 3; j++) {
                    int index = indices[pos + j] * 3;
                    QVector3D vertex(verticesData[index], verticesData[index + 1], verticesData[index + 2]);

                    vertex = modelMatrix * vertex;
                    triangle.append(vertex);
                }
                triangles.append(triangle);
            }
        }
        meshTriangles.insert(it.first, triangles);
    }
    return meshTriangles;
}

std::string PreviewMainWindow::caculateRayIntersection(QVector3D ray, QVector3D cameraPos) {
    // get mesh data
    QMap<std::string, QVector<QVector<QVector3D> >> meshTriangles = getMeshTriangles();

    std::string objectID;
    double t{1000.0};
    for (auto it : meshTriangles.toStdMap()) {
        QVector<float> vec_t = checkTriCollision(ray, cameraPos, it.second);

        QVector<float> vec_comt;
        for (auto iter: vec_t) {
            if (iter > 0.0f)
                vec_comt.push_back(iter);
        }

        if (!vec_comt.isEmpty()) {
            std::sort(vec_comt.begin(), vec_comt.end());
            if (vec_comt[0] < t && vec_comt[0] > 0) {
                t = vec_comt[0];
                objectID = it.first;
            }
        }
    }
    return objectID;
}

void PreviewMainWindow::mouseMove(QPoint position) {
    if (modelMoveDirect_ == MODEL_MOVE_DEFAULT && keyBoardType_ != KEY_BOARD_SCALING) {
        return;
    }

    if (!selModelID_.empty()) {
        switch (keyBoardType_) {
        case KEY_BOARD_TRANSLATION:{
            translationMovement(position);
            break;
        }
        case KEY_BOARD_ROTATION:{
            rotationMovement(position);
            break;
        }
        case KEY_BOARD_SCALING: {
            scalingMovement(position);
            break;
        }
        default:
            break;
        }
    }
}

void PreviewMainWindow::translationMovement(QPoint position) {
    QVector3D model = calculateModelTranslation(position);
    double offsetX = model.x() - selModelTranslation_.x();
    double offsetY = model.y() - selModelTranslation_.y();
    double offsetZ = model.z() - selModelTranslation_.z();
    selModelPos_ = position;

    switch (modelMoveDirect_) {
    case MODEL_MOVE_X: {
        signal::signalProxy::GetInstance().sigUpdateMeshNodeTransProperty(selModelID_, offsetX, 0, 0);
        break;
    }
    case MODEL_MOVE_Y: {
        signal::signalProxy::GetInstance().sigUpdateMeshNodeTransProperty(selModelID_, 0, offsetY, 0);
        break;
    }
    case MODEL_MOVE_Z: {
        signal::signalProxy::GetInstance().sigUpdateMeshNodeTransProperty(selModelID_, 0, 0, -offsetZ);
        break;
    }
    default:
        break;
    }
}

void PreviewMainWindow::rotationMovement(QPoint position) {
    auto scene = const_cast<ramses::Scene *>(sceneBackend_->currentScene());
    ramses::RamsesObject* object = scene->findObjectByName("PerspectiveCamera");
    if (object) {
        ramses::PerspectiveCamera* camera = static_cast<ramses::PerspectiveCamera*>(object);

        float rx, ry, rz;
        camera->getRotation(rx, ry, rz);

        int width = camera->getViewportWidth();
        int height = camera->getViewportHeight();

        float dx = position.x() - selModelPos_.x();
        float dy = -(position.y() - selModelPos_.y());
        float dz = sqrt(dx * dx + dy * dy);
        if (dx >= 0 && dy <= 0) {
            dz = -dz;
        }

        switch (modelMoveDirect_) {
        case MODEL_MOVE_X: {
            dy = 0;
            dz = 0;
            break;
        }
        case MODEL_MOVE_Y: {
            dx = 0;
            dz = 0;
            break;
        }
        case MODEL_MOVE_Z: {
            dx = 0;
            dy = 0;
            break;
        }
        default:
            break;
        }
        QVector3D euler = QVector3D(dy * 0.1f, dx * 0.1f, dz * 0.1f);

        selModelPos_.setX(position.x());
        selModelPos_.setY(position.y());

        signal::signalProxy::GetInstance().sigUpdateMeshNodeRotationProperty(selModelID_, euler[0], euler[1], euler[2]);
    }
}

void PreviewMainWindow::scalingMovement(QPoint position) {
    auto pixelScaling = 0.01f;
    int moveX = position.x() - selModelPos_.x();
    if (moveX == 0) {
        return;
    }
    double offsetScaling = moveX * pixelScaling;

    selModelPos_ = QPoint(position.x(), position.y());
    signal::signalProxy::GetInstance().sigUpdateMeshNodeScalingProperty(selModelID_, offsetScaling);
}

QVector3D PreviewMainWindow::calculateModelTranslation(QPoint position) {
    float cx, cy, cz;

    auto scene = const_cast<ramses::Scene *>(sceneBackend_->currentScene());
    ramses::RamsesObject* object = scene->findObjectByName("PerspectiveCamera");
    if (object) {
        ramses::PerspectiveCamera* camera = static_cast<ramses::PerspectiveCamera*>(object);
        camera->getTranslation(cx, cy, cz);

        QVector3D cameraPos(cx, cy, cz);

        // caculate projection matrix
        QMatrix4x4 projection_matrix;
        projection_matrix.perspective(camera->getVerticalFieldOfView(), camera->getAspectRatio(), camera->getNearPlane(), camera->getFarPlane());

        // caculate view matrix
        float rX{0.0f}, rY{0.0f}, rZ{0.0f};
        ramses::ERotationConvention convention;
        camera->getRotation(rX, rY, rZ, convention);
        QMatrix4x4 view_matrix = getViewMatrix2(cameraPos, rX, rY, rZ);

        QMatrix4x4 vp_matrix = view_matrix.transposed() * projection_matrix.transposed();
        QMatrix4x4 inverse_vp_matrix = view_matrix.transposed().inverted() * projection_matrix.transposed().inverted();

        // get view position
        int width = camera->getViewportWidth();
        int height = camera->getViewportHeight();

        float x = position.x() - selModelPos_.x();
        float y = position.y() - selModelPos_.y();

        guiData::NodeData* node = guiData::NodeDataManager::GetInstance().searchNodeByID(selModelID_);
        if (node) {
            Vec3 tran = std::any_cast<Vec3>(node->getSystemData("translation"));
            QVector3D translation(tran.x, tran.y, tran.z);

            // caculate zfac
            float zfac = abs(vp_matrix.row(0).w() * translation.x() + vp_matrix.row(1).w() * translation.y()
                            + vp_matrix.row(2).w() * translation.z() + vp_matrix.row(3).w());
            if (zfac < 1.e-6f && zfac > -1.e-6f) {
                zfac = 1.0f;
            }

            float dx = 2.0f * x * zfac / width;
            float dy = 2.0f * y * zfac / height;

            QVector3D model(inverse_vp_matrix.row(0).x() * dx + inverse_vp_matrix.row(1).x() * dy,
                            inverse_vp_matrix.row(0).y() * dx + inverse_vp_matrix.row(1).y() * dy,
                            inverse_vp_matrix.row(0).z() * dx + inverse_vp_matrix.row(1).z() + dy);

            return model;
        }
    }
    return QVector3D();
}

static QMatrix4x4 getRotateY(float r) {
     return QMatrix4x4((float)(cos(r)), 0, (float)(sin(r)), 0,
                       0, 0, 0, 0,
                       (float)(sin(r)), 0, (float)(sin(r)), 0,
                       0, 0, 0, 0);
}

static QMatrix4x4 getRotateX(float r) {
    return QMatrix4x4(0, 0, 0, 0,
                      0, (float)(cos(r)), (float)(sin(r)), 0,
                      0, (float)(sin(r)), (float)(cos(r)), 0,
                      0, 0, 0, 0);
}

static QMatrix4x4 getRotateZ(float r) {
     return QMatrix4x4((float)(cos(r)), (float)(sin(r)), 0, 0,
                       (float)(sin(r)), (float)(cos(r)), 0, 0,
                       0, 0, 0, 0,
                       0, 0, 0, 0);
}

static QMatrix4x4 getTranslate(float x, float y, float z) {
    return QMatrix4x4(1, 0, 0, 0,
                         0, 1, 0, 0,
                         0, 0, 1, 0,
                         x, y, z, 1);
}

void PreviewMainWindow::saveScreenshot() {
	const auto screenshotDir = components::RaCoPreferences::instance().screenshotDirectory.toStdString();
	if (screenshotDir.empty()) {
		QMessageBox::warning(this, "Could not save the screenshot", "Please make sure that the directory specified in \"File > Preferences > Screenshot Directory\" is not empty.", QMessageBox::Ok);
		return;
	}

	auto projectName = project_->projectName();
	if (projectName.empty()) {
		projectName = "default";
	}

	const auto currentTime = QDateTime::currentDateTime().toString("yyyy-MM-dd_HH-mm-ss-zzz").toStdString();
	const auto screenshotFileName = screenshotDir + "/" + projectName + "_" + currentTime + ".png";

	const auto saved = previewWidget_->saveScreenshot(screenshotFileName);
	if (!saved) {
		QMessageBox::warning(this, "Saving screenshot failed", "Could not save the screenshot. Please make sure that the directory specified in \"File > Preferences > Screenshot Directory\" exists and is accessable.", QMessageBox::Ok);
	}
}

void PreviewMainWindow::saveScreenshot(const std::string& fullPath) {
	const auto saved = previewWidget_->saveScreenshot(fullPath);
	if (!saved) {
		throw std::runtime_error {"Could not save screenshot to \"" + fullPath + "\". Please make sure that the path specified is correct and accessable."};
	}
}


}  // namespace raco::ramses_widgets
