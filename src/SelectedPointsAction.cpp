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

using namespace mv;

SelectedPointsAction::SelectedPointsAction(RendererSettingsAction& rendererSettingsAction, const QString& title) :
    GroupAction(reinterpret_cast<QObject*>(&rendererSettingsAction), title),
    _rendererSettingsAction(rendererSettingsAction),
    
    // color interpolation options with default nearest neighbor interpolations
    //_backgroundShowAction(this, "Surrounding data", {"Show background","Hide background"}, "Show background", "Show background"),
    //_backgroundAlphaAction(this, "Background alpha", 0.0f, 1.0f, 0.02f, 0.02f, 3),
    //_selectionAlphaAction(this, "Selection alpha", {"Opaque","Use transfer function"}, "Opaque", "Opaque"),
    /*_selectPointAction(this, "export xyz"),
    _selectPointActionSpeed(this, "export xyzspeed"),
    _selectPointActionTime(this, "export xyztime"),
    _selectPointActionSpeedTime(this, "export xyzspeedtime"),
    _selectPointActionDerivative(this, "export vector"),
    _selectPointActionDerivativeTime(this,"export vectortime"),*/
    //_selectPointActionDerivativePositie(this,"export vectortime"),
    //_positionAction(*this),
    _xyzToggled(this, "positions", false),
    _xyzDevToggled(this, "vector", false),
    _speedToggled(this, "speed", false),
    _timeToggled(this, "time", false),
    _constructSelection(this, "Create Selection"),

    _thresholdAction(this, "select timepoints", { 0,153 }, { 0,153 },0)


    //decimal range action
    
{
    
    setText("Selected data options");
    /*addAction(&_selectPointAction);
    addAction(&_selectPointActionSpeed);
    addAction(&_selectPointActionTime);
    addAction(&_selectPointActionSpeedTime);
    addAction(&_selectPointActionDerivative);
    addAction(&_selectPointActionDerivativeTime);
    addAction(&_selectPointActionDerivativeTime);*/

    addAction(&_xyzToggled);
    addAction(&_xyzDevToggled);
    addAction(&_speedToggled);
    addAction(&_timeToggled);
    addAction(&_constructSelection);

    addAction(&_thresholdAction);
}