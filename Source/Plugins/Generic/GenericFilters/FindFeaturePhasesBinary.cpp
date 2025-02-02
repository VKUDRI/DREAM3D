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
#include "FindFeaturePhasesBinary.h"

#include <QtCore/QTextStream>

#include "SIMPLib/Common/Constants.h"
#include "SIMPLib/DataContainers/DataContainerArray.h"
#include "SIMPLib/FilterParameters/AbstractFilterParametersReader.h"
#include "SIMPLib/FilterParameters/DataArrayCreationFilterParameter.h"
#include "SIMPLib/FilterParameters/DataArraySelectionFilterParameter.h"
#include "SIMPLib/FilterParameters/LinkedPathCreationFilterParameter.h"
#include "SIMPLib/FilterParameters/SeparatorFilterParameter.h"
#include "SIMPLib/FilterParameters/StringFilterParameter.h"
#include "SIMPLib/Geometry/ImageGeom.h"

#include "Generic/GenericConstants.h"
#include "Generic/GenericVersion.h"

/* Create Enumerations to allow the created Attribute Arrays to take part in renaming */
enum createdPathID : RenameDataPath::DataID_t
{
  AttributeMatrixID21 = 21,

  DataArrayID30 = 30,
  DataArrayID31 = 31,
};

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
FindFeaturePhasesBinary::FindFeaturePhasesBinary() = default;

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
FindFeaturePhasesBinary::~FindFeaturePhasesBinary() = default;

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void FindFeaturePhasesBinary::setupFilterParameters()
{
  FilterParameterVectorType parameters;
  parameters.push_back(SeparatorFilterParameter::Create("Cell Data", FilterParameter::Category::RequiredArray));
  {
    DataArraySelectionFilterParameter::RequirementType req = DataArraySelectionFilterParameter::CreateRequirement(SIMPL::TypeNames::Int32, 1, AttributeMatrix::Type::Cell, IGeometry::Type::Image);
    parameters.push_back(SIMPL_NEW_DA_SELECTION_FP("Feature Ids", FeatureIdsArrayPath, FilterParameter::Category::RequiredArray, FindFeaturePhasesBinary, req));
  }
  {
    DataArraySelectionFilterParameter::RequirementType req = DataArraySelectionFilterParameter::CreateRequirement(SIMPL::TypeNames::Bool, 1, AttributeMatrix::Type::Cell, IGeometry::Type::Image);
    parameters.push_back(SIMPL_NEW_DA_SELECTION_FP("Mask", GoodVoxelsArrayPath, FilterParameter::Category::RequiredArray, FindFeaturePhasesBinary, req));
  }
  parameters.push_back(SeparatorFilterParameter::Create("Cell Feature Data", FilterParameter::Category::CreatedArray));
  {
    DataArrayCreationFilterParameter::RequirementType req = DataArrayCreationFilterParameter::CreateRequirement(AttributeMatrix::Category::Any);
    req.dcGeometryTypes = IGeometry::Types(1, IGeometry::Type::Image);
    parameters.push_back(SIMPL_NEW_DA_CREATION_FP("Phases", FeaturePhasesArrayPath, FilterParameter::Category::CreatedArray, FindFeaturePhasesBinary, req));
  }
  parameters.push_back(SeparatorFilterParameter::Create("Cell Ensemble Data", FilterParameter::Category::CreatedArray));
  parameters.push_back(
      SIMPL_NEW_AM_WITH_LINKED_DC_FP("Cell Ensemble Attribute Matrix", CellEnsembleAttributeMatrixName, FeaturePhasesArrayPath, FilterParameter::Category::CreatedArray, FindFeaturePhasesBinary));
  setFilterParameters(parameters);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void FindFeaturePhasesBinary::readFilterParameters(AbstractFilterParametersReader* reader, int index)
{
  reader->openFilterGroup(this, index);
  setFeaturePhasesArrayPath(reader->readDataArrayPath("FeaturePhasesArrayPath", getFeaturePhasesArrayPath()));
  setGoodVoxelsArrayPath(reader->readDataArrayPath("GoodVoxelsArrayPath", getGoodVoxelsArrayPath()));
  setFeatureIdsArrayPath(reader->readDataArrayPath("FeatureIdsArrayPath", getFeatureIdsArrayPath()));
  setCellEnsembleAttributeMatrixName(reader->readString("CellEnsembleAttributeMatrixName", getCellEnsembleAttributeMatrixName()));
  reader->closeFilterGroup();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void FindFeaturePhasesBinary::initialize()
{
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void FindFeaturePhasesBinary::dataCheck()
{
  clearErrorCode();
  clearWarningCode();

  QVector<DataArrayPath> dataArrayPaths;

  getDataContainerArray()->getPrereqGeometryFromDataContainer<ImageGeom>(this, getFeatureIdsArrayPath().getDataContainerName());

  std::vector<size_t> cDims(1, 1);
  m_FeatureIdsPtr = getDataContainerArray()->getPrereqArrayFromPath<DataArray<int32_t>>(this, getFeatureIdsArrayPath(), cDims);
  if(nullptr != m_FeatureIdsPtr.lock())
  {
    m_FeatureIds = m_FeatureIdsPtr.lock()->getPointer(0);
  } /* Now assign the raw pointer to data from the DataArray<T> object */
  if(getErrorCode() >= 0)
  {
    dataArrayPaths.push_back(getFeatureIdsArrayPath());
  }

  m_GoodVoxelsPtr = getDataContainerArray()->getPrereqArrayFromPath<DataArray<bool>>(this, getGoodVoxelsArrayPath(), cDims);
  if(nullptr != m_GoodVoxelsPtr.lock())
  {
    m_GoodVoxels = m_GoodVoxelsPtr.lock()->getPointer(0);
  } /* Now assign the raw pointer to data from the DataArray<T> object */
  if(getErrorCode() >= 0)
  {
    dataArrayPaths.push_back(getGoodVoxelsArrayPath());
  }

  getDataContainerArray()->validateNumberOfTuples(this, dataArrayPaths);

  m_FeaturePhasesPtr = getDataContainerArray()->createNonPrereqArrayFromPath<DataArray<int32_t>>(this, getFeaturePhasesArrayPath(), 0, cDims, "", DataArrayID31);
  if(nullptr != m_FeaturePhasesPtr.lock())
  {
    m_FeaturePhases = m_FeaturePhasesPtr.lock()->getPointer(0);
  } /* Now assign the raw pointer to data from the DataArray<T> object */
  if(getErrorCode() < 0)
  {
    return;
  }

  std::vector<size_t> tDims(1, 2);
  QString cellFeatureDataContainer = getFeaturePhasesArrayPath().getDataContainerName();
  getDataContainerArray()
      ->getDataContainer(cellFeatureDataContainer)
      ->createNonPrereqAttributeMatrix(this, getCellEnsembleAttributeMatrixName(), tDims, AttributeMatrix::Type::CellEnsemble, AttributeMatrixID21);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void FindFeaturePhasesBinary::execute()
{
  dataCheck();
  if(getErrorCode() < 0)
  {
    return;
  }

  size_t totalPoints = m_FeatureIdsPtr.lock()->getNumberOfTuples();

  int32_t gnum = 0;
  for(size_t i = 0; i < totalPoints; i++)
  {
    gnum = m_FeatureIds[i];
    if(m_GoodVoxels[i])
    {
      m_FeaturePhases[gnum] = 1;
    }
    else
    {
      m_FeaturePhases[gnum] = 0;
    }
    // m_FeaturePhases[gnum] = m_CellPhases[i];
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
AbstractFilter::Pointer FindFeaturePhasesBinary::newFilterInstance(bool copyFilterParameters) const
{
  FindFeaturePhasesBinary::Pointer filter = FindFeaturePhasesBinary::New();
  if(copyFilterParameters)
  {
    copyFilterParameterInstanceVariables(filter.get());
  }
  return filter;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString FindFeaturePhasesBinary::getCompiledLibraryName() const
{
  return GenericConstants::GenericBaseName;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString FindFeaturePhasesBinary::getBrandingString() const
{
  return "Generic";
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString FindFeaturePhasesBinary::getFilterVersion() const
{
  QString version;
  QTextStream vStream(&version);
  vStream << Generic::Version::Major() << "." << Generic::Version::Minor() << "." << Generic::Version::Patch();
  return version;
}
// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString FindFeaturePhasesBinary::getGroupName() const
{
  return SIMPL::FilterGroups::Generic;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QUuid FindFeaturePhasesBinary::getUuid() const
{
  return QUuid("{64d20c7b-697c-5ff1-9d1d-8a27b071f363}");
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString FindFeaturePhasesBinary::getSubGroupName() const
{
  return SIMPL::FilterSubGroups::MorphologicalFilters;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString FindFeaturePhasesBinary::getHumanLabel() const
{
  return "Find Feature Phases Binary";
}

// -----------------------------------------------------------------------------
FindFeaturePhasesBinary::Pointer FindFeaturePhasesBinary::NullPointer()
{
  return Pointer(static_cast<Self*>(nullptr));
}

// -----------------------------------------------------------------------------
std::shared_ptr<FindFeaturePhasesBinary> FindFeaturePhasesBinary::New()
{
  struct make_shared_enabler : public FindFeaturePhasesBinary
  {
  };
  std::shared_ptr<make_shared_enabler> val = std::make_shared<make_shared_enabler>();
  val->setupFilterParameters();
  return val;
}

// -----------------------------------------------------------------------------
QString FindFeaturePhasesBinary::getNameOfClass() const
{
  return QString("FindFeaturePhasesBinary");
}

// -----------------------------------------------------------------------------
QString FindFeaturePhasesBinary::ClassName()
{
  return QString("FindFeaturePhasesBinary");
}

// -----------------------------------------------------------------------------
void FindFeaturePhasesBinary::setFeatureIdsArrayPath(const DataArrayPath& value)
{
  m_FeatureIdsArrayPath = value;
}

// -----------------------------------------------------------------------------
DataArrayPath FindFeaturePhasesBinary::getFeatureIdsArrayPath() const
{
  return m_FeatureIdsArrayPath;
}

// -----------------------------------------------------------------------------
void FindFeaturePhasesBinary::setGoodVoxelsArrayPath(const DataArrayPath& value)
{
  m_GoodVoxelsArrayPath = value;
}

// -----------------------------------------------------------------------------
DataArrayPath FindFeaturePhasesBinary::getGoodVoxelsArrayPath() const
{
  return m_GoodVoxelsArrayPath;
}

// -----------------------------------------------------------------------------
void FindFeaturePhasesBinary::setFeaturePhasesArrayPath(const DataArrayPath& value)
{
  m_FeaturePhasesArrayPath = value;
}

// -----------------------------------------------------------------------------
DataArrayPath FindFeaturePhasesBinary::getFeaturePhasesArrayPath() const
{
  return m_FeaturePhasesArrayPath;
}

// -----------------------------------------------------------------------------
void FindFeaturePhasesBinary::setCellEnsembleAttributeMatrixName(const QString& value)
{
  m_CellEnsembleAttributeMatrixName = value;
}

// -----------------------------------------------------------------------------
QString FindFeaturePhasesBinary::getCellEnsembleAttributeMatrixName() const
{
  return m_CellEnsembleAttributeMatrixName;
}
