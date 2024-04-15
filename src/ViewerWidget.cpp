/** General headers */
#include <math.h>
#include <algorithm> 
#include <vector>
#include <cmath>
#include <iostream>
#include <vector>
#include <iterator>
#include <algorithm>
/** Plugin headers */
#include <ViewerWidget.h>
#include <RendererSettingsAction.h>
#include <Flow4DViewerPlugin.h>

/** mv headers */
#include <Dataset.h>
#include <PointData/PointData.h>

#include <ClusterData/ClusterData.h>

/** QT headers */
#include <qwidget.h>
#include <qdialog.h>
/** VTK headers */
#include <QVTKOpenGLNativeWidget.h>
#include <vtkPlane.h>
#include <vtkPlaneCollection.h>
#include <vtkImageData.h>
#include <vtkVolume.h>
#include <vtkColorTransferFunction.h>
#include <vtkPiecewiseFunction.h>
#include <vtkVolumeProperty.h>
#include <vtkFloatArray.h>
#include <vtkPointData.h>
#include <vtkActor.h>
#include <vtkProperty.h>
#include <vtkCamera.h>
#include <vtkIntArray.h>
#include <vtkGPUVolumeRayCastMapper.h>

#include <vtkInteractorStyleTrackballCamera.h>
#include <vtkPointPicker.h>
#include <vtkRendererCollection.h>
#include <vtkNamedColors.h>
#include <vtkCellPicker.h>
#include <vtkCellLocator.h>
#include <vtkPoints.h>
#include <vtkImageThreshold.h>
#include <vtkPolyLine.h>
#include <vtkCellArray.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkActor.h>
#include <vtkLookupTable.h>

using namespace mv;
using namespace mv::gui;

namespace {
// Catch mouse events
class MouseInteractorStyle : public vtkInteractorStyleTrackballCamera
{
    public:
        static MouseInteractorStyle* New();
        MouseInteractorStyle()
        {
            selectedMapper = vtkSmartPointer<vtkGPUVolumeRayCastMapper>::New();
            selectedActor = vtkSmartPointer<vtkActor>::New();
        }
        virtual void OnLeftButtonDown() override
        {
            vtkNew<vtkNamedColors> colors;

            // Get the location of the click (in window coordinates)
            int* pos = this->GetInteractor()->GetEventPosition();

            vtkNew<vtkCellPicker> picker;
            picker->SetTolerance(0.0005);

            // Pick from this location.
            picker->Pick(pos[0], pos[1], 0, this->GetDefaultRenderer());
            double* worldPosition = picker->GetPickPosition();
            int cellID = picker->GetCellId();
            auto xyz = picker->GetCellIJK();          

            // Forward events
            vtkInteractorStyleTrackballCamera::OnLeftButtonDown();
        }

        void setViewer(ViewerWidget* widget) {
            _widget = widget;
        }        
        ViewerWidget* _widget;
        vtkSmartPointer<vtkGPUVolumeRayCastMapper> selectedMapper;
        vtkSmartPointer<vtkActor> selectedActor;
    };

vtkStandardNewMacro(MouseInteractorStyle);

} // namespace


ViewerWidget::ViewerWidget(Flow4DViewerPlugin& Flow4DViewerPlugin, QWidget* parent) :
	QWidget(parent),
	mRenderWindow(vtkSmartPointer<vtkGenericOpenGLRenderWindow>::New()),
	mRenderer(vtkSmartPointer<vtkRenderer>::New()),
	mInteractor(vtkSmartPointer<QVTKInteractor>::New()),
    _labelMap(vtkSmartPointer<vtkImageData>::New()),
    _imData(vtkSmartPointer<vtkImageData>::New()),
	numDimensions(),
	numPoints(),
	_Flow4DViewerPlugin(Flow4DViewerPlugin),
	_openGLWidget(),
    _dataSelected(false),
    _selectedCell(),
    _selectedCellCoordinate(),
    _thresholded(false),
    _lowerThreshold(),
    _upperThreshold(),
    _clusterData(),
    _clusterLoaded(false),
    _colorSelectionToggled(false),
    _firstRender(true),
    _polyData(), _savedSpeedArray(), _chosenDimension()

