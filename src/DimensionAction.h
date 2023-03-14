#pragma once

#include "actions/GroupAction.h"
#include "actions/ToggleAction.h"
#include "actions/DecimalAction.h"

#include <DimensionPickerAction.h>

using namespace hdps::gui;

class RendererSettingsAction;
class Flow4DViewerPlugin;
class ViewerWidget;

/**
 * Slicing action class
 *
 * Action class for choosing dimensions
 *
 * @author Thomas Kroes and Mitchell M. de Boer
 */
class DimensionAction : public GroupAction
{
    Q_OBJECT

public:
    /**
     * Constructor
     * @param rendererSettingsAction Reference to renderer settings action
     */
    DimensionAction(RendererSettingsAction& rendererSettingsAction, ViewerWidget* viewerWidet);

public: /** Action getters */

    DimensionPickerAction& getDimensionPickerAction() { return _dimensionAction; }

protected:
    RendererSettingsAction& _rendererSettingsAction;        /** Reference to renderer settings action */
    ViewerWidget* _viewerWidget;                            /** Pointer to the viewerWidget*/
    
    DimensionPickerAction       _dimensionAction;           /** Action to pick the current dimension*/
    
};