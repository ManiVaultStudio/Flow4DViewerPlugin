#pragma once

#include "actions/GroupAction.h"
#include "actions/ColorMapAction.h"
#include "actions/OptionAction.h"
#include "PositionAction.h"
#include "ThresholdAction.h"



using namespace hdps::gui;

class RendererSettingsAction;

/**
 * Coloring action class
 *
 * Action class for coloring parameters
 *
 * @author Thomas Kroes and Mitchell M. de Boer
 */
class SelectedPointsAction : public GroupAction
{
    Q_OBJECT

public:

    /**
     * Constructor
     * @param rendererSettingsAction Reference to renderer settings action
     */
    SelectedPointsAction(RendererSettingsAction& rendererSettingsAction, const QString& title);
   
public: /** Action getters */

    //OptionAction& getBackgroundShowAction() { return  _backgroundShowAction; }
    ////ColorMapAction& getColorMapAction() { return  _colorMapAction; }
    ////ToggleAction& getShadingAction() { return  _shadingEnableAction; }
    ////
    //DecimalAction& getBackgroundAlphaAction() { return _backgroundAlphaAction; }
    //OptionAction& getSelectionAlphaAction() { return  _selectionAlphaAction; }
    //PositionAction& getPositionAction() { return _positionAction; }
    TriggerAction& getSelectPointAction() { return  _selectPointAction; }
    ThresholdAction& getThresholdAction() { return _thresholdAction; }

    

    //DecimalAction& getDiffuseAction() { return _diffuseConstantAction; }
    //DecimalAction& getSpecularAction() { return _specularConstantAction; }


protected:
    RendererSettingsAction&     _rendererSettingsAction;        /** Reference to renderer settings action */
    //OptionAction                _backgroundShowAction;      /** Option menu for selecting interpolation mode*/
    //ColorMapAction              _colorMapAction;                 /** Color map Action */
    //ToggleAction                _shadingEnableAction;      /** Option menu for selecting interpolation mode*/
    //
    //DecimalAction                _backgroundAlphaAction;               /** Input box for ambient color constant.*/
    //OptionAction                    _selectionAlphaAction;
    //DecimalAction                _diffuseConstantAction;               /** Input box for diffuse color constant.*/
    //DecimalAction                _specularConstantAction;               /** Input box for specular color constant.*/
    TriggerAction                   _selectPointAction;                 /** Activate point selection*/
    //PositionAction                  _positionAction;
    ThresholdAction                 _thresholdAction;

};