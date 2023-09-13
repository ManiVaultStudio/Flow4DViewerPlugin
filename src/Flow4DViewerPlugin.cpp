/** General headers*/
#include <vector>
/** QT headers*/
#include <QDebug>
#include <QMimeData>
#include <QLayout>
/** Plugin headers*/
#include "Flow4DViewerPlugin.h"
#include "ViewerWidget.h"
//#include "Transfer/CustomColorMapEditor.h"
#include <widgets/DropWidget.h>

#include <actions/PluginTriggerAction.h>
#include <DatasetsMimeData.h>
/** HDPS headers*/
#include <PointData/PointData.h>
#include <ClusterData/ClusterData.h>
#include <ColorData/ColorData.h>
/** VTK headers*/
#include <vtkPlaneCollection.h>
#include <vtkPlane.h>

using namespace hdps;
using namespace hdps::gui;
using namespace hdps::util;

Flow4DViewerPlugin::Flow4DViewerPlugin(const PluginFactory* factory) :
    ViewPlugin(factory),
    _viewerWidget(nullptr),
    //_transferWidget(nullptr),
    //_selectionData(vtkSmartPointer<vtkImageData>::New()),
    _imageData(vtkSmartPointer<vtkImageData>::New()),
    
    _points(),

    _pointsParent(),
    _pointsColorCluster(),
    _rendererSettingsAction(),

    _dropWidget(nullptr),
    
    // boolian to indicate if data is loaded for selection visualization purposes
    _dataLoaded(false),
    // boolian to indicate if data has been selected in a scatterplot
    _dataSelected(false),
    // Boolian to indicate if shading has been enabled;
    _shadingEnabled(false),
    // Boolian to indicate if non-selected data should be shown
    _backgroundEnabled(true),
    // float to indicate alpha value of background when data is selected
    _backgroundAlpha(0.02),
    // Boolian to indicate wheter selected data should be opaque or use the transfer function
    _selectionOpaque(true),
    // Shading parameter vector.
    _shadingParameters(std::vector<double>{0.9,0.2,0.1}),
    // string variable to keep track of the interpolation option with default being Nearest Neighbour
    _interpolationOption("NN")
{
    getWidget().setContextMenuPolicy(Qt::CustomContextMenu);
    getWidget().setFocusPolicy(Qt::ClickFocus);
}

