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
/** mv headers*/
#include <PointData/PointData.h>
#include <ClusterData/ClusterData.h>
#include <ColorData/ColorData.h>
/** VTK headers*/
#include <vtkPlaneCollection.h>
#include <vtkPlane.h>
#include <vtkVersion.h>

Q_PLUGIN_METADATA(IID "nl.BioVault.Flow4DViewerPlugin")




Flow4DViewerPlugin::Flow4DViewerPlugin(const PluginFactory* factory) :
    ViewPlugin(factory),
    _viewerWidget(nullptr),
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

        const auto dataName = dataset->getGuiName();
       
        const auto dataType = dataset->getDataType();
        const auto dataTypes = DataTypes({ PointType });

        // Visually indicate if the dataset is of the wrong data type and thus cannot be dropped
        if (!dataTypes.contains(dataType)) {
            dropRegions << new DropWidget::DropRegion(this, "Incompatible data", "", "This type of data is not supported", false);
        }
        else {
            
            // Accept points datasets drag and drop
            if (dataType == PointType) {
                
                const auto candidateDataset = mv::data().getDataset(datasetId);     //requestDataset<Points>(datasetId);
                //const auto candidateDatasetName = candidateDataset.getName();
                const auto description = QString("Visualize %1 as voxels").arg(datasetGuiName);

                if (!_points.isValid()) {
                    dropRegions << new DropWidget::DropRegion(this, "Position", description, "cube", true, [this, candidateDataset]() {
                        _points = candidateDataset;
                        int iterator = 0;
                        
                        bool first = false;
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
                            int iterator = 0;
                            std::cout << _points->getNumPoints() << std::endl;
                            bool first = false;
                            if (_points->getDataHierarchyItem().hasParent()) {
                                _pointsParent = _points->getParent();
                                

                            }

                        });

                    }
                }
            }
        }
        // Cluster dataset is about to be dropped
        if (dataType == ClusterType) {


            // Get clusters dataset from the core
            auto candidateDataset = mv::data().getDataset(datasetId);  //_core->requestDataset<Clusters>(datasetId);


            // Establish drop region descriptio
            const auto description = QString("Color points by %1").arg(candidateDataset->getGuiName());

            // Only allow user to color by clusters when there is a positions dataset loaded
            if (_points.isValid()) {

                if (true) {

                    // The clusters dataset is already loaded
                    dropRegions << new DropWidget::DropRegion(this, "Color", description, "palette", true, [this, candidateDataset]() {
                        _pointsColorCluster = candidateDataset;
                    });
                }
                else {

                    // Use the clusters set for points color
                    dropRegions << new DropWidget::DropRegion(this, "Color", description, "palette", true, [this, candidateDataset]() {
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
        else if (dimensionName == "Flow Group") {
            chosenDimension = 6;
        }
        int iterator = 0;
        int count = 0;
        int first = false;
        auto children = _points->getChildren();
        _points->getDimensionNames();
        while (iterator / 10 < _points->getNumPoints() * _points->getProperty("lineSize").toInt()) {

            if (_points->getValueAt(iterator + 6) > 0  && first == false) {
                                
                std::cout << _points->getValueAt(iterator + 6) << "num1" << std::endl;
                first = true;
            }
            iterator = iterator + 10;
            count++;
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
            else if (dimensionName == "Flow Group") {
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

    connect(&this->getRendererSettingsAction().getColoringAction().getToggleColorSelection(), &ToggleAction::changed, this, [this]() {
        bool colorToggled = this->getRendererSettingsAction().getColoringAction().getToggleColorSelection().isChecked();

        this->getViewerWidget().setColorSelectionToggled(colorToggled);

    });

    connect(&this->getRendererSettingsAction().getSelectedPointsAction().getConstructSelection(), &TriggerAction::triggered, this, [this]() {
        auto xyzToggled = this->getRendererSettingsAction().getSelectedPointsAction().getXyzToggled().isChecked();
        bool xyzDevToggled = this->getRendererSettingsAction().getSelectedPointsAction().getXyzDevToggled().isChecked();
        bool speedToggled = this->getRendererSettingsAction().getSelectedPointsAction().getSpeedToggled().isChecked();
        bool timeToggled = this->getRendererSettingsAction().getSelectedPointsAction().getTimeToggled().isChecked();



        std::string xyz = "F:/BME_year2/thesis/VTK/Selection.txt";
        std::ofstream file(xyz);

        float lowerThreshold = this->getRendererSettingsAction().getSelectedPointsAction().getDecimalRangeAction().getMinimum();
        float upperThreshold = this->getRendererSettingsAction().getSelectedPointsAction().getDecimalRangeAction().getMaximum();

        if (file.is_open()) {
            for (int i = lowerThreshold; i <= upperThreshold; i++) {
                if (xyzToggled) {
                    file << "x" << i << "\n";
                    file << "y" << i << "\n";
                    file << "z" << i << "\n";
                }
                if (speedToggled) {
                    file << "speed" << i << "\n";
                }
                if (timeToggled) {
                    file << "time" << i << "\n";
                }
                if (xyzDevToggled) {
                    file << "x'" << i << "\n";
                    file << "y'" << i << "\n";
                    file << "z'" << i << "\n";
                }


            }
            file.close();
            std::cout << "File write successful." << std::endl;
        }
        else {
            std::cout << "Unable to open the file." << std::endl;
        }
    });
    // Connection for time window selection.
    connect(&this->getRendererSettingsAction().getSelectedPointsAction().getDecimalRangeAction(), &DecimalRangeAction::rangeChanged, this, [this]() {
        float lowerThreshold = this->getRendererSettingsAction().getSelectedPointsAction().getDecimalRangeAction().getMinimum();
        float upperThreshold = this->getRendererSettingsAction().getSelectedPointsAction().getDecimalRangeAction().getMaximum();
        int boundsArray[2] = { lowerThreshold,upperThreshold };
        
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
        else if (dimensionName == "Flow Group") {
            chosenDimension = 6;
        }
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
            else if (dimensionName == "Flow Group") {
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

mv::CoreInterface* Flow4DViewerPlugin::getCore()
{
    return _core;
}

QIcon Flow4DViewerPluginFactory::getIcon(const QColor& color /*= Qt::black*/) const
{
    return mv::Application::getIconFont("FontAwesome").getIcon("cube", color);
}

Flow4DViewerPlugin* Flow4DViewerPluginFactory::produce()
{
    return new Flow4DViewerPlugin(this);
}

mv::DataTypes Flow4DViewerPluginFactory::supportedDataTypes() const
{
    DataTypes supportedTypes;
    supportedTypes.append(PointType);
    return supportedTypes;
}

mv::gui::PluginTriggerActions Flow4DViewerPluginFactory::getPluginTriggerActions(const mv::Datasets& datasets) const
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
