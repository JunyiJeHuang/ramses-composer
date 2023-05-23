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

#include <QFrame>
#include <QPushButton>
#include <QStyle>
#include <QWidget>
#include <QLabel>
#include <QClipboard>
#include <QStandardItemModel>
#include <QComboBox>

#include "core/SceneBackendInterface.h"
#include "property_browser/PropertyBrowserItem.h"
#include "property_browser/PropertyBrowserLayouts.h"
#include "property_browser/PropertyBrowserModel.h"
#include "property_browser/PropertySubtreeChildrenContainer.h"
#include "property_browser/controls/ExpandButton.h"
#include "CurveData/CurveManager.h"
#include "NodeData/nodeManager.h"
#include "AnimationData/animationData.h"
#include "signal/SignalProxy.h"

using namespace raco::signal;
using namespace raco::guiData;
namespace raco::property_browser {
class PropertyControl;
class PropertyEditor;
class EmbeddedPropertyBrowserView final : public QFrame {
public:
	explicit EmbeddedPropertyBrowserView(PropertyBrowserItem* item, QWidget* parent);
};


class PropertySubtreeView final : public QWidget {
	Q_OBJECT
	Q_PROPERTY(float highlight MEMBER highlight_ NOTIFY update)
public:
	explicit PropertySubtreeView(raco::core::SceneBackendInterface* sceneBackend, PropertyBrowserModel* model, PropertyBrowserItem* item, QWidget* parent);
	PropertyBrowserItem const* item() { return item_; }
    void mousePressEvent(QMouseEvent* event) override;
	void setUniformControls(PropertyBrowserItem* item, PropertyBrowserHBoxLayout* labelLayout);
	std::vector<Uniform> Item2Uniform(PropertyBrowserItem* item);
	void setUniformsProperty(core::ValueHandle valueHandle, Uniform& tempUniform);

public Q_SLOTS:
	void playStructureChangeAnimation();
	void setLabelAreaWidth(int offset);
    void updateChildrenContainer();
	QString setCurveName(QString name);
	QString setPropertyName(QString name);
    void slotInsertKeyFrame();
    void slotCopyProperty();
	void updateUniformCombox();
	void delUniformButtonClicked();
	void slotUniformNameChanged(QString s);
	void updateMaterial(raco::core::ValueHandle& v);
    void updateMesh(raco::core::ValueHandle& v);
    void updateNode(raco::core::ValueHandle& v);

protected:
	void paintEvent(QPaintEvent* event) override;
	int getLabelAreaWidthHint() const;
	Q_SLOT void updateError();

private:
	void recalculateLabelWidth();
	void collectTabWidgets(QObject* item, QWidgetList& tabWidgets);
	void recalculateTabOrder();
	void registerCopyPasteContextMenu(QWidget* widget);
	raco::core::SceneBackendInterface* sceneBackend_;
    bool isValidValueHandle(QStringList list, raco::core::ValueHandle handle);
	void generateItemTooltip(PropertyBrowserItem* item, bool connectWithChangeEvents);

	PropertyBrowserItem* item_{nullptr};
	PropertyBrowserModel* model_ {nullptr};
	PropertyBrowserGridLayout layout_{nullptr};
	QWidget* decorationWidget_{nullptr};
	QLabel* label_{nullptr};
	PropertyEditor* propertyControl_{nullptr};
	PropertySubtreeChildrenContainer* childrenContainer_{nullptr};
    QAction* insertKeyFrameAction_{nullptr};

	int labelWidth_{0};
	float highlight_{0};

	// remove uniform attribute
	QPushButton* uniformDelButton_{nullptr};
	// Insert uniform attribute
	QComboBox* uniformComBox_{nullptr};
	bool isUniform_{false};
	bool isChecked_{false};
	QPalette palette_;
	QWidget* labelContainer_;
};

}  // namespace raco::property_browser