void Flow4DViewerPlugin::init()
{
    // Add the viewerwidget and dropwidget to the layout.
    _viewerWidget = new ViewerWidget(*this);
   // Add the dropwidget to the layout.
    _dropWidget = new DropWidget(_viewerWidget);
    _rendererSettingsAction = new RendererSettingsAction(this, _viewerWidget, "Primary Toolbar");
    
    
    // Create the layout.
    auto layout = new QHBoxLayout();
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->addWidget(_viewerWidget, 1);

    /*auto settingsLayout = new QVBoxLayout();

    settingsLayout->addWidget(_rendererSettingsAction.createWidget(&getWidget()));
    settingsLayout->setContentsMargins(6, 6, 6, 6);

     //Add the actions.
    GroupsAction::GroupActions groupActions;
    groupActions << &_rendererSettingsAction.getDimensionAction() << &_rendererSettingsAction.getColoringAction() << &_rendererSettingsAction.getSelectedPointsAction();
    _rendererSettingsAction.setGroupActions(groupActions);

    layout->addLayout(settingsLayout, 1);

    getWidget().setAutoFillBackground(true);*/
    getWidget().setLayout(layout);
    
    // Set the drop indicator widget (the widget that indicates that the view is eligible for data dropping)
    _dropWidget->setDropIndicatorWidget(new DropWidget::DropIndicatorWidget(&getWidget(), "No data loaded", "Drag an item from the data hierarchy and drop it here to visualize data..."));

    // Initialize the drop regions
    _dropWidget->initialize([this](const QMimeData* mimeData) -> DropWidget::DropRegions {

        // A drop widget can contain zero or more drop regions
        DropWidget::DropRegions dropRegions;

        const auto datasetsMimeData = dynamic_cast<const DatasetsMimeData*>(mimeData);

        if (datasetsMimeData == nullptr)
            return dropRegions;

        if (datasetsMimeData->getDatasets().count() > 1)
            return dropRegions;

        const auto dataset = datasetsMimeData->getDatasets().first();
        const auto datasetGuiName = dataset->text();
        const auto datasetId = dataset->getId();
        const auto dataType = dataset->getDataType();
        const auto dataTypes = DataTypes({ PointType });

        // Visually indicate if the dataset is of the wrong data type and thus cannot be dropped
        if (!dataTypes.contains(dataType)) {
            dropRegions << new DropWidget::DropRegion(this, "Incompatible data", "", "This type of data is not supported", false);
        }
        else {
            
            // Accept points datasets drag and drop
            if (dataType == PointType) {
                
                const auto candidateDataset = getCore()->requestDataset<Points>(datasetId);
                //const auto candidateDatasetName = candidateDataset.getName();
                const auto description = QString("Visualize %1 as voxels").arg(candidateDataset->getGuiName());

                if (!_points.isValid()) {
                    dropRegions << new DropWidget::DropRegion(this, "Position", description, "cube", true, [this, candidateDataset]() {
                        _points = candidateDataset;
                        if (_points->getDataHierarchyItem().hasParent()) {
                            _pointsParent = _points->getParent();
                        }

                    });
                }
                else {
                    if (candidateDataset == _points) {
                        dropRegions << new DropWidget::DropRegion(this, "Warning", "Data already loaded", "exclamation-circle", false);
                    }
                    else {
                        dropRegions << new DropWidget::DropRegion(this, "Voxels", description, "cube", true, [this, candidateDataset]() {
                            _points = candidateDataset;

                            if (_points->getDataHierarchyItem().hasParent()) {
                                _pointsParent = _points->getParent();
                            }

                        });
                        //if (candidateDataset->getNumPoints() == _points->getNumPoints()) {

                        //    // The number of points is equal, so offer the option to use the points dataset as source for points colors
                        //    dropRegions << new DropWidget::DropRegion(this, "Point color", QString("Colorize %1 points with %2"), "palette", true, [this, candidateDataset]() {
                        //        //_settingsAction.getColoringAction().addColorDataset(candidateDataset);
                        //        //_settingsAction.getColoringAction().setCurrentColorDataset(candidateDataset);
                        //        _pointsColorCluster = candidateDataset;
                        //    });

                    }
                }
            }
        }
        // Cluster dataset is about to be dropped
        if (dataType == ClusterType) {


            // Get clusters dataset from the core
            auto candidateDataset = _core->requestDataset<Clusters>(datasetId);


            // Establish drop region descriptio
            const auto description = QString("Color points by %1").arg(candidateDataset->getGuiName());

            // Only allow user to color by clusters when there is a positions dataset loaded
            if (_points.isValid()) {

                if (true) {

                    // The clusters dataset is already loaded
                    dropRegions << new DropWidget::DropRegion(this, "Color", description, "palette", true, [this, candidateDataset]() {
                        //auto test = candidateDataset->getClusters();


                        //auto t = test[0];
                        //std::cout << t.toString().toStdString() << std::endl;
                        _pointsColorCluster = candidateDataset;
                    });
                }
                else {

                    // Use the clusters set for points color
                    dropRegions << new DropWidget::DropRegion(this, "Color", description, "palette", true, [this, candidateDataset]() {
                        //_settingsAction.getColoringAction().addColorDataset(candidateDataset);
                        //_settingsAction.getColoringAction().setCurrentColorDataset(candidateDataset);
                    });
                }
            }
            else {

                // Only allow user to color by clusters when there is a positions dataset loaded
                dropRegions << new DropWidget::DropRegion(this, "No points data loaded", "Clusters can only be visualized in concert with points data", "exclamation-circle", false);
            }
        }
        return dropRegions;
    });

    // Respond when the name of the dataset in the dataset reference changes
    connect(&_points, &Dataset<Points>::changed, this, [this, layout]() {
        // Get current dimension index
        std::string dimensionName = _rendererSettingsAction->getDimensionAction().getSelectedDataAction().getCurrentText().toStdString(); // get the currently selected chosen dimension as indicated by the dimensionchooser in the options menu
        int chosenDimension = 3;
        if (dimensionName == "Flow Speed")
        {
            chosenDimension = 3;
        }
        else if (dimensionName == "Line Index") {
            chosenDimension = 4;
        }
        else if (dimensionName == "Time Interval") {
            chosenDimension = 5;
        }
        else if (dimensionName == "Group") {
            chosenDimension = 6;
        }


        
        // hide dropwidget
        _dropWidget->setShowDropIndicator(false);
        int boundsArray[2] = { 0,29 };
        
            // Pass the dataset and chosen dimension to the viewerwidget which initiates the data and renders it. returns the imagedata object for future operations.
            _imageData = _viewerWidget->setData(*_points, chosenDimension, boundsArray, _colorMap);
        
            
        //Initial render.
        _viewerWidget->renderData( _imageData, _interpolationOption, _colorMap, _shadingEnabled, _shadingParameters);

        
        
        // notify that data is indeed loaded into the widget
        _dataLoaded = true;
    });
    addDockingAction(_rendererSettingsAction, nullptr, DockAreaFlag::Left, true, AutoHideLocation::Right, QSize(300, 300));

    // Dropdown menu for chosen dimension.
    connect(&this->getRendererSettingsAction().getDimensionAction().getSelectedDataAction(), &OptionAction::currentTextChanged, this, [this](const QString& interpolType) {
        // check if there is a dataset loaded in
        if (_dataLoaded) {
            // get the value of the chosenDimension
            std::string dimensionName = interpolType.toStdString();
            int chosenDimension = 3;
            if (dimensionName == "Flow Speed")
            {
                chosenDimension = 3;
            }
            else if (dimensionName == "Line Index") {
                chosenDimension = 4;
            }
            else if (dimensionName == "Time Interval") {
                chosenDimension = 5;
            }
            else if (dimensionName == "Group") {
                chosenDimension = 6;
            }

            int boundsArrays[2] = { 0,29 };
            // pass the dataset and chosen dimension to the viewerwidget which initiates the data and renders it. returns the imagedata object for future operations
            _imageData = _viewerWidget->setData(*_points, chosenDimension, boundsArrays, _colorMap);

            // Get the selection set that changed
            const auto& selectionSet = _points->getSelection<Points>();
            float lowerThreshold = this->getRendererSettingsAction().getSelectedPointsAction().getDecimalRangeAction().getMinimum();
            float upperThreshold = this->getRendererSettingsAction().getSelectedPointsAction().getDecimalRangeAction().getMaximum();
            int boundsArray[2] = { lowerThreshold,upperThreshold };

            // get selectionData
            _viewerWidget->setSelectedData(*_points, selectionSet->indices, chosenDimension, boundsArray);
            runRenderData();

        }
    });

    //// Surrounding data enabled selector
    //connect(&this->getRendererSettingsAction().getSelectedPointsAction().getBackgroundShowAction(), &OptionAction::currentTextChanged, this, [this](const QString& show) {
    //    // Selector option handeling
    //    if (show == "Show background") {
    //        _backgroundEnabled = true;
    //        // Enable the alpha slider for the background
    //        this->getRendererSettingsAction().getSelectedPointsAction().getBackgroundAlphaAction().setDisabled(false);
    //    }
    //    else {
    //        _backgroundEnabled = false;
    //        // Disable the alpha slider for the background
    //        this->getRendererSettingsAction().getSelectedPointsAction().getBackgroundAlphaAction().setDisabled(true);
    //    }

    //    if (_dataSelected) {
    //        runRenderData();
    //    }
    //});
    //// Background alpha slider
    //connect(&this->getRendererSettingsAction().getSelectedPointsAction().getBackgroundAlphaAction(), &DecimalAction::valueChanged, this, [this](const float& value) {
    //    if (_backgroundEnabled) {
    //        _backgroundAlpha = value;
    //        if (_dataSelected) {
    //            runRenderData();
    //        }
    //    }
    //});
    //// Selection alpha setting selector
    //connect(&this->getRendererSettingsAction().getSelectedPointsAction().getSelectionAlphaAction(), &OptionAction::currentTextChanged, this, [this](const QString& opaqueOrTf) {
    //    if (opaqueOrTf == "Opaque") {
    //        _selectionOpaque = true;
    //    }
    //    else {
    //        _selectionOpaque = false;
    //    }
    //    if (_dataSelected) {
    //        runRenderData();
    //    }
    //});   
    
  
    connect(&this->getRendererSettingsAction().getSelectedPointsAction().getSelectPointAction(), &TriggerAction::triggered, this, [this]() {
        std::string xyz = "F:/BME_year2/thesis/VTK/xyz.txt";
        std::ofstream file(xyz);

        float lowerThreshold = this->getRendererSettingsAction().getSelectedPointsAction().getDecimalRangeAction().getMinimum();
        float upperThreshold = this->getRendererSettingsAction().getSelectedPointsAction().getDecimalRangeAction().getMaximum();

        if (file.is_open()) {
            for (int i = lowerThreshold; i <= upperThreshold; i++) {
                file << "x" << i << "\n";
                file << "y" << i << "\n";
                file << "z" << i << "\n";
            }
            file.close();
            std::cout << "File write successful." << std::endl;
        }
        else {
            std::cout << "Unable to open the file." << std::endl;
        }
    });
    connect(&this->getRendererSettingsAction().getSelectedPointsAction().getSelectPointActionSpeed(), &TriggerAction::triggered, this, [this]() {
        std::string xyzspeed = "F:/BME_year2/thesis/VTK/xyzspeed.txt";
        std::ofstream file(xyzspeed);

        float lowerThreshold = this->getRendererSettingsAction().getSelectedPointsAction().getDecimalRangeAction().getMinimum();
        float upperThreshold = this->getRendererSettingsAction().getSelectedPointsAction().getDecimalRangeAction().getMaximum();

        if (file.is_open()) {
            for (int i = lowerThreshold; i <= upperThreshold; i++) {
                file << "x" << i << "\n";
                file << "y" << i << "\n";
                file << "z" << i << "\n";
                file << "speed" << i << "\n";
            }
            file.close();
            std::cout << "File write successful." << std::endl;
        }
        else {
            std::cout << "Unable to open the file." << std::endl;
        }
    });
    connect(&this->getRendererSettingsAction().getSelectedPointsAction().getSelectPointActionTime(), &TriggerAction::triggered, this, [this]() {
        std::string xyzspeed = "F:/BME_year2/thesis/VTK/xyztime.txt";
        std::ofstream file(xyzspeed);

        float lowerThreshold = this->getRendererSettingsAction().getSelectedPointsAction().getDecimalRangeAction().getMinimum();
        float upperThreshold = this->getRendererSettingsAction().getSelectedPointsAction().getDecimalRangeAction().getMaximum();

        if (file.is_open()) {
            for (int i = lowerThreshold; i <= upperThreshold; i++) {
                file << "x" << i << "\n";
                file << "y" << i << "\n";
                file << "z" << i << "\n";
                file << "time" << i << "\n";
            }
            file.close();
            std::cout << "File write successful." << std::endl;
        }
        else {
            std::cout << "Unable to open the file." << std::endl;
        }
    });connect(&this->getRendererSettingsAction().getSelectedPointsAction().getSelectPointActionSpeedTime(), &TriggerAction::triggered, this, [this]() {
        std::string xyzspeed = "F:/BME_year2/thesis/VTK/xyzspeedtime.txt";
        std::ofstream file(xyzspeed);

        float lowerThreshold = this->getRendererSettingsAction().getSelectedPointsAction().getDecimalRangeAction().getMinimum();
        float upperThreshold = this->getRendererSettingsAction().getSelectedPointsAction().getDecimalRangeAction().getMaximum();

        if (file.is_open()) {
            for (int i = lowerThreshold; i <= upperThreshold; i++) {
                file << "x" << i << "\n";
                file << "y" << i << "\n";
                file << "z" << i << "\n";
                file << "speed" << i << "\n";
                file << "time" << i << "\n";
            }
            file.close();
            std::cout << "File write successful." << std::endl;
        }
        else {
            std::cout << "Unable to open the file." << std::endl;
        }
    });

    connect(&this->getRendererSettingsAction().getSelectedPointsAction().getDecimalRangeAction(), &DecimalRangeAction::rangeChanged, this, [this]() {
        float lowerThreshold = this->getRendererSettingsAction().getSelectedPointsAction().getDecimalRangeAction().getMinimum();
        float upperThreshold = this->getRendererSettingsAction().getSelectedPointsAction().getDecimalRangeAction().getMaximum();
        int boundsArray[2] = { lowerThreshold,upperThreshold };
        //_points->setSelectionIndices();
        
        // Get the selection set that changed
        const auto& selectionSet = _points->getSelection<Points>();

        // Get ChosenDimension
        
        std::string dimensionName = _rendererSettingsAction->getDimensionAction().getSelectedDataAction().getCurrentText().toStdString(); // get the currently selected chosen dimension as indicated by the dimensionchooser in the options menu

        int chosenDimension = 3;
        if (dimensionName == "Flow Speed")
        {
            chosenDimension = 3;
        }
        else if (dimensionName == "Line Index") {
            chosenDimension = 4;
        }
        else if (dimensionName == "Time Interval") {
            chosenDimension = 5;
        }
        else if (dimensionName == "Group") {
            chosenDimension = 6;
        }
        //_imageData = _viewerWidget->setData(*_points, chosenDimension, boundsArray, _colorMap);
        //_viewerWidget->renderData( _imageData, _interpolationOption, _colorMap, _shadingEnabled, _shadingParameters);

        // create a selectiondata imagedata object with 0 values for all non selected items
        _viewerWidget->setSelectedDataTime(*_points, selectionSet->indices, chosenDimension, boundsArray);


        // if the selection is not empty add the selection to the vector 
        if (selectionSet->indices.size() != 0) {

            _dataSelected = true;
        }
        else {
            _dataSelected = false;
        }

        // Render the data with the current slicing planes and selections
        _viewerWidget->renderData(_imageData, _interpolationOption, _colorMap, _shadingEnabled, _shadingParameters);
        
    });

    // Colormap selector
    connect(&getRendererSettingsAction().getColoringAction().getColorMapAction(), &ColorMapAction::imageChanged, this, [this](const QImage& colorMapImage) {
        if (_dataLoaded) {
            runRenderData();
        }
    });

    // Selection changed connection.
    connect(&_points, &Dataset<Points>::dataSelectionChanged, this, [this]{
        // if data is loaded
        if (_dataLoaded) {

            // Get the selection set that changed
            const auto& selectionSet = _points->getSelection<Points>();

            // Get ChosenDimension
            std::string dimensionName = _rendererSettingsAction->getDimensionAction().getSelectedDataAction().getCurrentText().toStdString(); // get the currently selected chosen dimension as indicated by the dimensionchooser in the options menu
            int chosenDimension = 3;
            if (dimensionName == "Flow Speed")
            {
                chosenDimension = 3;
            }
            else if (dimensionName == "Line Index") {
                chosenDimension = 4;
            }
            else if (dimensionName == "Time Interval") {
                chosenDimension = 5;
            }
            else if (dimensionName == "Group") {
                chosenDimension = 6;
            }

            const auto backGroundValue = _imageData->GetScalarRange()[0];

            float lowerThreshold = this->getRendererSettingsAction().getSelectedPointsAction().getDecimalRangeAction().getMinimum();
            float upperThreshold = this->getRendererSettingsAction().getSelectedPointsAction().getDecimalRangeAction().getMaximum();
            int boundsArray[2] = { lowerThreshold,upperThreshold };
            // create a selectiondata imagedata object with 0 values for all non selected items
            _viewerWidget->setSelectedData(*_points, selectionSet->indices, chosenDimension, boundsArray);
            
           
            // if the selection is not empty add the selection to the vector 
            if (selectionSet->indices.size() != 0) {
                
                _dataSelected = true;
            }
            else {
                _dataSelected = false;
            }

            // Render the data with the current slicing planes and selections
            _viewerWidget->renderData(_imageData, _interpolationOption, _colorMap, _shadingEnabled, _shadingParameters);
                
                
        }
    });
}

