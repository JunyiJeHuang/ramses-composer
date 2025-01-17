#include "property_browser_extend/PropertyBrowserCurveBindingWidget.h"

namespace raco::property_browser {
PropertyBrowserCurveBindingWidget::PropertyBrowserCurveBindingWidget(core::CommandInterface *commandInterface, QWidget *parent)
    : QWidget{parent},
    commandInterface_(commandInterface) {
    QWidget* mainWidget = new QWidget(this);
    layout_ = new QGridLayout(this);
    layout_->setSizeConstraint(QVBoxLayout::SetMinAndMaxSize);
    mainWidget->setLayout(layout_);

    QVBoxLayout* vBoxLayout = new QVBoxLayout(this);
	vBoxLayout->setMargin(0);
    scroll_ = new QScrollArea(this);
    scroll_->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));
    scroll_->setWidgetResizable(true);
    scroll_->setWidget(mainWidget);
    vBoxLayout->addWidget(scroll_);
    this->setLayout(vBoxLayout);
}

void PropertyBrowserCurveBindingWidget::initCurveBindingWidget() {
    clearCurveBinding();
    std::string sampleProperty = animationDataManager::GetInstance().getActiveAnimationName();
    if (sampleProperty == std::string()) {
        return;
    }
    std::map<std::string, std::string> bindingDataMap;
	NodeData* nodeData = NodeDataManager::GetInstance().getActiveNode();
	if (nodeData) {
		nodeData->NodeExtendRef().curveBindingRef().getPropCurve(sampleProperty, bindingDataMap);
    }
    
    for (const auto &it : bindingDataMap) {
        insertCurveBinding(QString::fromStdString(sampleProperty), QString::fromStdString(it.first), QString::fromStdString(it.second));
    }
    Q_EMIT signalProxy::GetInstance().sigRepaintTimeAxis_From_NodeUI();
}

void PropertyBrowserCurveBindingWidget::insertCurveBinding(QString sampleProperty, QString property, QString curve) {

    PropertyBrowserCurveBindingView* widget = new PropertyBrowserCurveBindingView(commandInterface_, sampleProperty, property, curve, this);
    layout_->addWidget(widget, listWidget_.size(), 0);
    listWidget_.append(widget);
}

void PropertyBrowserCurveBindingWidget::clearCurveBinding() {
    for (QWidget* widget : qAsConst(listWidget_)) {
        layout_->removeWidget(widget);
		if (widget != nullptr) {
		    delete widget;
            widget = nullptr;
        }
    }
    listWidget_.clear();
}

void PropertyBrowserCurveBindingWidget::insertData() {
    if (listWidget_.size() < 0) {
        return;
    }

    std::string stdStrSampleProperty = animationDataManager::GetInstance().getActiveAnimationName();
    QString sampleProperty = QString::fromStdString(stdStrSampleProperty);
    if (stdStrSampleProperty == std::string()) {
        return;
    }
    QString curve = "Curve" + QString::number(index_);
    QString property = "Property" + QString::number(index_);
    while (!isInValidBindingData(sampleProperty, property, curve)) {
        index_++;
        curve = "Curve" + QString::number(index_);
        property = "Property" + QString::number(index_);
    }
    index_++;
    insertCurveBinding(sampleProperty, property, curve);
    NodeDataManager::GetInstance().getActiveNode()->NodeExtendRef().curveBindingRef().insertBindingDataItem(sampleProperty.toStdString(), property.toStdString(), curve.toStdString());
    Q_EMIT signalProxy::GetInstance().sigRepaintTimeAxis_From_NodeUI();

    raco::core::UndoState undoState;
    undoState.saveCurrentUndoState();
    std::string description = fmt::format("insert curveBinding to '{}'", NodeDataManager::GetInstance().getActiveNode()->getName());
    commandInterface_->undoStack().push(description, undoState);
}

void PropertyBrowserCurveBindingWidget::removeData() {
    std::string stdStrSampleProperty = animationDataManager::GetInstance().getActiveAnimationName();
    if (stdStrSampleProperty == std::string()) {
        return;
    }
    std::string property;
    for (QWidget* widget : qAsConst(listWidget_)) {
		if (widget != nullptr) {
			PropertyBrowserCurveBindingView* view = dynamic_cast<PropertyBrowserCurveBindingView*>(widget);
            if (view->isSelected()) {
                property = view->property();
				view->deleteCurveBinding();
				layout_->removeWidget(widget);
                listWidget_.removeOne(widget);
				delete widget;
                widget = nullptr;
			}
		}
    }
    Q_EMIT signalProxy::GetInstance().sigRepaintTimeAxis_From_NodeUI();

    raco::core::UndoState undoState;
    undoState.saveCurrentUndoState();
    std::string description = fmt::format("delete '{}' property of curveBinding from '{}'", property, NodeDataManager::GetInstance().getActiveNode()->getName());
    commandInterface_->undoStack().push(description, undoState);
}

bool PropertyBrowserCurveBindingWidget::isInValidBindingData(QString sampleProperty, QString property, QString curve) {
    std::map<std::string, std::string> bindingDataMap;
    NodeDataManager::GetInstance().getActiveNode()->NodeExtendRef().curveBindingRef().getPropCurve(sampleProperty.toStdString(), bindingDataMap);

    auto it = bindingDataMap.find(property.toStdString());
    if (it != bindingDataMap.end()) {
        return false;
    }

    for (const auto &bindingIt : bindingDataMap) {
        if (bindingIt.second.compare(curve.toStdString()) == 0) {
            return false;
        }
    }
    return true;
}

}
