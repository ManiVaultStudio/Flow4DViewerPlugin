#include "SlicingAction.h"
#include "RendererSettingsAction.h"
#include "ViewerWidget.h"
#include "Flow4DViewerPlugin.h"
#include <QtCore>
#include <QtDebug>
#include <QFileDialog>
#include <qmessagebox.h>

using namespace hdps;

SlicingAction::SlicingAction(RendererSettingsAction& rendererSettingsAction,ViewerWidget* viewerWidget) :
    GroupAction(reinterpret_cast<QObject*>(&rendererSettingsAction)),
    _rendererSettingsAction(rendererSettingsAction),
    _xAxisEnabledAction(this, "x-axis enabled"),
    _xAxisPositionAction(this, "x-axis position", 0.0f, 100.0f, 0.0f, 0.0f, 0),
    _yAxisEnabledAction(this, "y-axis enabled"),
    _yAxisPositionAction(this, "y-axis position", 0.0f, 100.0f, 0.0f, 0.0f, 0),
    _zAxisEnabledAction(this, "z-axis enabled"),
    _zAxisPositionAction(this, "z-axis position", 0.0f, 100.0f, 0.0f, 0.0f, 0)
{
    setText("Slicing parameters");

    // Disable sliders by default due to disabled toggles
    _xAxisPositionAction.setDisabled(true);
    _yAxisPositionAction.setDisabled(true);
    _zAxisPositionAction.setDisabled(true);

    
    _xToggled = false; 
    _yToggled = false;
    _zToggled = false;
    
    connect(&_xAxisEnabledAction, &ToggleAction::toggled, this, [this](bool toggled) {        
        _xToggled = toggled;
       
    });

    connect(&_yAxisEnabledAction, &ToggleAction::toggled, this, [this](bool toggled) {       
        _yToggled = toggled;
        
    });

    
    connect(&_zAxisEnabledAction, &ToggleAction::toggled, this, [this](bool toggled) {        
        _zToggled = toggled;
             
    });
}