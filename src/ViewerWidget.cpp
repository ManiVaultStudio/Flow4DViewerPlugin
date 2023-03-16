/** General headers */
#include <math.h>
#include <algorithm> 
#include <vector>
#include <cmath>
#include <iostream>
#include <vector>
#include <algorithm>
/** Plugin headers */
#include <ViewerWidget.h>
#include <RendererSettingsAction.h>
#include <Flow4DViewerPlugin.h>
/** HDPS headers */
#include <Dataset.h>
#include <PointData.h>
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



//#include <vtkIdTypeArray.h>
//#include <vtkSelectionNode.h>
//#include <vtkSelection.h>
//#include <vtkExtractSelection.h>
//#include <vtkUnstructuredGrid.h>

using namespace hdps;
using namespace hdps::gui;

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
            _widget->setSelectedCell(cellID, xyz);

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
	//mInteractorStyle(vtkSmartPointer<MouseInteractorStyle>::New()),
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
    _polyData()

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
    
}

ViewerWidget::~ViewerWidget()
{

}

vtkSmartPointer<vtkImageData> ViewerWidget::setData(Points& data, int chosenDim, std::string interpolationOption, std::string colorMap)
{
	// Get number of points from points dataset.
	numPoints = data.getNumPoints();
    std::cout << data.getValueAt(0) << std::endl;
	// Get number of dimensions from points dataset.
	numDimensions = 1;
    int lineSize = data.getProperty("lineSize").toInt();
    vtkSmartPointer<vtkPoints> vtkPointObject = vtkSmartPointer<vtkPoints>::New();
    vtkPointObject->SetNumberOfPoints(numPoints);
    int iterator = 0;
    vtkSmartPointer<vtkCellArray> cells = vtkSmartPointer<vtkCellArray>::New();
    vtkSmartPointer<vtkFloatArray> dataArray = vtkSmartPointer<vtkFloatArray>::New();
    dataArray->SetNumberOfValues(numPoints);
    int pointCounter = 0;
    for (int k = 0; k < numPoints / (lineSize); k++) {

        vtkSmartPointer<vtkPolyLine> polyLine = vtkSmartPointer<vtkPolyLine>::New();
        polyLine->GetPointIds()->SetNumberOfIds(lineSize);
        
        for (int i = 0; i < lineSize; i++) {
            double p[3] = { data.getValueAt(iterator), data.getValueAt(iterator + 1), data.getValueAt(iterator + 2) };
            dataArray->SetValue(pointCounter, data.getValueAt(iterator + 3));
            iterator = iterator + 4;
            vtkPointObject->SetPoint(pointCounter, p);
            
            polyLine->GetPointIds()->SetId(i, pointCounter);
            pointCounter++;
        }
        cells->InsertNextCell(polyLine);

    }

    
    //vtkPointObject->SetData(dataArray);
    _polyData->SetPoints(vtkPointObject);
    _polyData->GetPointData()->SetScalars(dataArray);

    _polyData->SetLines(cells);

   
    



	//// Create a vtkimagedata object of size xSize, ySize and zSize with vtk_float type vectors.
	vtkSmartPointer<vtkImageData> imData = vtkSmartPointer<vtkImageData>::New();
	//imData->SetDimensions(xSize, ySize, zSize);
	//imData->AllocateScalars(VTK_FLOAT, 1);


	return imData;
}
	
void ViewerWidget::renderData(vtkSmartPointer<vtkPlaneCollection> planeCollection, vtkSmartPointer<vtkImageData> imData, std::string interpolationOption, std::string colorMap, bool shadingEnabled, std::vector<double> shadingParameters){
 


    vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    mapper->SetInputData(_polyData);

    vtkSmartPointer<vtkActor> volActor = vtkSmartPointer<vtkActor>::New();
    volActor->SetMapper(mapper);
    volActor->GetProperty()->SetColor(1, 0, 0);
 
	// Add the current volume to the renderer.
	mRenderer->AddViewProp(volActor);
	// Center camera.
	mRenderer->ResetCamera();
	// Render.
	mRenderWindow->Render();
}

