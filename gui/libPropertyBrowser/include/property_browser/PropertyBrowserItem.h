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

#include "core/Context.h"
#include "core/CommandInterface.h"
#include "core/ErrorItem.h"
#include "core/Handles.h"
#include "core/Queries.h"
#include "core/SceneBackendInterface.h"
#include "components/DataChangeDispatcher.h"
#include <QList>
#include <QMetaMethod>
#include <QObject>
#include <QString>
#include <functional>
#include <optional>
#include <sstream>
#include <string>

namespace raco::property_browser {
class PropertyBrowserModel;

class PropertyBrowserRef;

/**
 * Indirection wrapper around ValueHandle. 
 */
class PropertyBrowserItem final : public QObject {
	Q_OBJECT
public:
	friend std::string to_string(PropertyBrowserItem& item);


	PropertyBrowserItem(raco::core::ValueHandle valueHandle, raco::components::SDataChangeDispatcher dispatcher, raco::core::CommandInterface* commandInterface, raco::core::SceneBackendInterface* sceneBackend, PropertyBrowserModel *model, QObject* parent = nullptr);
	raco::core::PrimitiveType type() const noexcept;
	std::string luaTypeName() const noexcept;
	std::string displayName() const noexcept;
	size_t size() noexcept;
	raco::core::Queries::LinkState linkState() const noexcept;
	std::string linkText(bool fullLinkPath = false) const noexcept;
	void setLink(const core::ValueHandle& start, bool isWeak) noexcept;
	void removeLink() noexcept;
	bool editable() noexcept;
	bool expandable() const noexcept;
	template <typename T>
	raco::core::AnnotationHandle<T> query() const {
		return valueHandle_.query<T>();
	}
	template <typename T>
	void set(T v) {
		commandInterface_->set(valueHandle_, v);
	}
	void setTags(std::vector<std::string> const& tags);
	void setTags(std::vector<std::pair<std::string, int>> const& prioritizedTags);

	raco::core::Project* project() const;
	const raco::core::CommandInterface* commandInterface() const;
	raco::components::SDataChangeDispatcher dispatcher() const;
	raco::core::EngineInterface& engineInterface() const;
	raco::core::ValueHandle& valueHandle() noexcept;
	const QList<PropertyBrowserItem*>& children();
	PropertyBrowserItem* parentItem() const noexcept;
	PropertyBrowserItem* siblingItem(std::string_view propertyName) const noexcept;
	PropertyBrowserRef* refItem() noexcept;
	PropertyBrowserModel* model() const noexcept;
	bool isRoot() const noexcept;
	/** locates the firstitem in hierachy which has no collapsed parent (can be this) */
	PropertyBrowserItem* findItemWithNoCollapsedParentInHierarchy() noexcept;
	bool hasCollapsedParent() const noexcept;
	/** Local expanded flag, doesn't take into account if ancestors are collapsed or not. */
	bool expanded() const noexcept;
	void setExpanded(bool expanded) noexcept;
	void setExpandedRecursively(bool expanded) noexcept;
	bool showChildren() const;
	
	void requestNextSiblingFocus();

	const core::ErrorItem& error() const;
	bool hasError() const noexcept;

	void markForDeletion();

	bool canBeChosenByColorPicker() const;

Q_SIGNALS:
	void linkStateChanged(raco::core::Queries::LinkState state);
	void linkTextChanged(const QString& text);
	void childrenChangedOrCollapsedChildChanged();
	void showChildrenChanged(bool show);
	void valueChanged(raco::core::ValueHandle& v);
	void errorChanged(raco::core::ValueHandle& v);
	void displayNameChanged(const QString& string);
	void expandedChanged(bool expanded);
	void childrenChanged(const QList<PropertyBrowserItem*>& children);
	void editableChanged(bool editable);
	void widgetRequestFocus();

protected:
	Q_SLOT void updateLinkState() noexcept;

private:
	void createChildren(core::SceneBackendInterface* sceneBackend);
	void syncChildrenWithValueHandle(core::SceneBackendInterface* sceneBackend);

	PropertyBrowserItem* parentItem_{nullptr};
	PropertyBrowserRef* refItem_{nullptr};
	raco::core::ValueHandle valueHandle_;
	raco::components::Subscription subscription_;
	raco::components::Subscription errorSubscription_;
	raco::components::Subscription linkValidityChangeSub_;
	raco::components::Subscription linkLifecycleStartSub_;
	raco::components::Subscription linkLifecycleEndSub_;
	raco::components::Subscription changeChildrenSub_;
	raco::core::CommandInterface* commandInterface_;
	raco::components::SDataChangeDispatcher dispatcher_;
	PropertyBrowserModel* model_;
	QList<PropertyBrowserItem*> children_;
	bool expanded_;
	bool editable_ = true;

	bool getDefaultExpandedFromValueHandleType() const;
};

std::string to_string(PropertyBrowserItem& item);

}  // namespace raco::property_browser