{
	setAcceptDrops(true);
	// Initiate the QVTKOpenGLWidget.
	_openGLWidget = new QVTKOpenGLNativeWidget(this);
    vtkNew<MouseInteractorStyle> mStyle;
	// Setup the Renderwindow.
	mRenderWindow->AddRenderer(mRenderer);
	mInteractor->SetRenderWindow(mRenderWindow);
	_openGLWidget->setRenderWindow(mRenderWindow);
	mInteractor->Initialize();
    mStyle->SetDefaultRenderer(mRenderer);
    mStyle->setViewer(this);
    //mInteractorStyle->get
    mInteractor->SetInteractorStyle(mStyle);
    _polyData = vtkSmartPointer<vtkPolyData>::New();
	// Set the background color.
	mRenderer->SetBackground(0, 0, 0);
	numDimensions = 0;
	numPoints = 0;
    _selectedCell = -1;
    _savedSpeedArray = vtkSmartPointer<vtkFloatArray>::New();
    _chosenDimension = 3;
    
}

ViewerWidget::~ViewerWidget()
{

}
// Function to add data into the viewerwidget.
vtkSmartPointer<vtkImageData> ViewerWidget::setData(Points& data, int chosenDim, int boundsArray[2], std::string colorMap)
{
    // Save time window bounds.
    int upperBound = boundsArray[1];
    int lowerBound = boundsArray[0];
    
    _firstRender = true;
	// Get number of points from points dataset.
    _chosenDimension = chosenDim;
	// Get number of dimensions from points dataset.
	numDimensions = data.getNumDimensions();
    int lineSize = data.getProperty("lineSize").toInt();

    // Was used for alternative visualization method, should now work using just the else statement.
    if (numDimensions > 10) {
        numPoints = data.getNumPoints()* lineSize;
        
    }
    else {
        numPoints = data.getNumPoints();
    }
    
    vtkSmartPointer<vtkPoints> vtkPointObject = vtkSmartPointer<vtkPoints>::New();
    vtkPointObject->SetNumberOfPoints(numPoints);
    int iterator = 0;
    vtkSmartPointer<vtkCellArray> cells = vtkSmartPointer<vtkCellArray>::New();
    vtkSmartPointer<vtkFloatArray> dataArray = vtkSmartPointer<vtkFloatArray>::New();
    dataArray->SetNumberOfValues(numPoints);
    int pointCounter = 0;
    
    //  If statement no longer in use i believe with the current vtkloaderplugin.
    if (numDimensions>10) {     
        while (iterator / 10 < numPoints) {
            vtkSmartPointer<vtkPolyLine> polyLine = vtkSmartPointer<vtkPolyLine>::New();
            float currentLineIndex = data.getValueAt(iterator + 4);//lineIndex
            float nextLineIndex = currentLineIndex;
            int i = 0;
            
            std::vector<std::vector<int>> savePoints;
            
            while (currentLineIndex == nextLineIndex)
            {
                double p[3] = { data.getValueAt(iterator), data.getValueAt(iterator + 1), data.getValueAt(iterator + 2) };
                dataArray->SetValue(pointCounter, data.getValueAt(iterator + chosenDim));
                
                vtkPointObject->SetPoint(pointCounter, p);

                savePoints.push_back({ i,pointCounter });
                i++;
                iterator = iterator + 10;
                pointCounter++;
                if (iterator > numPoints * 10) {
                    break;
                }
                nextLineIndex = data.getValueAt(iterator + 4);
            }
            
            polyLine->GetPointIds()->SetNumberOfIds(i);
            for (int j = 0; j < savePoints.size(); j++) {
                polyLine->GetPointIds()->SetId(savePoints[j][0], savePoints[j][1]);
            }
            cells->InsertNextCell(polyLine);
        }
    }
    else {
        // Loop over all points.
        while (iterator / 10 < numPoints) {
            // Create a polyline object for the generation of pathlines
            vtkSmartPointer<vtkPolyLine> polyLine = vtkSmartPointer<vtkPolyLine>::New();

            // Record the next and current line indices so that the line creation can be stopped at the correct time.
            float currentLineIndex = data.getValueAt(iterator + 4);//lineIndex
            float nextLineIndex = currentLineIndex;
            int i = 0;

            // Iterate over points in the current pathline, if the current lineindex is different from the next one stop.
            std::vector<std::vector<int>> savePoints;
            while (currentLineIndex == nextLineIndex)
            {
                // If data falls within selected timewindow record that datapoint.
                if (data.getValueAt(iterator + 5) <= upperBound && data.getValueAt(iterator + 5) >= lowerBound) {
                   
                    double p[3] = { data.getValueAt(iterator), data.getValueAt(iterator + 1), data.getValueAt(iterator + 2) };
                    dataArray->SetValue(pointCounter, data.getValueAt(iterator + chosenDim));

                    vtkPointObject->SetPoint(pointCounter, p);

                    savePoints.push_back({ i,pointCounter });
                    i++;
                }

                iterator = iterator + 10;
                pointCounter++;
                nextLineIndex = data.getValueAt(iterator + 4);
            }
                // Set the number of points that need to be used to create a pathline.
                polyLine->GetPointIds()->SetNumberOfIds(i);
                // Create a line from these points.
                for (int j = 0; j < savePoints.size(); j++) {
                    polyLine->GetPointIds()->SetId(savePoints[j][0], savePoints[j][1]);
                }
                cells->InsertNextCell(polyLine);
                
            
        }
    }
    // Put all line data in a polydataset.
    _polyData->SetPoints(vtkPointObject);
    
    _savedSpeedArray = dataArray;
    
    _polyData->GetPointData()->SetScalars(dataArray);
    
    _polyData->SetLines(cells);

    // Imdata is no longer used and is a leftover statement from the volumeviewerplugin. 
    // Its still in here because it is build into a lot of functions and was not interfering when using the plugin for my thesis.
    // Imdata was replaced by polydata
	vtkSmartPointer<vtkImageData> imData = vtkSmartPointer<vtkImageData>::New();

	return imData;
}
	
