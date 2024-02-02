#include "DimensionAction.h"
#include "RendererSettingsAction.h"
#include "ViewerWidget.h"
#include "Flow4DViewerPlugin.h"
#include <QtCore>
#include <QtDebug>
#include <QFileDialog>
#include <qmessagebox.h>

using namespace mv;

DimensionAction::DimensionAction(RendererSettingsAction& rendererSettingsAction, ViewerWidget* viewerWidget, const QString& title) :
    GroupAction(reinterpret_cast<QObject*>(&rendererSettingsAction), title),
    _rendererSettingsAction(rendererSettingsAction),

    _viewerWidget(nullptr),

    _selectedDataAction(this, "Select data", { "Flow Speed","Line Index", "Time Interval", "Flow Group"}, "Flow Group")

{
    setText("Data value selecter");
    addAction(&_selectedDataAction);
    _viewerWidget = viewerWidget;

    
}