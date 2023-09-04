#include "RendererSettingsAction.h"
#include <ViewerWidget.h>

using namespace hdps;

RendererSettingsAction::RendererSettingsAction(QObject* parent, ViewerWidget* viewerWidget, const QString& title) :
    GroupsAction(parent, title),
    _dimensionAction(*this, viewerWidget, title),
    _slicingAction(*this, viewerWidget, title),
    _coloringAction(*this, title),
    _selectedPointsAction(*this, title)
{
    addGroupAction(&_dimensionAction);
    addGroupAction(&_slicingAction);
    addGroupAction(&_coloringAction);
    addGroupAction(&_selectedPointsAction);
}