void ViewerWidget::renderData( vtkSmartPointer<vtkImageData> imData, std::string interpolationOption, std::string colorMap, bool shadingEnabled, std::vector<double> shadingParameters){
 
    // Get the data range from the polydata.
    double dataMinimum = _polyData->GetScalarRange()[0];
    double background = 0;
    double dataMaximum = _polyData->GetScalarRange()[1];
    // Empty the renderer to avoid overlapping visualizations.
    mRenderer->RemoveAllViewProps();

    // Create color transfer function.
    vtkSmartPointer<vtkColorTransferFunction> color = vtkSmartPointer<vtkColorTransferFunction>::New();
    color->AddRGBPoint(0, 0, 0, 1, 1, 1);
    
    // Get the colormap action.
    auto& colorMapAction = _Flow4DViewerPlugin.getRendererSettingsAction().getColoringAction().getColorMapAction();

    // Get the colormap image.
    auto colorMapImage = colorMapAction.getColorMapImage();

    // If the dimension equals the flow component dimension, use pre defined colormap, otherwise loop to read in colors from the colormap qimage.
    if (_chosenDimension == 6) {
        
            color->AddRGBPoint(0, 1, 0, 0);
            color->AddRGBPoint(1, 1, 1, 0);
            color->AddRGBPoint(2, 0, 0, 1);
            color->AddRGBPoint(3, 0, 1, 0);
            color->AddRGBPoint(4, 0, 1, 1);
            color->AddRGBPoint(5, 0, 1, 1);
            color->AddRGBPoint(6, 0, 1, 1);
    }
    else {
        for (int pixelX = 0; pixelX < colorMapImage.width(); pixelX++) {
            const auto normalizedPixelX = static_cast<float>(pixelX) / static_cast<float>(colorMapImage.width());
            const auto pixelColor = colorMapImage.pixelColor(pixelX, 0);
            color->AddRGBPoint(normalizedPixelX * (dataMaximum - dataMinimum) + dataMinimum, pixelColor.redF(), pixelColor.greenF(), pixelColor.blueF());
        }
    }
  
    // Apply data and colormap to the mapper that is used when data is not selected.
    vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    mapper->SetInputData(_polyData);
    mapper->SetLookupTable(color);
    
    vtkSmartPointer<vtkActor> volActor = vtkSmartPointer<vtkActor>::New();
    volActor->SetMapper(mapper);
    volActor->GetProperty()->SetOpacity(0.2); 
    
    
    if (_dataSelected) {
       
            //When data is selected a different colormap action needs to be used in order to allow for opacity difference.
        
            vtkSmartPointer<vtkLookupTable> lut = vtkSmartPointer<vtkLookupTable>::New();
            if (_colorSelectionToggled) {

                
                lut->SetNumberOfTableValues(5);
                lut->SetRampToLinear();
                
                
                lut->SetTableValue(0, 1, 1, 1, 0.01);
                lut->SetTableValue(1, 1, 0, 0, 0.2);
                lut->SetTableValue(2, 1, 1, 0, 0.2);
                lut->SetTableValue(3, 0, 0, 1, 0.2);
                lut->SetTableValue(4, 0, 1, 0, 0.2);

            }
            else {
                lut->SetNumberOfTableValues(3);
                lut->SetRange(0.0, 3.0);
                lut->SetTableValue(0, 1, 1, 1, 0.01);
                lut->SetTableValue(1, 0, 1, 0, 0.1);
                lut->SetTableValue(2, 1, 0, 0, 0.5);
            }
            
            lut->Build();

        

        vtkSmartPointer<vtkPolyDataMapper> mapper2 = vtkSmartPointer<vtkPolyDataMapper>::New();
        mapper2->SetInputData(_polyData);
        mapper2->SetLookupTable(lut);

        vtkSmartPointer<vtkActor> volActor2 = vtkSmartPointer<vtkActor>::New();
        volActor2->SetMapper(mapper2);

        // Add the current volume to the renderer.
        mRenderer->AddViewProp(volActor2);
    }
    else {
        mRenderer->AddViewProp(volActor);
    }
	// Center camera.
    if (_firstRender) {
        mRenderer->ResetCamera(); 
        _firstRender = false;
    }
	// Render.
	mRenderWindow->Render();
}

