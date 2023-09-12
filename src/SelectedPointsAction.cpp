#include "SelectedPointsAction.h"
#include <QtCore>
#include <QtDebug>
#include <QFileDialog>
#include <qmessagebox.h>
#include "RendererSettingsAction.h"
#include <QGraphicsScene>
#include <PositionAction.h>
#include <ThresholdAction.h>
#include <actions/DecimalRangeAction.h>

using namespace hdps;

SelectedPointsAction::SelectedPointsAction(RendererSettingsAction& rendererSettingsAction) :
    GroupAction(reinterpret_cast<QObject*>(&rendererSettingsAction)),
    _rendererSettingsAction(rendererSettingsAction),
    
    // color interpolation options with default nearest neighbor interpolations
    //_backgroundShowAction(this, "Surrounding data", {"Show background","Hide background"}, "Show background", "Show background"),
    //_backgroundAlphaAction(this, "Background alpha", 0.0f, 1.0f, 0.02f, 0.02f, 3),
    //_selectionAlphaAction(this, "Selection alpha", {"Opaque","Use transfer function"}, "Opaque", "Opaque"),
    _selectPointAction(this, "export xyz"),
    _selectPointActionSpeed(this, "export xyzspeed"),
    _selectPointActionTime(this, "export xyztime"),
    _selectPointActionSpeedTime(this, "export xyzspeedtime"),
    //_positionAction(*this),
    
    _thresholdActionTest(this,"test",{0,153},{0,153},0)


    //decimal range action
    
{
    
    setText("Selected data options");
}