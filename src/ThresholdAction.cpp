#include "ThresholdAction.h"
#include "SelectedPointsAction.h"

#include <QHBoxLayout>

using namespace mv;

ThresholdAction::ThresholdAction(SelectedPointsAction& SelectedPointsAction, const QString& title) :
    WidgetAction(reinterpret_cast<QObject*>(&SelectedPointsAction), title),
    _selectedPointsAction(SelectedPointsAction),
    _lowerAction(this, "Lower Threshold", -100000.0f, 100000.0f, 0.0f, 0.0f),
    _upperAction(this, "UpperThreshold", -100000.0f, 100000.0f, 0.0f, 0.0f)
    //_thresholdAction(this, "value deviation %", 0, 1,0,1,0,1)
    
{
    setText("upper/lower thresholds");
    

    // Configure position widgets
    _lowerAction.setDefaultWidgetFlags(DecimalAction::SpinBox);
    _upperAction.setDefaultWidgetFlags(DecimalAction::SpinBox);
    

    const auto notifyChanged = [this]() -> void {
        emit changed();
    };

    connect(&_lowerAction, &DecimalAction::valueChanged, this, notifyChanged);
    connect(&_upperAction, &DecimalAction::valueChanged, this, notifyChanged);
    
}

QWidget* ThresholdAction::getWidget(QWidget* parent, const std::int32_t& widgetFlags)
{
    auto widget = new WidgetActionWidget(parent, this);
    auto layout = new QHBoxLayout();

    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(3);

    layout->addWidget(_lowerAction.createWidget(widget));
    layout->addWidget(_upperAction.createWidget(widget));
    

    widget->setLayout(layout);

    return widget;
}