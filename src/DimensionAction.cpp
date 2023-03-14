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
    // Action to change the current dimension
    _dimensionAction(this, "Data dimension")
{
    setText("Dimension parameters");
    
    _viewerWidget = viewerWidget;

    
}