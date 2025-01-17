#include "common_editors/Int64Editor.h"
#include "qdebug.h"

namespace raco::common_editors {
Int64Editor::Int64Editor(QWidget *parent)
    : QWidget{parent} {
    auto* layout = new PropertyBrowserGridLayout{this};
    stack_ = new QStackedWidget{this};

    spinBox_ = new Int64SpinBox{stack_};
    using raco::property_browser::Int64Slider;
    slider_ = new Int64Slider{stack_};

    slider_->setValue(0);
    spinBox_->setValue(0);

    // connect everything to our item values
    {
        QObject::connect(spinBox_, &Int64SpinBox::valueEdited, this, [this](int value) {
            if (value < min_) {
                spinBox_->setValue(min_);
                slider_->setValue(min_);
                return;
            }
            if (value > max_) {
                spinBox_->setValue(max_);
                slider_->setValue(max_);
                return;
            }
            spinBox_->setValue(value);
            slider_->setValue(value);
            Q_EMIT sigValueChanged(value);
        });
        QObject::connect(slider_, &Int64Slider::valueEdited, this, [this](int value) {
            if (value < min_) {
                spinBox_->setValue(min_);
                slider_->setValue(min_);
                return;
            }
            if (value > max_) {
                spinBox_->setValue(max_);
                slider_->setValue(max_);
                return;
            }
            spinBox_->setValue(value);
            Q_EMIT sigValueChanged(value);
        });
    }

    // State change: Show spinbox or slider
    QObject::connect(slider_, &Int64Slider::singleClicked, this, [this]() { stack_->setCurrentWidget(spinBox_); });
    QObject::connect(slider_, &Int64Slider::finished, this, [this]() {
        int value = spinBox_->value();
        if (value < min_) {
            spinBox_->setValue(min_);
            slider_->setValue(min_);
            return;
        }
        if (value > max_) {
            spinBox_->setValue(max_);
            slider_->setValue(max_);
            return;
        }
        Q_EMIT sigEditingFinished(); });
    QObject::connect(spinBox_, &Int64SpinBox::editingFinished, this, [this]() {
        stack_->setCurrentWidget(slider_);
        slider_->clearFocus();
        Q_EMIT sigEditingFinished();
    });

    stack_->addWidget(slider_);
    stack_->addWidget(spinBox_);

    stack_->setCurrentWidget(slider_);
    layout->addWidget(stack_);
    setHeight(20);
}

void Int64Editor::setRange(int min, int max) {
    min_ = min;
    max_ = max;
    spinBox_->setSoftRange(min, max);
    slider_->setSoftRange(min, max);
}

void Int64Editor::setValue(int value) {
    slider_->setValue(value);
    spinBox_->setValue(value);
}

void Int64Editor::setSize(int width, int height) {
    spinBox_->setFixedSize(width, height);
    stack_->setFixedSize(width, height);
}

void Int64Editor::setWidth(int width) {
    spinBox_->setFixedWidth(width);
    stack_->setFixedWidth(width);
}

void Int64Editor::setHeight(int height) {
    spinBox_->setFixedHeight(height);
    stack_->setFixedHeight(height);
}

int Int64Editor::value() {
    return spinBox_->value();
}
}
