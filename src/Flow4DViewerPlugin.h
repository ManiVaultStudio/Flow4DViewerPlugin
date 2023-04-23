#pragma once
/** QT headers*/
#include <QItemSelectionModel>
#include <QWidget>
#include <QLayout>
/** Plugin headers*/
#include <ViewPlugin.h>
#include <ViewerWidget.h>
//#include <Transfer/CustomColorMapEditor.h>
/** HDPS headers*/
#include <Dataset.h>
#include <widgets/DropWidget.h>
#include <RendererSettingsAction.h>
//#include <Transfer/TransferFunctionControlAction.h>
#include <PointData.h>
/** VTK headers*/
#include <vtkPlane.h>
#include <vtkPlaneCollection.h>

using hdps::plugin::ViewPluginFactory;
using hdps::plugin::ViewPlugin;
using namespace hdps::util;
using namespace hdps;

class Images;
class ViewerWidget;
class SettingsWidget;
class ViewerWidget;
class Points;

namespace hdps {
    class CoreInterface;
    namespace gui {
        class DropWidget;
    }
}

/**
 * 3D viewer plugin class
 * This HDPS view plugin class provides functionality to view high-dimensional Points Data loaded by the HDVOL loader plugin
 *
 * @author Mitchell M. de Boer
 */
class Flow4DViewerPlugin : public ViewPlugin
{
    Q_OBJECT

public:
    /** Constructor */
    Flow4DViewerPlugin(const hdps::plugin::PluginFactory* factory);

public: // Inherited from ViewPlugin

    ~Flow4DViewerPlugin() override = default;

    /** Initializes the plugin */
    void init() override;
    hdps::CoreInterface* getCore();

    void reInitializeLayout(QHBoxLayout layout);

    void runRenderData();

    void setSelectionPosition(double x, double y, double z);

    /** Returns a pointer to the core interface */
    hdps::CoreInterface* core() { return _core; }


public: // Miscellaneous
    /** Returns the image viewer widget */
    ViewerWidget& getViewerWidget() {
        return *_viewerWidget;
    }

    /** Returns the render settings action*/
    RendererSettingsAction& getRendererSettingsAction() {
        return _rendererSettingsAction;
    }


    /** Returns the names of the points datasets in HDPS */
    QStringList getPointsDatasets() const {
        return _pointsDatasets;
    }
    
    /** Returns the imageData */
    vtkSmartPointer<vtkImageData> getImageData() {
        return _imageData;
    }

    bool& getBackgroundEndabled() {
        return _backgroundEnabled;
    }

    float& getBackgroundAlpha() {
        return _backgroundAlpha;
    }

    bool& getSelectionOpaque() {
        return _selectionOpaque;
    }

    Dataset<Points>& getDataset() {
        return _points;
    }

signals:
    /** Signals that list of point datasets in HDPS has changed */
    void pointsDatasetsChanged(QStringList pointsDatasets);

private:
    RendererSettingsAction              _rendererSettingsAction;    /** The options menu on the side of the viewer*/
    ViewerWidget*                       _viewerWidget;              /** The image viewer widget */
    vtkSmartPointer<vtkImageData>       _imageData;                 /** The full data loaded into the viewer */
    
    Dataset<Points>                     _points;                    /** Declare a points dataset reference */
    QStringList                         _pointsDatasets;            /** Point datasets loaded in HDPS */
    hdps::gui::DropWidget*              _dropWidget;                /** Widget for dropping data */
    QString                             _currentDatasetName;        /** Name of the current dataset */
    std::vector<int>                    _planeArray;                /** Array indicating the index+1 of the x,y and z clipping planes in the plane collection*/
    std::vector<double>                  _shadingParameters;         /** Shading parameter save vector index 0 = ambient, index 1 = diffuse and index 2 = specular*/
    std::vector<double>                 _position;
    std::string                         _interpolationOption;       /** String for storing the current color interpolation option*/
    std::string                         _colorMap;                  /** String for storing the current color map*/
    bool                                _dataLoaded;                /** Booling indicating if data has been loaded in*/
    bool                                _dataSelected;              /** Boolian indicating if data has been selected in a scatterplot*/
    bool                                _shadingEnabled;            /** Boolian for inicating if shading should be enabled*/
    bool                                _backgroundEnabled;         /** Boolian for indicating if the non-selected datapoints should be shown*/
    bool                                _selectionOpaque;           /** Boolian indicating wether the selected datapoints should be opaque or use the transfer function*/
    float                               _backgroundAlpha;           /** Float indcating the alpha value of the background during selection.*/
};

/**
 * Image viewer plugin factory class
 * A factory for creating image viewer plugin instances
 */
class Flow4DViewerPluginFactory : public ViewPluginFactory
{
    Q_INTERFACES(hdps::plugin::ViewPluginFactory hdps::plugin::PluginFactory)
        Q_OBJECT
        Q_PLUGIN_METADATA(IID "nl.tudelft.Flow4DViewerPlugin" FILE "Flow4DViewerPlugin.json")

public:
    /** Default constructor */
    Flow4DViewerPluginFactory() {}

    /** Destructor */
    ~Flow4DViewerPluginFactory() override {}

    /**
     * Get plugin icon
     * @param color Icon color for flat (font) icons
     * @return Icon
     */
    QIcon getIcon(const QColor& color = Qt::black) const override;

    /** Creates an image viewer plugin instance */
    Flow4DViewerPlugin* produce() override;

    hdps::DataTypes supportedDataTypes() const override;

    /**
     * Get plugin trigger actions given \p datasets
     * @param datasets Vector of input datasets
     * @return Vector of plugin trigger actions
     */
    PluginTriggerActions getPluginTriggerActions(const hdps::Datasets& datasets) const override;
};