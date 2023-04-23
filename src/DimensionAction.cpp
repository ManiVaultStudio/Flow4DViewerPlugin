#include "DimensionAction.h"
#include "RendererSettingsAction.h"
#include "ViewerWidget.h"
#include "Flow4DViewerPlugin.h"
#include <QtCore>
#include <QtDebug>
#include <QFileDialog>
#include <qmessagebox.h>

using namespace hdps;

DimensionAction::DimensionAction(RendererSettingsAction& rendererSettingsAction, ViewerWidget* viewerWidget) :
    GroupAction(reinterpret_cast<QObject*>(&rendererSettingsAction)),
    _rendererSettingsAction(rendererSettingsAction),

    _viewerWidget(nullptr),
    _selectedDataAction(this, "Select data", { "Flow Speed","Line Index", "Time Interval"}, "Flow Speed", "Flow Speed")
    
{
    setText("Data value selecter");
    
    _viewerWidget = viewerWidget;

    
}