void ViewerWidget::setSelectedData(Points& points, std::vector<unsigned int, std::allocator<unsigned int>> selectionIndices, int chosenDim) {
        // Get x, y and z size from the points dataset.
        QVariant xQSize = points.getProperty("xDim");
        int xSize = xQSize.toInt();
        QVariant yQSize = points.getProperty("yDim");
        int ySize = yQSize.toInt();
        QVariant zQSize = points.getProperty("zDim");
        int zSize = zQSize.toInt();
        int dim;
        
        // Create bool variable to indicate if data has been selected.
        std::vector<bool> selected;
        points.selectedLocalIndices(selectionIndices, selected);
        // Count the number of selected datapoints.
        int count = std::count(selected.begin(), selected.end(), true);

        //Array to store selected image data.
        vtkSmartPointer<vtkFloatArray> dataArray = vtkSmartPointer<vtkFloatArray>::New();
        //Array to store which part of the image is part of the selection.
        vtkSmartPointer<vtkIntArray> labelArray = vtkSmartPointer<vtkIntArray>::New();

        // Set the number of values in the dataArray equal to the number of points in the pointsdataset.
        dataArray->SetNumberOfValues(numPoints);
        labelArray->SetNumberOfValues(numPoints);

        // Counter for the amount of values that have been read.
        int j = 0;
        // Counter for the number of selected datapoints to avoid overflowing selectionIndices vector.
        int numSelectedLoaded = 0;

        bool firstRead = false;

        auto backgroundValue = points.getValueAt(0);

        // Loop over the number of values in the pointsdata and find minimum values for current dimension to set background color.
        for (int i = 0; i < numPoints * numDimensions; i++) {
            // The remainder of the current value divided by the number of dimensions is the current dimension.
            dim = i % numDimensions;
            if (chosenDim == dim) {
                if (!firstRead || points.getValueAt(i) < backgroundValue) {
                    firstRead = true;
                    backgroundValue = points.getValueAt(i);
                }
            }
        }

        // Loop over the number of values in the pointsdata and write values into the dataArray if the current dimension  equals the chosen dimension and the selected indices.
        for (int i = 0; i < numPoints * numDimensions; i++) {
            // The remainder of the current value divided by the number of dimensions is the current dimension.
            dim = i % numDimensions;

            if (chosenDim == dim) {
                // Ensure that numSelectedLoaded does not overflow the slectionIndeces vector to avoid a crash.
                if (numSelectedLoaded != selectionIndices.size()) {
                    // If the index is equal to the current point in the array.
                    if (selected[i / numDimensions]) {
                        
                        // Write value into the dataArray.
                        dataArray->SetValue(j, points.getValueAt(i));
                        labelArray->SetValue(j, 1);
                        numSelectedLoaded++;
                    }
                    else {
                        // All other indices are non-Object.
                        dataArray->SetValue(j, backgroundValue);
                        labelArray->SetValue(j, 0);
                    }
                }
                else {
                    // All other indices are non-Object.
                    dataArray->SetValue(j, backgroundValue);
                    labelArray->SetValue(j, 0);
                }
                j++;
            }
        }

        // Add scalarData to the imageData object.
       
        _labelMap->GetPointData()->SetScalars(labelArray);

        if (selectionIndices.size() == 0) {
            _dataSelected = false;
        }
        else {
            _dataSelected = true;
        }
        // Return the selection imagedata object.   
}


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

void ViewerWidget::setSelectedCell(int cellID, int *xyz) {
    _selectedCell = cellID;   
    _selectedCellCoordinate = xyz;
    _Flow4DViewerPlugin.getRendererSettingsAction().getSelectedPointsAction().getPositionAction().changeValue(xyz);
}