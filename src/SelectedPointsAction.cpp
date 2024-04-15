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
    
    _xyzToggled(this, "positions", false),
    _xyzDevToggled(this, "vector", false),
    _speedToggled(this, "speed", false),
    _timeToggled(this, "time", false),
    _constructSelection(this, "Create Selection"),

    _thresholdAction(this, "select timepoints", { 0,153 }, { 0,153 },0)


    //decimal range action
    
{  
    setText("Selected data options");
    addAction(&_xyzToggled);
    addAction(&_xyzDevToggled);
    addAction(&_speedToggled);
    addAction(&_timeToggled);
    addAction(&_constructSelection);

    addAction(&_thresholdAction);
}