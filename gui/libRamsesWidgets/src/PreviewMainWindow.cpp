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

#include "ui_PreviewMainWindow.h"
#include <QLabel>
#include <QMenu>
#include <QToolButton>
#include <windows.h>
#include <QMatrix4x4>
#include <log_system/log.h>

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

PreviewMainWindow::PreviewMainWindow(RendererBackend& rendererBackend, raco::ramses_adaptor::SceneBackend* sceneBackend, const QSize& sceneSize, raco::core::Project* project,
	raco::components::SDataChangeDispatcher dispatcher, raco::core::CommandInterface* commandInterface, QWidget* parent)
	: project_(project),
	  commandInterface_(commandInterface),
	  sceneBackend_(sceneBackend),
	  QMainWindow{parent},
	  ui_{new Ui::PreviewMainWindow()} {
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
		this->sceneScaleUpdate(this->zUp_, (float)scale, addvalue);
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
        QWidget *spacer = new QWidget(this);
        spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        ui_->toolBar->addWidget(spacer);
        ui_->toolBar->addWidget(upLabel_);
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
//                if (zup) {
//                    camera->setTranslation(x * cameraScale, y * cameraScale, z);
//                } else {
//                    camera->setTranslation(x * cameraScale, y, z * cameraScale);
//                }
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
                QMatrix4x4 view_matrix = getViewMatrix(position, rX, rY, rZ);

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
                QVector4D ray_world = view_matrix.inverted() * ray_eye;

                if (ray_world.w() != 0.0f) {
                    ray_world.setX(ray_world.x()/ray_world.w());
                    ray_world.setY(ray_world.y()/ray_world.w());
                    ray_world.setZ(ray_world.z()/ray_world.w());
                }
                QVector3D ray = (QVector3D(ray_world.x(), ray_world.y(), ray_world.z()) - position).normalized();

                // caculate Ray Intersection; return: node objectID
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
        if (keyBoardType_ != KEY_BOARD_NONE) {
            keyBoardType_ = KEY_BOARD_NONE;
            this->setCursor(Qt::ArrowCursor);
            return;
        }
        auto previewPosition = scrollAreaWidget_->globalPositionToPreviewPosition(this->cursor().pos());
        if (event->key() == Qt::Key_G) {
            keyBoardType_ = KEY_BOARD_TRANSLATION;
            this->setCursor(Qt::SizeAllCursor);
            selModelPos_ = QPoint(previewPosition->x(), previewPosition->y());
        } else if (event->key() == Qt::Key_R) {
            keyBoardType_ = KEY_BOARD_ROTATION;
            this->setCursor(Qt::SizeAllCursor);
            selModelPos_ = QPoint(previewPosition->x(), previewPosition->y());
        } else if (event->key() == Qt::Key_S) {
            keyBoardType_ = KEY_BOARD_SCALING;
            this->setCursor(Qt::SizeHorCursor);
            selModelPos_ = QPoint(previewPosition->x(), previewPosition->y());
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

    float yawR = qDegreesToRadians(-rY - 90.0f);
    float picthR = qDegreesToRadians(rX);
    float rollR = qDegreesToRadians(rZ);

    QVector3D direction = QVector3D(cos(yawR) * cos(picthR), sin(picthR), sin(yawR) * cos(picthR));

    QVector3D front = (direction).normalized();
    QVector3D right = QVector3D::crossProduct(front, worldUp).normalized();
    QVector3D up = QVector3D::crossProduct(right, front).normalized();

    QMatrix4x4 view;
    view.lookAt(position, position + front, up);
    return view;
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
                    QVector4D temp(verticesData[index], verticesData[index + 1], verticesData[index + 2], 1.0f);

                    temp = modelMatrix * temp;
                    QVector3D vertex(temp.x(), temp.y(), temp.z());
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
        QMatrix4x4 view_matrix = getViewMatrix(cameraPos, rX, rY, rZ);

        // get view position
//        int width = this->scrollAreaWidget_->width();
//        int height = this->scrollAreaWidget_->height();
        int width = camera->getViewportWidth();
        int height = camera->getViewportHeight();

        float x = position.x() - selModelPos_.x();
        float y = position.y() - selModelPos_.y();

        guiData::NodeData* node = guiData::NodeDataManager::GetInstance().searchNodeByID(selModelID_);
        if (node) {
            Vec3 tran = std::any_cast<Vec3>(node->getSystemData("translation"));
            QVector3D translation(tran.x, tran.y, tran.z);

            // caculate zfac
            float zfac = abs(projection_matrix.row(0).w() * translation.x() + projection_matrix.row(1).w() * translation.y()
                            + projection_matrix.row(2).w() * translation.z() + projection_matrix.row(3).w());
            if (zfac < 1.e-6f && zfac > -1.e-6f) {
                zfac = 1.0f;
            }

            float dx = 2.0f * x * zfac / width;
            float dy = 2.0f * y * zfac / height;

            QMatrix4x4 inverse_projection_matrix = projection_matrix.inverted();
            QVector3D model(inverse_projection_matrix.row(0).x() * dx + inverse_projection_matrix.row(1).x() * dy,
                            inverse_projection_matrix.row(0).y() * dx + inverse_projection_matrix.row(1).y() * dy,
                            inverse_projection_matrix.row(0).z() * dx + inverse_projection_matrix.row(1).z() + dy);

            signal::signalProxy::GetInstance().sigUpdateMeshNodeTransProperty(selModelID_, model.x(), model.y());
        }

//        int width = camera->getViewportWidth();
//        int height = camera->getViewportHeight();

//        float x = 2.0f * position.x() / width - 1.0f;
//        float y = 1.0f - (2.0f * position.y()) / height;

//        float x2 = 2.0f * selModelPos_.x() / width - 1.0f;
//        float y2 = 1.0f - (2.0f * selModelPos_.y()) / height;

//        guiData::MeshData meshData;
//        if (guiData::MeshDataManager::GetInstance().getMeshData(selModelID_, meshData)) {

//            auto model_matrix = meshData.getModelMatrix();

//            QVector4D clip = QVector4D(position.x(), position.y(), 0.0, 1.0);
//            QVector4D eye = projection_matrix.inverted() * clip;
//            QVector4D world = view_matrix.inverted() * eye;
//            QVector4D model = world * model_matrix;

//            QVector4D clip2 = QVector4D(selModelPos_.x(), selModelPos_.y(), 0.0, 1.0);
//            QVector4D eye2 = projection_matrix.inverted() * clip2;
//            QVector4D world2 = view_matrix.inverted() * eye2;

//            float offsetX = world.x() - world2.x();
//            float offsetY = world.y() - world2.y();

//            selModelPos_.setX(position.x());
//            selModelPos_.setY(position.y());

//            signal::signalProxy::GetInstance().sigUpdateMeshNodeTransProperty(selModelID_, offsetX, offsetY);
//            qDebug() << "test model move:" << offsetX << offsetY;
//        }
    }
}