void Flow4DViewerPlugin::reInitializeLayout(QHBoxLayout layout) {

}

hdps::CoreInterface* Flow4DViewerPlugin::getCore()
{
    return _core;
}

QIcon Flow4DViewerPluginFactory::getIcon(const QColor& color /*= Qt::black*/) const
{
    return hdps::Application::getIconFont("FontAwesome").getIcon("cube", color);
}

Flow4DViewerPlugin* Flow4DViewerPluginFactory::produce()
{
    return new Flow4DViewerPlugin(this);
}

hdps::DataTypes Flow4DViewerPluginFactory::supportedDataTypes() const
{
    DataTypes supportedTypes;
    supportedTypes.append(PointType);
    return supportedTypes;
}

hdps::gui::PluginTriggerActions Flow4DViewerPluginFactory::getPluginTriggerActions(const hdps::Datasets& datasets) const
{
    PluginTriggerActions pluginTriggerActions;

    const auto getInstance = [this]() -> Flow4DViewerPlugin* {
        return dynamic_cast<Flow4DViewerPlugin*>(plugins().requestViewPlugin(getKind()));
    };

    const auto numberOfDatasets = datasets.count();

    if (PluginFactory::areAllDatasetsOfTheSameType(datasets, PointType)) {
        if (numberOfDatasets >= 1) {
            if (datasets.first()->getDataType() == PointType) {
                auto pluginTriggerAction = new PluginTriggerAction(const_cast<Flow4DViewerPluginFactory*>(this), this, "Volume viewer", "Load dataset in volume viewer", getIcon(), [this, getInstance, datasets](PluginTriggerAction& pluginTriggerAction) -> void {
                    for (auto dataset : datasets)
                        getInstance()->loadData(Datasets({ dataset }));
                });

                pluginTriggerActions << pluginTriggerAction;
            }
        }
    }

    return pluginTriggerActions;
}

/** Create a vtkimagedatavector to store the current imagedataand selected data(if present).
*   Vector is needed due to the possibility of having data selected in a scatterplot wich
*   changes the colormapping of renderdata and creates an aditional actor to visualize the selected data.
*/
void Flow4DViewerPlugin::runRenderData() {
    _viewerWidget->renderData( _imageData, _interpolationOption, _colorMap, _shadingEnabled, _shadingParameters);
}

void Flow4DViewerPlugin::setSelectionPosition(double x, double y, double z) {
    _position[0] = x;
    _position[1] = y;
    _position[2] = z;
   
}