void ViewerWidget::setSelectedData(Points& points, std::vector<unsigned int, std::allocator<unsigned int>> selectionIndices, int chosenDim, int boundsArray[2]) {
        // Create the data array used to safe the selected points
        vtkSmartPointer<vtkFloatArray> dataArray = vtkSmartPointer<vtkFloatArray>::New();
        dataArray->SetNumberOfValues(_polyData->GetPointData()->GetScalars()->GetSize());
        // Set all values in the array to 0 for non selected data.
        for (int i = 0; i < dataArray->GetSize(); i++)
        {
            dataArray->SetValue(i, 0);
        }
        
        // Now change these datapoints to a different value if they are selected in order to do differenction between them.
        int count = 0;
        for (int i = 0; i < selectionIndices.size(); i++)
        {
            for (int j = 0; j < points.getProperty("lineSize").toInt(); j++)
            {
                dataArray->SetValue(selectionIndices[i]* points.getProperty("lineSize").toInt()+j, 0.5);
            }
            for (int j = boundsArray[0]; j < boundsArray[1]; j++)
            {
                if (_colorSelectionToggled) {
                    dataArray->SetValue(selectionIndices[i] * points.getProperty("lineSize").toInt() + j, (_savedSpeedArray->GetValue(selectionIndices[i] * points.getProperty("lineSize").toInt() + j) + 1) * 0.25);
                }
                else {
                    dataArray->SetValue(selectionIndices[i] * points.getProperty("lineSize").toInt() + j, 1);

                }
            }
        }
        _polyData->GetPointData()->SetScalars(dataArray);

        if (selectionIndices.size() == 0) {
            _dataSelected = false;
            _polyData->GetPointData()->SetScalars(_savedSpeedArray);
        }
        else {
            _dataSelected = true;
            _polyData->GetPointData()->SetScalars(dataArray);
            
        }
        // Return the selection imagedata object.   
}
void ViewerWidget::setSelectedDataTime(Points& points, std::vector<unsigned int, std::allocator<unsigned int>> selectionIndices, int chosenDim, int boundsArray[2]) {
        vtkSmartPointer<vtkFloatArray> dataArray = vtkSmartPointer<vtkFloatArray>::New();
        dataArray->SetNumberOfValues(_polyData->GetPointData()->GetScalars()->GetSize());
        for (int i = 0; i < dataArray->GetSize(); i++)
        {
            dataArray->SetValue(i, 0);
        }
        
        int count = 0;
        for (int i = 0; i < selectionIndices.size(); i++)
        {
            for (int j = 0; j < 153; j++)
            {
                dataArray->SetValue(selectionIndices[i]* points.getProperty("lineSize").toInt()+j, 0.5);
            }
            for (int j = boundsArray[0]; j < boundsArray[1]; j++)
            {
                dataArray->SetValue(selectionIndices[i]* points.getProperty("lineSize").toInt()+j, 1);
            }
        }

        _polyData->GetPointData()->SetScalars(dataArray);
        if (selectionIndices.size() == 0) {
            _dataSelected = false;
            _polyData->GetPointData()->SetScalars(_savedSpeedArray);
        }
        else {
            _dataSelected = true;
            _polyData->GetPointData()->SetScalars(dataArray);
        }
        // Return the selection imagedata object.   
}

