#include "PositionAction.h"
#include "SelectedPointsAction.h"

#include <QHBoxLayout>


using namespace hdps;

PositionAction::PositionAction(SelectedPointsAction& SelectedPointsAction, const QString& title) :
    WidgetAction(reinterpret_cast<QObject*>(&SelectedPointsAction), title),
    _selectedPointsAction(SelectedPointsAction),
    _xAction(this, "X position", -100000.0f, 100000.0f, 0.0f, 0.0f),
    _yAction(this, "Y position", -100000.0f, 100000.0f, 0.0f, 0.0f),
    _zAction(this, "Y position", -100000.0f, 100000.0f, 0.0f, 0.0f)
{
    setText("Position");
    

    // Configure position widgets
    _xAction.setDefaultWidgetFlags(DecimalAction::SpinBox);
    _yAction.setDefaultWidgetFlags(DecimalAction::SpinBox);
    _zAction.setDefaultWidgetFlags(DecimalAction::SpinBox);

    const auto notifyChanged = [this]() -> void {
        emit changed();
    };

    connect(&_xAction, &DecimalAction::valueChanged, this, notifyChanged);
    connect(&_yAction, &DecimalAction::valueChanged, this, notifyChanged);
    connect(&_zAction, &DecimalAction::valueChanged, this, notifyChanged);
}

QWidget* PositionAction::getWidget(QWidget* parent, const std::int32_t& widgetFlags)
{
    auto widget = new WidgetActionWidget(parent, this);
    auto layout = new QHBoxLayout();

    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(3);

    layout->addWidget(_xAction.createWidget(widget));
    layout->addWidget(_yAction.createWidget(widget));
    layout->addWidget(_zAction.createWidget(widget));

    widget->setLayout(layout);

    return widget;
}

void PositionAction::changeValue(int *xyz) {
    _xAction.setValue(xyz[0]);
    _yAction.setValue(xyz[1]);
    _zAction.setValue(xyz[2]);
}