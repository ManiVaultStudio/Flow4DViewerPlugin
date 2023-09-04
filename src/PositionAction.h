#pragma once

#include <actions/WidgetAction.h>
#include <actions/DecimalAction.h>

using namespace hdps::gui;

class SelectedPointsAction;

/**
 * Position action class
 *
 * Action class for configurin layer position
 *
 * @author Thomas Kroes
 */
class PositionAction : public WidgetAction
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
    PositionAction(SelectedPointsAction& selectedPointsAction, const QString& title);
    void changeValue(int *xyz);

signals:

    /** Signals that the position changed */
    void changed();

public: /** Action getters */

    DecimalAction& getXAction() { return _xAction; }
    DecimalAction& getYAction() { return _yAction; }
    DecimalAction& getZAction() { return _yAction; }

protected:
    SelectedPointsAction& _selectedPointsAction;     /** Reference to general action */
    DecimalAction   _xAction;           /** X-position action */
    DecimalAction   _yAction;           /** Y-position action */
    DecimalAction   _zAction;           /** z-position action */
};
