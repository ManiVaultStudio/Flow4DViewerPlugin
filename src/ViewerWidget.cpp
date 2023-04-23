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
            //_widget->setSelectedCell(cellID, xyz);

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
    _polyData(), _savedSpeedArray()

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
    
}

ViewerWidget::~ViewerWidget()
{

}

vtkSmartPointer<vtkImageData> ViewerWidget::setData(Points& data, int chosenDim, int boundsArray[2], std::string colorMap)
{
    int upperBound = boundsArray[1];
    int lowerBound = boundsArray[0];
    
    std::cout << boundsArray[0] << std::endl;
    std::cout << boundsArray[1] << std::endl;
    
    //chosenDim = 3;
	// Get number of points from points dataset.
	numPoints = data.getNumPoints();
    
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
    

        //for (int k = 0; k < numPoints / (lineSize); k++) {
        while(iterator/6 < numPoints){
            vtkSmartPointer<vtkPolyLine> polyLine = vtkSmartPointer<vtkPolyLine>::New();
           

            float currentLineIndex = data.getValueAt(iterator + 4);//lineIndex
            float nextLineIndex = currentLineIndex;
            int i = 0;
            //std::cout << currentLineIndex << "==" << nextLineIndex << std::endl;
            //std::cout <<  "pre loop" <<  std::endl;
            std::vector<std::vector<int>> savePoints;
            while (currentLineIndex == nextLineIndex)
            {
                if (data.getValueAt(iterator + 5) <= upperBound && data.getValueAt(iterator + 5) >= lowerBound) {
                    //std::cout << "im here2" << std::endl;

                    //std::cout << currentLineIndex << "==" << nextLineIndex << std::endl;
                    double p[3] = { data.getValueAt(iterator), data.getValueAt(iterator + 1), data.getValueAt(iterator + 2) };
                    dataArray->SetValue(pointCounter, data.getValueAt(iterator + chosenDim));

                    vtkPointObject->SetPoint(pointCounter, p);

                    savePoints.push_back({ i,pointCounter });
                    i++;
                }
                    //polyLine->GetPointIds()->SetId(i, pointCounter);
                    iterator = iterator + 6;
                    pointCounter++;
                    nextLineIndex = data.getValueAt(iterator + 4);
                    
                
            }
            polyLine->GetPointIds()->SetNumberOfIds(i);
            for (int i = 0; i < savePoints.size(); i++) {
                
                polyLine->GetPointIds()->SetId(savePoints[i][0], savePoints[i][1]);
            }
           
            cells->InsertNextCell(polyLine);

        }

        
    //vtkPointObject->SetData(dataArray);
    _polyData->SetPoints(vtkPointObject);
    _savedSpeedArray = dataArray;
    _polyData->GetPointData()->SetScalars(dataArray);
    
    _polyData->SetLines(cells);

   
    
   


	//// Create a vtkimagedata object of size xSize, ySize and zSize with vtk_float type vectors.
	vtkSmartPointer<vtkImageData> imData = vtkSmartPointer<vtkImageData>::New();
	//imData->SetDimensions(xSize, ySize, zSize);
	//imData->AllocateScalars(VTK_FLOAT, 1);


	return imData;
}
	
void ViewerWidget::renderData( vtkSmartPointer<vtkImageData> imData, std::string interpolationOption, std::string colorMap, bool shadingEnabled, std::vector<double> shadingParameters){
 
    
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

    

    // Loop to read in colors from the colormap qimage.
    if (_dataSelected)
    {
        color->AddRGBPoint(1, 1, 0, 0, 1, 1);
    }
    else {
        for (int pixelX = 0; pixelX < colorMapImage.width(); pixelX++) {
            const auto normalizedPixelX = static_cast<float>(pixelX) / static_cast<float>(colorMapImage.width());
            const auto pixelColor = colorMapImage.pixelColor(pixelX, 0);
            color->AddRGBPoint(normalizedPixelX * (dataMaximum - dataMinimum) + dataMinimum, pixelColor.redF(), pixelColor.greenF(), pixelColor.blueF());
        }

    }
    
    
    vtkSmartPointer<vtkPiecewiseFunction> colormapOpacity = vtkSmartPointer<vtkPiecewiseFunction>::New();
     //Set the opacity of the non-object voxels to 0.
    colormapOpacity->AddPoint(background, 0, 1, 1);

    
  

    vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    mapper->SetInputData(_polyData);
    mapper->SetLookupTable(color);
    
   
    


    vtkSmartPointer<vtkActor> volActor = vtkSmartPointer<vtkActor>::New();
    volActor->SetMapper(mapper);
    volActor->GetProperty()->SetOpacity(0.5);
    
    
 
	// Add the current volume to the renderer.
	mRenderer->AddViewProp(volActor);
   
	// Center camera.
	mRenderer->ResetCamera(); // Breaks here with subset visualization
    
	// Render.
	mRenderWindow->Render();
    
}

void ViewerWidget::setSelectedData(Points& points, std::vector<unsigned int, std::allocator<unsigned int>> selectionIndices, int chosenDim) {
      



        vtkSmartPointer<vtkFloatArray> dataArray = vtkSmartPointer<vtkFloatArray>::New();
        dataArray->SetNumberOfValues(_polyData->GetPointData()->GetScalars()->GetSize());
        

        for (int i = 0; i < dataArray->GetSize(); i++)
        {
            
            if(std::binary_search(selectionIndices.begin(), selectionIndices.end(),i))
            {
                
                dataArray->SetValue(i,1);
                
            }
            else
            {
                dataArray->SetValue(i, 0);
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


vtkSmartPointer<vtkImageData> ViewerWidget::connectedSelection(Points& points,int chosenDim, int *seedpoint, float upperThreshold, float lowerThreshold) {
    double dataMinimum = _imData->GetScalarRange()[0]+1;

    double dataMaximum = _imData->GetScalarRange()[1];
    std::cout << "connectselected" << std::endl;
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

//void ViewerWidget::setSelectedCell(int cellID, int *xyz) {
//    _selectedCell = cellID;   
//    _selectedCellCoordinate = xyz;
//    _Flow4DViewerPlugin.getRendererSettingsAction().getSelectedPointsAction().getPositionAction().changeValue(xyz);
//}