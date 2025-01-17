/*
 * SPDX-License-Identifier: MPL-2.0
 *
 * This file is part of Ramses Composer
 * (see https://github.com/bmwcarit/ramses-composer).
 *
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
 * If a copy of the MPL was not distributed with this file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include "property_browser/PropertyBrowserItem.h"

#include "core/CoreFormatter.h"
#include "core/Errors.h"
#include "core/Project.h"
#include "core/PropertyDescriptor.h"
#include "core/Queries.h"
#include "core/BasicAnnotations.h"
#include "property_browser/PropertyBrowserRef.h"

#include "user_types/EngineTypeAnnotation.h"
#include "user_types/LuaInterface.h"
#include "user_types/LuaScript.h"
#include "user_types/MeshNode.h"
#include "user_types/RenderPass.h"

using raco::log_system::PROPERTY_BROWSER;
using raco::data_storage::PrimitiveType;

using SDataChangeDispatcher = raco::components::SDataChangeDispatcher;

namespace raco::property_browser {

	// Lua start index counting at 1 instead of 0
constexpr size_t LUA_INDEX_OFFSET = 1;

PropertyBrowserItem::PropertyBrowserItem(
	core::ValueHandle valueHandle,
	SDataChangeDispatcher dispatcher,
	core::CommandInterface* commandInterface,
	core::SceneBackendInterface* sceneBackend,
	PropertyBrowserModel *model,
	QObject* parent)
	: QObject{parent},
	  parentItem_{dynamic_cast<PropertyBrowserItem*>(parent)},
	  valueHandle_{std::move(valueHandle)},
	  subscription_{dispatcher->registerOn(valueHandle_, [this, sceneBackend]() {
		  if (valueHandle_.isObject() || hasTypeSubstructure(valueHandle_.type())) {
			  syncChildrenWithValueHandle(sceneBackend);
		  }
		  Q_EMIT valueChanged(valueHandle_);
		  if (valueHandle_.isProperty()) {
			  if (auto newEditable = editable(); newEditable != editable_) {
				  editable_ = newEditable;
				  Q_EMIT editableChanged(editable());
				  Q_EMIT linkStateChanged(linkState());
			  }
		  }
	  })},
	  errorSubscription_{dispatcher->registerOnErrorChanged(valueHandle_, [this]() {
		  Q_EMIT errorChanged(valueHandle_);
	  })},

	  commandInterface_{commandInterface},
	  dispatcher_{dispatcher},
	  model_{model},
	  expanded_{getDefaultExpandedFromValueHandleType()} {
	createChildren(sceneBackend);
	if (!valueHandle_.isObject() && valueHandle_.type() == core::PrimitiveType::Ref) {
		refItem_ = new PropertyBrowserRef(this);
	}
	
	if (valueHandle_.isProperty()) {
		editable_ = editable();
	}

	bool linkEnd = raco::core::Queries::isValidLinkEnd(*project(), valueHandle_);
	bool linkStart = raco::core::Queries::isValidLinkStart(valueHandle_);
	
	auto linkValidityCallback = [this](const core::LinkDescriptor& link) {
		auto endHandle = core::ValueHandle(link.end);
		if (endHandle && endHandle == valueHandle_) {
			updateLinkState();
		}
		auto startHandle = core::ValueHandle(link.start);
		if (startHandle && startHandle == valueHandle_) {
			updateLinkState();
		}
	};

	if (linkStart || linkEnd) {
		if (auto link = raco::core::Queries::getLink(*project(), valueHandle_.getDescriptor())) {
			linkValidityChangeSub_ = dispatcher_->registerOnLinkValidityChange(linkValidityCallback);
		}
	}
	
	if (linkEnd) {
		linkLifecycleEndSub_ = dispatcher_->registerOnLinksLifeCycleForEnd(
			valueHandle_.rootObject(),
			[this, linkValidityCallback](const core::LinkDescriptor& link) {
			if (valueHandle_) {
				auto endHandle = core::ValueHandle(link.end);
				if (endHandle && endHandle == valueHandle_) {
					linkValidityChangeSub_ = dispatcher_->registerOnLinkValidityChange(linkValidityCallback);
					updateLinkState();
				}
			} },
			[this](const core::LinkDescriptor& link) {
				if (valueHandle_) {
					auto endHandle = core::ValueHandle(link.end);
					if (endHandle && endHandle == valueHandle_) {
						linkValidityChangeSub_ = components::Subscription();
						updateLinkState();
					}
				}
			});
	}
	if (linkStart) {
		linkLifecycleStartSub_ = dispatcher_->registerOnLinksLifeCycleForStart(
			valueHandle_.rootObject(),
			[this, linkValidityCallback](const core::LinkDescriptor& link) {
			if (valueHandle_) {
				auto startHandle = core::ValueHandle(link.start);
				if (startHandle && startHandle == valueHandle_) {
					linkValidityChangeSub_ = dispatcher_->registerOnLinkValidityChange(linkValidityCallback);
					updateLinkState();
				}
			} },
			[this](const core::LinkDescriptor& link) {
				if (valueHandle_) {
					auto startHandle = core::ValueHandle(link.start);
					if (startHandle && startHandle == valueHandle_) {
						linkValidityChangeSub_ = components::Subscription();
						updateLinkState();
					}
				}
			});
	}

	// This needs refactoring. There needs to be a way for a user_type to say that some properties are only visible (or enabled?)
	// when certain conditions depending on another property are fulfilled.
	static const std::map<data_storage::ReflectionInterface::TypeDescriptor const*, std::string> requiredChildSubscriptions = {
		{&user_types::RenderPass::typeDescription, "target"},
		{&user_types::RenderBuffer::typeDescription, "format"},
		{&user_types::RenderLayer::typeDescription, "sortOrder"}
	};
	if (const auto itChildSub = requiredChildSubscriptions.find(&valueHandle_.rootObject()->getTypeDescription()); valueHandle_.depth() == 0 && itChildSub != requiredChildSubscriptions.end()) {
		changeChildrenSub_ = dispatcher->registerOn(core::ValueHandle{valueHandle_.rootObject(), {itChildSub->second}}, [this, sceneBackend] {
			if (valueHandle_) {
				syncChildrenWithValueHandle(sceneBackend);
			}
		});	
	}
}

void PropertyBrowserItem::updateLinkState() noexcept {
	if (valueHandle_) {
		Q_EMIT linkTextChanged(linkText().c_str());
		Q_EMIT linkStateChanged(linkState());
		Q_EMIT editableChanged(editable());
		for (auto* child_ : children_) {
			child_->updateLinkState();
		}
	}
}

core::PrimitiveType PropertyBrowserItem::type() const noexcept {
	return valueHandle_.type();
}

std::string PropertyBrowserItem::luaTypeName() const noexcept {
	if (auto anno = valueHandle_.constValueRef()->query<raco::user_types::EngineTypeAnnotation>()) {
		return commandInterface_->engineInterface().luaNameForPrimitiveType(static_cast<raco::core::EnginePrimitive>(*anno->engineType_));
	}
	return {};
}

PropertyBrowserRef* PropertyBrowserItem::refItem() noexcept {
	return refItem_;
}

PropertyBrowserModel* PropertyBrowserItem::model() const noexcept {
	return model_;
}

const QList<PropertyBrowserItem*>& PropertyBrowserItem::children() {
	return children_;
}

bool PropertyBrowserItem::hasError() const noexcept {
	return commandInterface_->errors().hasError(valueHandle_);
}

void PropertyBrowserItem::markForDeletion() {
	// prevent crashes caused by delayed subscription callbacks
	subscription_ = raco::components::Subscription{};
	errorSubscription_ = raco::components::Subscription{};
	linkValidityChangeSub_ = raco::components::Subscription{};
	linkLifecycleStartSub_ = raco::components::Subscription{};
	linkLifecycleEndSub_ = raco::components::Subscription{};
	changeChildrenSub_ = raco::components::Subscription{};

	for (auto& child : children_) {
		child->markForDeletion();
	}

	this->deleteLater();
}

const core::ErrorItem& PropertyBrowserItem::error() const {
	return commandInterface_->errors().getError(valueHandle_);
}

size_t PropertyBrowserItem::size() noexcept {
	return children_.size();
}

std::string PropertyBrowserItem::displayName() const noexcept {
	if (auto displayNameAnno = query<core::DisplayNameAnnotation>()) {
		return *displayNameAnno->name_;
	}

	auto propName = valueHandle_.getPropName();
	if (propName.empty()) {
		auto* p = reinterpret_cast<PropertyBrowserItem*>(parent());
		auto it = std::find(p->children().begin(), p->children().end(), this);
		return "#" + std::to_string(std::distance(p->children().begin(), it) + LUA_INDEX_OFFSET);
	}
	return propName;
}

core::ValueHandle& PropertyBrowserItem::valueHandle() noexcept {
	return valueHandle_;
}

raco::core::Project* PropertyBrowserItem::project() const {
	return commandInterface_->project();
}

const raco::core::CommandInterface* PropertyBrowserItem::commandInterface() const {
	return commandInterface_;
}

raco::components::SDataChangeDispatcher PropertyBrowserItem::dispatcher() const {
	return dispatcher_;
}

raco::core::EngineInterface& PropertyBrowserItem::engineInterface() const {
	return commandInterface_->engineInterface();
}

bool PropertyBrowserItem::editable() noexcept {
	return !core::Queries::isReadOnly(*commandInterface_->project(), valueHandle());
}

bool PropertyBrowserItem::expandable() const noexcept {
	return valueHandle_.isObject() ||
		   valueHandle_.hasSubstructure() && !query<core::TagContainerAnnotation>() && !query<core::UserTagContainerAnnotation>();
}

bool PropertyBrowserItem::showChildren() const {
	return expandable() && children_.size() > 0 && expanded_;
}

void PropertyBrowserItem::requestNextSiblingFocus() {
	auto children = &parentItem()->children();
	int index = children->indexOf(this);
	if (index >= 0 && index < children->size() - 1) {
		Q_EMIT children->at(index + 1)->widgetRequestFocus();
	}
}

raco::core::Queries::LinkState PropertyBrowserItem::linkState() const noexcept {
	return core::Queries::linkState(*commandInterface_->project(), valueHandle_);
}

void PropertyBrowserItem::setLink(const core::ValueHandle& start, bool isWeak) noexcept {
	commandInterface_->addLink(start, valueHandle_, isWeak);
}

void PropertyBrowserItem::removeLink() noexcept {
	commandInterface_->removeLink(valueHandle_.getDescriptor());
}

std::string PropertyBrowserItem::linkText(bool fullLinkPath) const noexcept {
	if (auto link{raco::core::Queries::getLink(*commandInterface_->project(), valueHandle_.getDescriptor())}) {
		auto propertyDesc = link->startProp();
		auto propertyPath = (fullLinkPath) ? propertyDesc.getFullPropertyPath() : propertyDesc.getPropertyPath();
		if (*link->isWeak_) {
			propertyPath += " (weak)";
		}
		if (!link->isValid()) {
			propertyPath += " (broken)";
		}

		return propertyPath;
	}

	return "";
}

PropertyBrowserItem* PropertyBrowserItem::parentItem() const noexcept {
	return parentItem_;
}

PropertyBrowserItem* PropertyBrowserItem::siblingItem(std::string_view propertyName) const noexcept {
	if (!parentItem()) {
		return nullptr;
	}
	for (auto* sibling : parentItem()->children()) {
		if (sibling->valueHandle().getPropName() == propertyName) {
			return sibling;
		}
	}
	return nullptr;
}

bool PropertyBrowserItem::isRoot() const noexcept {
	return parentItem() == nullptr;
}

bool PropertyBrowserItem::hasCollapsedParent() const noexcept {
	if (isRoot()) {
		return false;
	} else {
		return !parentItem()->expanded() || parentItem()->hasCollapsedParent();
	}
}

PropertyBrowserItem* PropertyBrowserItem::findItemWithNoCollapsedParentInHierarchy() noexcept {
	PropertyBrowserItem* current = this;
	while (current->hasCollapsedParent()) {
		current = current->parentItem();
	}
	return current;
}

bool PropertyBrowserItem::expanded() const noexcept {
	return expanded_;
}

void PropertyBrowserItem::setExpanded(bool expanded) noexcept {
	assert(expandable());

	if (expanded_ != expanded) {
		expanded_ = expanded;

		Q_EMIT expandedChanged(expanded_);
		Q_EMIT showChildrenChanged(showChildren());
	}
}

void PropertyBrowserItem::setExpandedRecursively(bool expanded) noexcept {
	if (expandable()) {
		setExpanded(expanded);

		for (const auto& child : children_) {
			child->setExpandedRecursively(expanded);
		}
	}
}

void PropertyBrowserItem::setTags(std::vector<std::string> const& tags) {
	commandInterface_->setTags(valueHandle_, tags);
}

void PropertyBrowserItem::setTags(std::vector<std::pair<std::string, int>> const& prioritizedTags) {
	commandInterface_->setRenderableTags(valueHandle_, prioritizedTags);
}

void PropertyBrowserItem::createChildren(core::SceneBackendInterface* sceneBackend) {
	children_.reserve(static_cast<int>(valueHandle_.size()));

	for (int i{0}; i < valueHandle_.size(); i++) {
		bool hidden = raco::core::Queries::isHiddenInPropertyBrowser(*project(), valueHandle_[i]);

		// The render passes flags for clearing the target can only be used for offscreen rendering.
		// For the default framebuffer, the settings are in the project settings.
		// Given that this is a dynamic setting, do it here explicitly and not in Queries::isHidden for now.
		if (const auto& renderPass = valueHandle_.rootObject()->as<user_types::RenderPass>(); renderPass != nullptr && renderPass->target_.asRef() == nullptr && renderPass->isClearTargetProperty(valueHandle_[i])) {
			hidden = true;
		} else if (const auto& renderBuffer = valueHandle_.rootObject()->as<user_types::RenderBuffer>(); renderBuffer != nullptr && !renderBuffer->areSamplingParametersSupported(engineInterface()) && renderBuffer->isSamplingProperty(valueHandle_[i])) {
			hidden = true;
		}

		if (!hidden) {
			children_.push_back(new PropertyBrowserItem(valueHandle_[i], dispatcher_, commandInterface_, sceneBackend, model_, this));
		}
	}
}

void PropertyBrowserItem::syncChildrenWithValueHandle(core::SceneBackendInterface* sceneBackend) {
	// clear children
	{
		for (auto& child : children_) {
			child->markForDeletion();
		}
		children_.clear();
	}

	// create new children
	createChildren(sceneBackend);

	Q_EMIT childrenChanged(children_);

	// notify first not collapsed item about the structural changed
	{
		auto* visibleItem = findItemWithNoCollapsedParentInHierarchy();
		assert(visibleItem != nullptr);
		Q_EMIT visibleItem->childrenChangedOrCollapsedChildChanged();
	}

	// notify the view state accordingly
	if (children_.size() <= 1) {
		Q_EMIT expandedChanged(expanded());
		Q_EMIT showChildrenChanged(showChildren());
	}
}

bool PropertyBrowserItem::canBeChosenByColorPicker() const {

	if(valueHandle_.isObject() || !(valueHandle_.isVec3f() || valueHandle_.isVec4f())) {		
		return false;
	}

	const auto rootTypeRef = &valueHandle_.rootObject()->getTypeDescription();
	
	if (!(rootTypeRef == &raco::user_types::ProjectSettings::typeDescription || 
		rootTypeRef == &raco::user_types::LuaScript::typeDescription || 
		rootTypeRef == &raco::user_types::LuaInterface::typeDescription ||
		rootTypeRef == &raco::user_types::MeshNode::typeDescription || 
		rootTypeRef == &raco::user_types::Material::typeDescription)) {
		return false;	
	}

	if(valueHandle_.isRefToProp(&raco::user_types::Node::translation_) || valueHandle_.isRefToProp(&raco::user_types::Node::rotation_) || valueHandle_.isRefToProp(&raco::user_types::Node::scaling_)) {
		return false;
	}
	
	return true;
}

bool PropertyBrowserItem::getDefaultExpandedFromValueHandleType() const {
	if (valueHandle_.isObject()) {
		return true;
	}

	const auto rootTypeRef = &valueHandle_.rootObject()->getTypeDescription();
	
	if (rootTypeRef == &raco::user_types::MeshNode::typeDescription || rootTypeRef == &raco::user_types::Material::typeDescription) {
		auto parent = valueHandle_.parent();

		if (parent.isProperty()) {
			auto parentPropName = parent.getPropName();
			bool isOptionsOrUniformGroup = parentPropName == "options" || parentPropName == "uniforms";
			return !isOptionsOrUniformGroup;
		}
	} else if (rootTypeRef == &raco::user_types::LuaScript::typeDescription || rootTypeRef == &raco::user_types::LuaInterface::typeDescription) {
		auto parent = valueHandle_.parent();

		bool isTopLevelLuaValueGroup = parent.isObject() &&
									   (valueHandle_.isRefToProp(&raco::user_types::LuaScript::inputs_) || valueHandle_.isRefToProp(&raco::user_types::LuaInterface::inputs_)
									  || valueHandle_.isRefToProp(&raco::user_types::LuaScript::outputs_)
									  || valueHandle_.isRefToProp(&raco::user_types::LuaScript::luaModules_));

		bool isFirstLevelChildOfInputOutputGroup = parent.isProperty() && 
											       parent.parent().isObject() &&
												   (parent.isRefToProp(&raco::user_types::LuaScript::inputs_) || parent.isRefToProp(&raco::user_types::LuaInterface::inputs_) 
												  || parent.isRefToProp(&raco::user_types::LuaScript::outputs_));


		bool isTableOrStruct = valueHandle_.type() == PrimitiveType::Table || valueHandle_.type() == PrimitiveType::Struct;

		return isTopLevelLuaValueGroup || (isFirstLevelChildOfInputOutputGroup && isTableOrStruct);
	}

	return true;
}

std::string to_string(PropertyBrowserItem& item) {
	std::stringstream ss{};
	ss << "PropertyBrowserItem { type: ";
	ss << static_cast<int>(item.valueHandle_.type());
	ss << ", displayName: \"";
	ss << item.displayName();
	ss << "\"";
	if (!hasTypeSubstructure(item.valueHandle_.type())) {
		ss << ", value: ";
		switch (item.valueHandle_.type()) {
			case core::PrimitiveType::Double:
				ss << std::to_string(item.valueHandle_.as<double>());
				break;
		}
	}
	if (item.children_.size() > 0) {
		ss << ", children: [ ";
		for (auto& child : item.children_) {
			ss << to_string(*child);
		}
		ss << " ]";
	}
	ss << " }";
	return ss.str();
}

}  // namespace raco::property_browser