// Unused function in the flowviewer, left over from the volumeviewer.
vtkSmartPointer<vtkImageData> ViewerWidget::connectedSelection(Points& points,int chosenDim, int *seedpoint, float upperThreshold, float lowerThreshold) {
    double dataMinimum = _imData->GetScalarRange()[0]+1;

    double dataMaximum = _imData->GetScalarRange()[1];
    
    auto dataBounds = _imData->GetExtent();

    float onePercent = (abs(dataMaximum - dataMinimum))/ 100;
    
    _upperThreshold = _imData->GetScalarComponentAsFloat(seedpoint[0], seedpoint[1], seedpoint[2], 0) + upperThreshold * onePercent;
    _lowerThreshold = _imData->GetScalarComponentAsFloat(seedpoint[0], seedpoint[1], seedpoint[2], 0) - lowerThreshold * onePercent;
    
    _thresholded = true;
    std::vector<unsigned int> selectionIndeces;
    int dim = 0;
   
    // Loop over the number of values in the pointsdata and write values into the dataArray if the current dimension  equals the chosen dimension and the selected indices.
    for (int i = 0; i < numPoints * numDimensions; i++) {
        // The remainder of the current value divided by the number of dimensions is the current dimension.
        dim = i % numDimensions;

        if (chosenDim == dim) {
            if (points.getValueAt(i) <= _upperThreshold && points.getValueAt(i) >= _lowerThreshold) {               
                selectionIndeces.push_back(i/numDimensions);               
            }
        }
    }
    points.setSelectionIndices(selectionIndeces);
   
    if (selectionIndeces.size() == 0) {
        _dataSelected = false;
    }
    else {
        _dataSelected = true;
    }
  
    return _imData;
}

void ViewerWidget::setClusterColor(const Dataset<Clusters>& clusterData) {
    _clusterData = clusterData;
    _clusterLoaded = true;
}