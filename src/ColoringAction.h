#pragma once

#include "actions/GroupAction.h"
#include "actions/ColorMapAction.h"
#include "actions/OptionAction.h"

using namespace hdps::gui;

class RendererSettingsAction;

/**
 * Coloring action class
 *
 * Action class for coloring parameters
 *
 * @author Thomas Kroes and Mitchell M. de Boer
 */
class ColoringAction : public GroupAction
{
    Q_OBJECT

public:

    /**
     * Constructor
     * @param rendererSettingsAction Reference to renderer settings action
     */
    ColoringAction(RendererSettingsAction& rendererSettingsAction);
   

public: /** Action getters */

    OptionAction& getInterpolationAction() { return  _colorInterpolationAction; }
    ColorMapAction& getColorMapAction() { return  _colorMapAction; }


protected:
    RendererSettingsAction&     _rendererSettingsAction;        /** Reference to renderer settings action */
    OptionAction                _colorInterpolationAction;      /** Option menu for selecting interpolation mode*/
    ColorMapAction              _colorMapAction;                 /** Color map Action */
};