#pragma once

#include "actions/GroupAction.h"
#include "actions/ColorMapAction.h"
#include "actions/OptionAction.h"
#include "PositionAction.h"
#include "ThresholdAction.h"
#include <actions/DecimalRangeAction.h>


using namespace mv::gui;

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
    /*TriggerAction& getSelectPointAction() { return  _selectPointAction; }
    TriggerAction& getSelectPointActionSpeed() { return  _selectPointActionSpeed; }
    TriggerAction& getSelectPointActionTime() { return  _selectPointActionTime; }
    TriggerAction& getSelectPointActionSpeedTime() { return  _selectPointActionSpeedTime; }
    TriggerAction& getSelectPointActionDerivative() { return  _selectPointActionDerivative; }
    TriggerAction& getSelectPointActionDerivativeTime() { return  _selectPointActionDerivativeTime; }*/
    ToggleAction& getXyzToggled() { return _xyzToggled; }
    ToggleAction& getXyzDevToggled() { return _xyzDevToggled; }
    ToggleAction& getSpeedToggled() { return _speedToggled; }
    ToggleAction& getTimeToggled() { return _timeToggled; }
    TriggerAction& getConstructSelection() { return _constructSelection; }
    
   
    DecimalRangeAction& getDecimalRangeAction() { return _thresholdAction; }
    

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
    //TriggerAction                   _selectPointAction;                 /** Activate point selection*/
    //TriggerAction                   _selectPointActionSpeed;                 /** Activate point selection*/
    //TriggerAction                   _selectPointActionTime;                 /** Activate point selection*/
    //TriggerAction                   _selectPointActionSpeedTime;                 /** Activate point selection*/
    //TriggerAction                   _selectPointActionDerivative;                 /** Activate point selection*/
    //TriggerAction                   _selectPointActionDerivativeTime;                 /** Activate point selection*/
    ToggleAction                    _xyzToggled;
    ToggleAction                    _xyzDevToggled;
    ToggleAction                    _speedToggled;
    ToggleAction                    _timeToggled;
    TriggerAction                   _constructSelection;
    //PositionAction                  _positionAction;
    
    DecimalRangeAction              _thresholdAction;

};