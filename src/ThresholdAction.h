#pragma once

#include <actions/WidgetAction.h>
#include <actions/DecimalAction.h>
#include <actions/DecimalRangeAction.h>


using namespace hdps::gui;

class SelectedPointsAction;

/**
 * Position action class
 *
 * Action class for configurin layer position
 *
 * @author Thomas Kroes
 */
class ThresholdAction : public WidgetAction
{
    Q_OBJECT

protected:

    /**
     * Get widget representation of the position action
     * @param parent Pointer to parent widget
     * @param widgetFlags Widget flags for the configuration of the widget (type)
     */
    QWidget* getWidget(QWidget* parent, const std::int32_t& widgetFlags) override;

public:

    /** 
     * Constructor
     * @param generalAction Reference to general action
     */
    ThresholdAction(SelectedPointsAction& selectedPointsAction, const QString& title);

signals:

    /** Signals that the position changed */
    void changed();

public: /** Action getters */

    DecimalAction& getLowerThresholdAction() { return _lowerAction; }
    DecimalAction& getUpperThresholdAction() { return _upperAction; }
    

protected:
    SelectedPointsAction& _selectedPointsAction;     /** Reference to general action */
    DecimalAction   _lowerAction;           /** X-position action */
    DecimalAction   _upperAction;           /** Y-position action */
    //DecimalRangeAction _thresholdAction;
};