void PreviewMainWindow::rotationMovement(QPoint position) {
    auto scene = const_cast<ramses::Scene *>(sceneBackend_->currentScene());
    ramses::RamsesObject* object = scene->findObjectByName("PerspectiveCamera");
    if (object) {
        ramses::PerspectiveCamera* camera = static_cast<ramses::PerspectiveCamera*>(object);

        int width = camera->getViewportWidth();
        int height = camera->getViewportHeight();

        float centerScreen[2] = {width / 2.0f, height / 2.0f};
        float dir_prev[2], dir_cur[2];
        dir_prev[0] = selModelPos_.x() - centerScreen[0];
        dir_prev[1] = selModelPos_.y() - centerScreen[1];
        dir_cur[0] = position.x() - centerScreen[0];
        dir_cur[1] = position.y() - centerScreen[1];

        if (normalize_v2(dir_prev) && normalize_v2(dir_cur)) {
            float dphi = angle_normalize_v2v2(dir_prev, dir_cur);

            float cross = dir_prev[0] * dir_cur[1] - dir_prev[1] * dir_cur[0];
            if (cross > 0.0f) {
                dphi = -dphi;
            }

            selModelPos_.setX(position.x());
            selModelPos_.setY(position.y());

            signal::signalProxy::GetInstance().sigUpdateMeshNodeRotationProperty(selModelID_, dphi, 0.0f);
        }
    }
    return;

    auto pixelRotation = 0.05f;
    int moveX{0}, moveY{0};
    moveX = position.x() - selModelPos_.x();
    moveY = position.y() - selModelPos_.y();
    double offsetRotationX{0.0}, offsetRotationY{0.0};
    if (moveX == 0 && moveY == 0) {
        return;
    }
    offsetRotationY = moveX * pixelRotation;
    offsetRotationX = -(moveY * pixelRotation);

    selModelPos_ = QPoint(position.x(), position.y());

    signal::signalProxy::GetInstance().sigUpdateMeshNodeRotationProperty(selModelID_, offsetRotationX, offsetRotationY);
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

}  // namespace raco::ramses_widgets
