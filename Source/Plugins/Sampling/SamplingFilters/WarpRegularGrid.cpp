/* ============================================================================
 * Copyright (c) 2009-2016 BlueQuartz Software, LLC
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright notice, this
 * list of conditions and the following disclaimer in the documentation and/or
 * other materials provided with the distribution.
 *
 * Neither the name of BlueQuartz Software, the US Air Force, nor the names of its
 * contributors may be used to endorse or promote products derived from this software
 * without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 * USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * The code contained herein was partially funded by the following contracts:
 *    United States Air Force Prime Contract FA8650-07-D-5800
 *    United States Air Force Prime Contract FA8650-10-D-5210
 *    United States Prime Contract Navy N00173-07-C-2068
 *
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
#include "WarpRegularGrid.h"

#include <QtCore/QTextStream>

#include "SIMPLib/Common/Constants.h"
#include "SIMPLib/DataContainers/DataContainer.h"
#include "SIMPLib/DataContainers/DataContainerArray.h"
#include "SIMPLib/FilterParameters/AbstractFilterParametersReader.h"
#include "SIMPLib/FilterParameters/AttributeMatrixSelectionFilterParameter.h"
#include "SIMPLib/FilterParameters/DataContainerCreationFilterParameter.h"
#include "SIMPLib/FilterParameters/FourthOrderPolynomialFilterParameter.h"
#include "SIMPLib/FilterParameters/LinkedBooleanFilterParameter.h"
#include "SIMPLib/FilterParameters/LinkedChoicesFilterParameter.h"
#include "SIMPLib/FilterParameters/SecondOrderPolynomialFilterParameter.h"
#include "SIMPLib/FilterParameters/SeparatorFilterParameter.h"
#include "SIMPLib/FilterParameters/StringFilterParameter.h"
#include "SIMPLib/FilterParameters/ThirdOrderPolynomialFilterParameter.h"
#include "SIMPLib/Geometry/ImageGeom.h"

#include "Sampling/SamplingConstants.h"
#include "Sampling/SamplingVersion.h"

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
WarpRegularGrid::WarpRegularGrid()
{
  m_SecondOrderACoeff.c20 = 0.0f, m_SecondOrderACoeff.c02 = 0.0f, m_SecondOrderACoeff.c11 = 0.0f;
  m_SecondOrderACoeff.c10 = 1.0f, m_SecondOrderACoeff.c01 = 1.0f, m_SecondOrderACoeff.c00 = 0.0f;
  m_SecondOrderBCoeff.c20 = 0.0f, m_SecondOrderBCoeff.c02 = 0.0f, m_SecondOrderBCoeff.c11 = 0.0f;
  m_SecondOrderBCoeff.c10 = 1.0f, m_SecondOrderBCoeff.c01 = 1.0f, m_SecondOrderBCoeff.c00 = 0.0f;

  m_ThirdOrderACoeff.c30 = 0.0f, m_ThirdOrderACoeff.c03 = 0.0f, m_ThirdOrderACoeff.c21 = 0.0f, m_ThirdOrderACoeff.c12 = 0.0f, m_ThirdOrderACoeff.c20 = 0.0f;
  m_ThirdOrderACoeff.c02 = 0.0f, m_ThirdOrderACoeff.c11 = 0.0f, m_ThirdOrderACoeff.c10 = 1.0f;
  m_ThirdOrderACoeff.c01 = 1.0f, m_ThirdOrderACoeff.c00 = 0.0f;
  m_ThirdOrderBCoeff.c30 = 0.0f, m_ThirdOrderBCoeff.c03 = 0.0f, m_ThirdOrderBCoeff.c21 = 0.0f, m_ThirdOrderBCoeff.c12 = 0.0f, m_ThirdOrderBCoeff.c20 = 0.0f;
  m_ThirdOrderBCoeff.c02 = 0.0f, m_ThirdOrderBCoeff.c11 = 0.0f, m_ThirdOrderBCoeff.c10 = 1.0f;
  m_ThirdOrderBCoeff.c01 = 1.0f, m_ThirdOrderBCoeff.c00 = 0.0f;

  m_FourthOrderACoeff.c40 = 0.0f, m_FourthOrderACoeff.c04 = 0.0f, m_FourthOrderACoeff.c31 = 0.0f;
  m_FourthOrderACoeff.c13 = 0.0f, m_FourthOrderACoeff.c22 = 0.0f;
  m_FourthOrderACoeff.c30 = 0.0f, m_FourthOrderACoeff.c03 = 0.0f, m_FourthOrderACoeff.c21 = 0.0f, m_FourthOrderACoeff.c12 = 0.0f, m_FourthOrderACoeff.c20 = 0.0f;
  m_FourthOrderACoeff.c02 = 0.0f, m_FourthOrderACoeff.c11 = 0.0f, m_FourthOrderACoeff.c10 = 1.0f;
  m_FourthOrderACoeff.c01 = 1.0f, m_FourthOrderACoeff.c00 = 0.0f;
  m_FourthOrderBCoeff.c40 = 0.0f, m_FourthOrderBCoeff.c04 = 0.0f, m_FourthOrderBCoeff.c31 = 0.0f;
  m_FourthOrderBCoeff.c13 = 0.0f, m_FourthOrderBCoeff.c22 = 0.0f;
  m_FourthOrderBCoeff.c30 = 0.0f, m_FourthOrderBCoeff.c03 = 0.0f, m_FourthOrderBCoeff.c21 = 0.0f, m_FourthOrderBCoeff.c12 = 0.0f, m_FourthOrderBCoeff.c20 = 0.0f;
  m_FourthOrderBCoeff.c02 = 0.0f, m_FourthOrderBCoeff.c11 = 0.0f, m_FourthOrderBCoeff.c10 = 1.0f;
  m_FourthOrderBCoeff.c01 = 1.0f, m_FourthOrderBCoeff.c00 = 0.0f;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
WarpRegularGrid::~WarpRegularGrid() = default;

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void WarpRegularGrid::setupFilterParameters()
{
  FilterParameterVectorType parameters;

  {
    LinkedChoicesFilterParameter::Pointer parameter = LinkedChoicesFilterParameter::New();
    parameter->setHumanLabel("Polynomial Order");
    parameter->setPropertyName("PolyOrder");
    parameter->setSetterCallback(SIMPL_BIND_SETTER(WarpRegularGrid, this, PolyOrder));
    parameter->setGetterCallback(SIMPL_BIND_GETTER(WarpRegularGrid, this, PolyOrder));

    std::vector<QString> choices;
    choices.push_back("2nd");
    choices.push_back("3rd");
    choices.push_back("4th");
    parameter->setChoices(choices);
    std::vector<QString> linkedProps;
    linkedProps.push_back("SecondOrderACoeff");
    linkedProps.push_back("ThirdOrderACoeff");
    linkedProps.push_back("FourthOrderACoeff");
    linkedProps.push_back("SecondOrderBCoeff");
    linkedProps.push_back("ThirdOrderBCoeff");
    linkedProps.push_back("FourthOrderBCoeff");
    parameter->setLinkedProperties(linkedProps);
    parameter->setEditable(false);
    parameter->setCategory(FilterParameter::Category::Parameter);
    parameters.push_back(parameter);
  }
  parameters.push_back(SIMPL_NEW_SecondO_POLY_FP("Second Order A Coefficients", SecondOrderACoeff, FilterParameter::Category::Parameter, WarpRegularGrid, 0));
  parameters.push_back(SIMPL_NEW_SecondO_POLY_FP("Second Order B Coefficients", SecondOrderBCoeff, FilterParameter::Category::Parameter, WarpRegularGrid, 0));
  parameters.push_back(SIMPL_NEW_ThirdO_POLY_FP("Third Order A Coefficients", ThirdOrderACoeff, FilterParameter::Category::Parameter, WarpRegularGrid, 1));
  parameters.push_back(SIMPL_NEW_ThirdO_POLY_FP("Third Order B Coefficients", ThirdOrderBCoeff, FilterParameter::Category::Parameter, WarpRegularGrid, 1));
  parameters.push_back(SIMPL_NEW_FourthO_POLY_FP("Fourth Order A Coefficients", FourthOrderACoeff, FilterParameter::Category::Parameter, WarpRegularGrid, 2));
  parameters.push_back(SIMPL_NEW_FourthO_POLY_FP("Fourth Order B Coefficients", FourthOrderBCoeff, FilterParameter::Category::Parameter, WarpRegularGrid, 2));
  std::vector<QString> linkedProps;
  linkedProps.push_back("NewDataContainerName");
  parameters.push_back(SIMPL_NEW_LINKED_BOOL_FP("Save as New Data Container", SaveAsNewDataContainer, FilterParameter::Category::Parameter, WarpRegularGrid, linkedProps));
  parameters.push_back(SIMPL_NEW_DC_CREATION_FP("Data Container", NewDataContainerName, FilterParameter::Category::CreatedArray, WarpRegularGrid));
  parameters.push_back(SeparatorFilterParameter::Create("Cell Data", FilterParameter::Category::RequiredArray));
  {
    AttributeMatrixSelectionFilterParameter::RequirementType req = AttributeMatrixSelectionFilterParameter::CreateRequirement(AttributeMatrix::Type::Cell, IGeometry::Type::Image);
    parameters.push_back(SIMPL_NEW_AM_SELECTION_FP("Cell Attribute Matrix", CellAttributeMatrixPath, FilterParameter::Category::RequiredArray, WarpRegularGrid, req));
  }
  setFilterParameters(parameters);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void WarpRegularGrid::readFilterParameters(AbstractFilterParametersReader* reader, int index)
{
  reader->openFilterGroup(this, index);
  setNewDataContainerName(reader->readDataArrayPath("NewDataContainerName", getNewDataContainerName()));
  setCellAttributeMatrixPath(reader->readDataArrayPath("CellAttributeMatrixPath", getCellAttributeMatrixPath()));
  setSecondOrderACoeff(reader->readFloat2ndOrderPoly("SecondOrderACoeff", getSecondOrderACoeff()));
  setSecondOrderBCoeff(reader->readFloat2ndOrderPoly("SecondOrderBCoeff", getSecondOrderBCoeff()));
  setThirdOrderACoeff(reader->readFloat3rdOrderPoly("ThirdOrderACoeff", getThirdOrderACoeff()));
  setThirdOrderBCoeff(reader->readFloat3rdOrderPoly("ThirdOrderBCoeff", getThirdOrderBCoeff()));
  setFourthOrderACoeff(reader->readFloat4thOrderPoly("FourthOrderACoeff", getFourthOrderACoeff()));
  setFourthOrderBCoeff(reader->readFloat4thOrderPoly("FourthOrderBCoeff", getFourthOrderBCoeff()));
  setSaveAsNewDataContainer(reader->readValue("SaveAsNewDataContainer", getSaveAsNewDataContainer()));
  reader->closeFilterGroup();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void WarpRegularGrid::initialize()
{
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void WarpRegularGrid::dataCheck()
{

  clearErrorCode();
  clearWarningCode();

  getDataContainerArray()->getPrereqAttributeMatrixFromPath(this, getCellAttributeMatrixPath(), -301);

  if(!m_SaveAsNewDataContainer)
  {
    getDataContainerArray()->getPrereqGeometryFromDataContainer<ImageGeom>(this, getCellAttributeMatrixPath().getDataContainerName());
  }
  else
  {
    getDataContainerArray()->duplicateDataContainer(getCellAttributeMatrixPath().getDataContainerName(), getNewDataContainerName().getDataContainerName());
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void WarpRegularGrid::determine_warped_coordinates(float x, float y, float& newX, float& newY)
{
  if(m_PolyOrder == 0)
  {
    newX = x * x * m_SecondOrderACoeff.c20 + y * y * m_SecondOrderACoeff.c02 + x * y * m_SecondOrderACoeff.c11 + x * m_SecondOrderACoeff.c10 + y * m_SecondOrderACoeff.c01 + m_SecondOrderACoeff.c00;
    newY = x * x * m_SecondOrderBCoeff.c20 + y * y * m_SecondOrderBCoeff.c02 + x * y * m_SecondOrderBCoeff.c11 + x * m_SecondOrderBCoeff.c10 + y * m_SecondOrderBCoeff.c01 + m_SecondOrderBCoeff.c00;
  }
  if(m_PolyOrder == 1)
  {
  }
  if(m_PolyOrder == 2)
  {
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void WarpRegularGrid::execute()
{
  dataCheck();
  if(getErrorCode() < 0)
  {
    return;
  }

  DataContainer::Pointer m;
  if(!m_SaveAsNewDataContainer)
  {
    m = getDataContainerArray()->getDataContainer(getCellAttributeMatrixPath().getDataContainerName());
  }
  else
  {
    m = getDataContainerArray()->getDataContainer(getNewDataContainerName());
  }

  AttributeMatrix::Pointer cellAttrMat = m->getAttributeMatrix(getCellAttributeMatrixPath().getAttributeMatrixName());
  AttributeMatrix::Pointer newCellAttrMat = cellAttrMat->deepCopy(false);

  SizeVec3Type dims = m->getGeometryAs<ImageGeom>()->getDimensions();
  FloatVec3Type res = m->getGeometryAs<ImageGeom>()->getSpacing();
  size_t totalPoints = m->getGeometryAs<ImageGeom>()->getNumberOfElements();

  float x = 0.0f, y = 0.0f, z = 0.0f;
  float newX = 0.0f, newY = 0.0f;
  int col = 0.0f, row = 0.0f, plane = 0.0f;
  size_t index;
  size_t index_old;
  std::vector<size_t> newindicies(totalPoints);
  std::vector<bool> goodPoint(totalPoints, true);

  for(size_t i = 0; i < dims[2]; i++)
  {
    QString ss = QObject::tr("Warping Data - %1 Percent Complete").arg(((float)i / dims[2]) * 100);
    notifyStatusMessage(ss);
    for(size_t j = 0; j < dims[1]; j++)
    {
      for(size_t k = 0; k < dims[0]; k++)
      {
        x = static_cast<float>((k * res[0]));
        y = static_cast<float>((j * res[1]));
        z = static_cast<float>((i * res[2]));
        index = (i * dims[0] * dims[1]) + (j * dims[0]) + k;

        determine_warped_coordinates(x, y, newX, newY);
        col = newX / res[0];
        row = newY / res[1];
        plane = i;

        index_old = (plane * dims[0] * dims[1]) + (row * dims[0]) + col;
        newindicies[index] = index_old;
        if(col > 0 && col < dims[0] && row > 0 && row < dims[1])
        {
          goodPoint[index] = true;
        }
        else
        {
          goodPoint[index] = false;
        }
      }
    }
  }

  QList<QString> voxelArrayNames = cellAttrMat->getAttributeArrayNames();
  for(QList<QString>::iterator iter = voxelArrayNames.begin(); iter != voxelArrayNames.end(); ++iter)
  {
    IDataArray::Pointer p = cellAttrMat->getAttributeArray(*iter);
    // Make a copy of the 'p' array that has the same name. When placed into
    // the data container this will over write the current array with
    // the same name. At least in theory
    IDataArray::Pointer data = p->createNewArray(p->getNumberOfTuples(), p->getComponentDimensions(), p->getName());
    data->resizeTuples(totalPoints);
    void* source = nullptr;
    void* destination = nullptr;
    size_t newIndicies_I = 0;
    int nComp = data->getNumberOfComponents();
    for(size_t i = 0; i < static_cast<size_t>(totalPoints); i++)
    {
      newIndicies_I = newindicies[i];

      if(goodPoint[i])
      {
        source = p->getVoidPointer((nComp * newIndicies_I));
        destination = data->getVoidPointer((data->getNumberOfComponents() * i));
        ::memcpy(destination, source, p->getTypeSize() * data->getNumberOfComponents());
      }
      else
      {
        int var = 0;
        data->initializeTuple(i, &var);
      }
    }
    cellAttrMat->removeAttributeArray(*iter);
    newCellAttrMat->insertOrAssign(data);
  }
  m->removeAttributeMatrix(getCellAttributeMatrixPath().getAttributeMatrixName());
  m->addOrReplaceAttributeMatrix(newCellAttrMat);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
AbstractFilter::Pointer WarpRegularGrid::newFilterInstance(bool copyFilterParameters) const
{
  WarpRegularGrid::Pointer filter = WarpRegularGrid::New();
  if(copyFilterParameters)
  {
    copyFilterParameterInstanceVariables(filter.get());
  }
  return filter;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString WarpRegularGrid::getCompiledLibraryName() const
{
  return SamplingConstants::SamplingBaseName;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString WarpRegularGrid::getBrandingString() const
{
  return "Sampling";
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString WarpRegularGrid::getFilterVersion() const
{
  QString version;
  QTextStream vStream(&version);
  vStream << Sampling::Version::Major() << "." << Sampling::Version::Minor() << "." << Sampling::Version::Patch();
  return version;
}
// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString WarpRegularGrid::getGroupName() const
{
  return SIMPL::FilterGroups::SamplingFilters;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QUuid WarpRegularGrid::getUuid() const
{
  return QUuid("{520fc4c4-9c22-5520-9e75-a64b81a5a38d}");
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString WarpRegularGrid::getSubGroupName() const
{
  return SIMPL::FilterSubGroups::WarpingFilters;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString WarpRegularGrid::getHumanLabel() const
{
  return "Warp Rectilinear Grid";
}

// -----------------------------------------------------------------------------
WarpRegularGrid::Pointer WarpRegularGrid::NullPointer()
{
  return Pointer(static_cast<Self*>(nullptr));
}

// -----------------------------------------------------------------------------
std::shared_ptr<WarpRegularGrid> WarpRegularGrid::New()
{
  struct make_shared_enabler : public WarpRegularGrid
  {
  };
  std::shared_ptr<make_shared_enabler> val = std::make_shared<make_shared_enabler>();
  val->setupFilterParameters();
  return val;
}

// -----------------------------------------------------------------------------
QString WarpRegularGrid::getNameOfClass() const
{
  return QString("WarpRegularGrid");
}

// -----------------------------------------------------------------------------
QString WarpRegularGrid::ClassName()
{
  return QString("WarpRegularGrid");
}

// -----------------------------------------------------------------------------
void WarpRegularGrid::setNewDataContainerName(const DataArrayPath& value)
{
  m_NewDataContainerName = value;
}

// -----------------------------------------------------------------------------
DataArrayPath WarpRegularGrid::getNewDataContainerName() const
{
  return m_NewDataContainerName;
}

// -----------------------------------------------------------------------------
void WarpRegularGrid::setCellAttributeMatrixPath(const DataArrayPath& value)
{
  m_CellAttributeMatrixPath = value;
}

// -----------------------------------------------------------------------------
DataArrayPath WarpRegularGrid::getCellAttributeMatrixPath() const
{
  return m_CellAttributeMatrixPath;
}

// -----------------------------------------------------------------------------
void WarpRegularGrid::setPolyOrder(int value)
{
  m_PolyOrder = value;
}

// -----------------------------------------------------------------------------
int WarpRegularGrid::getPolyOrder() const
{
  return m_PolyOrder;
}

// -----------------------------------------------------------------------------
void WarpRegularGrid::setSecondOrderACoeff(const Float2ndOrderPolynomial& value)
{
  m_SecondOrderACoeff = value;
}

// -----------------------------------------------------------------------------
Float2ndOrderPolynomial WarpRegularGrid::getSecondOrderACoeff() const
{
  return m_SecondOrderACoeff;
}

// -----------------------------------------------------------------------------
void WarpRegularGrid::setSecondOrderBCoeff(const Float2ndOrderPolynomial& value)
{
  m_SecondOrderBCoeff = value;
}

// -----------------------------------------------------------------------------
Float2ndOrderPolynomial WarpRegularGrid::getSecondOrderBCoeff() const
{
  return m_SecondOrderBCoeff;
}

// -----------------------------------------------------------------------------
void WarpRegularGrid::setThirdOrderACoeff(const Float3rdOrderPoly_t& value)
{
  m_ThirdOrderACoeff = value;
}

// -----------------------------------------------------------------------------
Float3rdOrderPoly_t WarpRegularGrid::getThirdOrderACoeff() const
{
  return m_ThirdOrderACoeff;
}

// -----------------------------------------------------------------------------
void WarpRegularGrid::setThirdOrderBCoeff(const Float3rdOrderPoly_t& value)
{
  m_ThirdOrderBCoeff = value;
}

// -----------------------------------------------------------------------------
Float3rdOrderPoly_t WarpRegularGrid::getThirdOrderBCoeff() const
{
  return m_ThirdOrderBCoeff;
}

// -----------------------------------------------------------------------------
void WarpRegularGrid::setFourthOrderACoeff(const Float4thOrderPolynomial& value)
{
  m_FourthOrderACoeff = value;
}

// -----------------------------------------------------------------------------
Float4thOrderPolynomial WarpRegularGrid::getFourthOrderACoeff() const
{
  return m_FourthOrderACoeff;
}

// -----------------------------------------------------------------------------
void WarpRegularGrid::setFourthOrderBCoeff(const Float4thOrderPolynomial& value)
{
  m_FourthOrderBCoeff = value;
}

// -----------------------------------------------------------------------------
Float4thOrderPolynomial WarpRegularGrid::getFourthOrderBCoeff() const
{
  return m_FourthOrderBCoeff;
}

// -----------------------------------------------------------------------------
void WarpRegularGrid::setSaveAsNewDataContainer(bool value)
{
  m_SaveAsNewDataContainer = value;
}

// -----------------------------------------------------------------------------
bool WarpRegularGrid::getSaveAsNewDataContainer() const
{
  return m_SaveAsNewDataContainer;
}
