#include "ColoringAction.h"
#include <QtCore>
#include <QtDebug>
#include <QFileDialog>
#include <qmessagebox.h>
#include "RendererSettingsAction.h"
#include <QGraphicsScene>

using namespace mv;

ColoringAction::ColoringAction(RendererSettingsAction& rendererSettingsAction, const QString& title) :
    GroupAction(reinterpret_cast<QObject*>(&rendererSettingsAction), title),
    _rendererSettingsAction(rendererSettingsAction),
    
    _toggleColorSelection(this, "SelectionColor", false),
    
    // colormap options 
    _colorMapAction(this, "Transfer Function")
    

{
    setText("Coloring parameters");
    addAction(&_colorMapAction);
    addAction(&_toggleColorSelection);

}