#pragma once

#include "actions/GroupAction.h"
#include "actions/ColorMap1DAction.h"
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
    ColoringAction(RendererSettingsAction& rendererSettingsAction, const QString& title);
   
public: /** Action getters */

    
    ColorMap1DAction& getColorMapAction() { return  _colorMapAction; }
    
    


protected:
    RendererSettingsAction&     _rendererSettingsAction;        /** Reference to renderer settings action */
   
    ColorMap1DAction              _colorMapAction;                 /** Color map Action */
    
    
};