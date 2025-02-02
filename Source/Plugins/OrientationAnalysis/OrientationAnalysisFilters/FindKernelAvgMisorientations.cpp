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

#include "FindKernelAvgMisorientations.h"

#include <QtCore/QTextStream>

#include "SIMPLib/Common/Constants.h"
#include "SIMPLib/DataContainers/DataContainer.h"
#include "SIMPLib/DataContainers/DataContainerArray.h"
#include "SIMPLib/FilterParameters/AbstractFilterParametersReader.h"
#include "SIMPLib/FilterParameters/DataArraySelectionFilterParameter.h"
#include "SIMPLib/FilterParameters/IntVec3FilterParameter.h"
#include "SIMPLib/FilterParameters/LinkedPathCreationFilterParameter.h"
#include "SIMPLib/FilterParameters/SeparatorFilterParameter.h"
#include "SIMPLib/FilterParameters/StringFilterParameter.h"
#include "SIMPLib/Geometry/ImageGeom.h"
#include "SIMPLib/Math/SIMPLibMath.h"

#include "EbsdLib/Core/EbsdLibConstants.h"
#include "EbsdLib/Core/Orientation.hpp"
#include "EbsdLib/Core/OrientationTransformation.hpp"
#include "EbsdLib/Core/Quaternion.hpp"
#include "EbsdLib/LaueOps/LaueOps.h"
#include "OrientationAnalysis/OrientationAnalysisConstants.h"
#include "OrientationAnalysis/OrientationAnalysisVersion.h"

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
FindKernelAvgMisorientations::FindKernelAvgMisorientations()
{
  m_KernelSize[0] = 1;
  m_KernelSize[1] = 1;
  m_KernelSize[2] = 1;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
FindKernelAvgMisorientations::~FindKernelAvgMisorientations() = default;

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void FindKernelAvgMisorientations::setupFilterParameters()
{
  FilterParameterVectorType parameters;
  parameters.push_back(SIMPL_NEW_INT_VEC3_FP("Kernel Radius", KernelSize, FilterParameter::Category::Parameter, FindKernelAvgMisorientations));
  parameters.push_back(SeparatorFilterParameter::Create("Cell Data", FilterParameter::Category::RequiredArray));

  {
    DataArraySelectionFilterParameter::RequirementType req = DataArraySelectionFilterParameter::CreateRequirement(SIMPL::TypeNames::Int32, 1, AttributeMatrix::Type::Cell, IGeometry::Type::Image);
    parameters.push_back(SIMPL_NEW_DA_SELECTION_FP("Feature Ids", FeatureIdsArrayPath, FilterParameter::Category::RequiredArray, FindKernelAvgMisorientations, req));
  }
  {
    DataArraySelectionFilterParameter::RequirementType req = DataArraySelectionFilterParameter::CreateRequirement(SIMPL::TypeNames::Int32, 1, AttributeMatrix::Type::Cell, IGeometry::Type::Image);
    parameters.push_back(SIMPL_NEW_DA_SELECTION_FP("Phases", CellPhasesArrayPath, FilterParameter::Category::RequiredArray, FindKernelAvgMisorientations, req));
  }
  {
    DataArraySelectionFilterParameter::RequirementType req = DataArraySelectionFilterParameter::CreateRequirement(SIMPL::TypeNames::Float, 4, AttributeMatrix::Type::Cell, IGeometry::Type::Image);
    parameters.push_back(SIMPL_NEW_DA_SELECTION_FP("Quaternions", QuatsArrayPath, FilterParameter::Category::RequiredArray, FindKernelAvgMisorientations, req));
  }
  parameters.push_back(SeparatorFilterParameter::Create("Cell Ensemble Data", FilterParameter::Category::RequiredArray));
  {
    DataArraySelectionFilterParameter::RequirementType req =
        DataArraySelectionFilterParameter::CreateRequirement(SIMPL::TypeNames::UInt32, 1, AttributeMatrix::Type::CellEnsemble, IGeometry::Type::Image);
    parameters.push_back(SIMPL_NEW_DA_SELECTION_FP("Crystal Structures", CrystalStructuresArrayPath, FilterParameter::Category::RequiredArray, FindKernelAvgMisorientations, req));
  }
  parameters.push_back(SeparatorFilterParameter::Create("Cell Data", FilterParameter::Category::CreatedArray));
  parameters.push_back(SIMPL_NEW_DA_WITH_LINKED_AM_FP("Kernel Average Misorientations", KernelAverageMisorientationsArrayName, FeatureIdsArrayPath, FeatureIdsArrayPath,
                                                      FilterParameter::Category::CreatedArray, FindKernelAvgMisorientations));
  setFilterParameters(parameters);
}

// -----------------------------------------------------------------------------
void FindKernelAvgMisorientations::readFilterParameters(AbstractFilterParametersReader* reader, int index)
{
  reader->openFilterGroup(this, index);
  setKernelAverageMisorientationsArrayName(reader->readString("KernelAverageMisorientationsArrayName", getKernelAverageMisorientationsArrayName()));
  setQuatsArrayPath(reader->readDataArrayPath("QuatsArrayPath", getQuatsArrayPath()));
  setCrystalStructuresArrayPath(reader->readDataArrayPath("CrystalStructuresArrayPath", getCrystalStructuresArrayPath()));
  setCellPhasesArrayPath(reader->readDataArrayPath("CellPhasesArrayPath", getCellPhasesArrayPath()));
  setFeatureIdsArrayPath(reader->readDataArrayPath("FeatureIdsArrayPath", getFeatureIdsArrayPath()));
  setKernelSize(reader->readIntVec3("KernelSize", getKernelSize()));
  reader->closeFilterGroup();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void FindKernelAvgMisorientations::initialize()
{
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void FindKernelAvgMisorientations::dataCheck()
{
  clearErrorCode();
  clearWarningCode();
  DataArrayPath tempPath;

  getDataContainerArray()->getPrereqGeometryFromDataContainer<ImageGeom>(this, getFeatureIdsArrayPath().getDataContainerName());

  QVector<DataArrayPath> dataArrayPaths;

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

  m_CellPhasesPtr = getDataContainerArray()->getPrereqArrayFromPath<DataArray<int32_t>>(this, getCellPhasesArrayPath(), cDims);
  if(nullptr != m_CellPhasesPtr.lock())
  {
    m_CellPhases = m_CellPhasesPtr.lock()->getPointer(0);
  } /* Now assign the raw pointer to data from the DataArray<T> object */
  if(getErrorCode() >= 0)
  {
    dataArrayPaths.push_back(getCellPhasesArrayPath());
  }

  tempPath.update(m_FeatureIdsArrayPath.getDataContainerName(), getFeatureIdsArrayPath().getAttributeMatrixName(), getKernelAverageMisorientationsArrayName());
  m_KernelAverageMisorientationsPtr = getDataContainerArray()->createNonPrereqArrayFromPath<DataArray<float>>(this, tempPath, 0, cDims);
  if(nullptr != m_KernelAverageMisorientationsPtr.lock())
  {
    m_KernelAverageMisorientations = m_KernelAverageMisorientationsPtr.lock()->getPointer(0);
  } /* Now assign the raw pointer to data from the DataArray<T> object */

  m_CrystalStructuresPtr = getDataContainerArray()->getPrereqArrayFromPath<DataArray<uint32_t>>(this, getCrystalStructuresArrayPath(), cDims);
  if(nullptr != m_CrystalStructuresPtr.lock())
  {
    m_CrystalStructures = m_CrystalStructuresPtr.lock()->getPointer(0);
  } /* Now assign the raw pointer to data from the DataArray<T> object */

  cDims[0] = 4;
  m_QuatsPtr = getDataContainerArray()->getPrereqArrayFromPath<DataArray<float>>(this, getQuatsArrayPath(), cDims);
  if(nullptr != m_QuatsPtr.lock())
  {
    m_Quats = m_QuatsPtr.lock()->getPointer(0);
  } /* Now assign the raw pointer to data from the DataArray<T> object */
  if(getErrorCode() >= 0)
  {
    dataArrayPaths.push_back(getQuatsArrayPath());
  }

  getDataContainerArray()->validateNumberOfTuples(this, dataArrayPaths);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void FindKernelAvgMisorientations::execute()
{
  dataCheck();
  if(getErrorCode() < 0)
  {
    return;
  }

  DataContainer::Pointer m = getDataContainerArray()->getDataContainer(m_FeatureIdsArrayPath.getDataContainerName());

  std::vector<LaueOps::Pointer> m_OrientationOps = LaueOps::GetAllOrientationOps();
  FloatArrayType::Pointer quatPtr = m_QuatsPtr.lock();

  int32_t numVoxel = 0; // number of voxels in the feature...
  bool good = false;

  float totalmisorientation = 0.0f;

  uint32_t phase1 = EbsdLib::CrystalStructure::UnknownCrystalStructure;
  uint32_t phase2 = EbsdLib::CrystalStructure::UnknownCrystalStructure;
  SizeVec3Type udims = m->getGeometryAs<ImageGeom>()->getDimensions();

  int64_t xPoints = static_cast<int64_t>(udims[0]);
  int64_t yPoints = static_cast<int64_t>(udims[1]);
  int64_t zPoints = static_cast<int64_t>(udims[2]);
  int64_t point = 0;
  size_t neighbor = 0;
  int64_t jStride = 0;
  int64_t kStride = 0;
  float* currentQuatPtr = nullptr;
  for(int64_t col = 0; col < xPoints; col++)
  {
    for(int64_t row = 0; row < yPoints; row++)
    {
      for(int64_t plane = 0; plane < zPoints; plane++)
      {
        point = (plane * xPoints * yPoints) + (row * xPoints) + col;
        if(m_FeatureIds[point] > 0 && m_CellPhases[point] > 0)
        {
          totalmisorientation = 0.0f;
          numVoxel = 0;
          currentQuatPtr = quatPtr->getTuplePointer(point);
          QuatF q1(currentQuatPtr[0], currentQuatPtr[1], currentQuatPtr[2], currentQuatPtr[3]);

          phase1 = m_CrystalStructures[m_CellPhases[point]];
          for(int32_t j = -m_KernelSize[2]; j < m_KernelSize[2] + 1; j++)
          {
            jStride = j * xPoints * yPoints;
            for(int32_t k = -m_KernelSize[1]; k < m_KernelSize[1] + 1; k++)
            {
              kStride = k * xPoints;
              for(int32_t l = -m_KernelSize[0]; l < m_KernelSize[0] + 1; l++)
              {
                good = true;
                neighbor = point + (jStride) + (kStride) + (l);
                if(plane + j < 0)
                {
                  good = false;
                }
                else if(plane + j > zPoints - 1)
                {
                  good = false;
                }
                else if(row + k < 0)
                {
                  good = false;
                }
                else if(row + k > yPoints - 1)
                {
                  good = false;
                }
                else if(col + l < 0)
                {
                  good = false;
                }
                else if(col + l > xPoints - 1)
                {
                  good = false;
                }
                if(good && m_FeatureIds[point] == m_FeatureIds[neighbor])
                {
                  currentQuatPtr = quatPtr->getTuplePointer(neighbor);

                  QuatF q2(currentQuatPtr[0], currentQuatPtr[1], currentQuatPtr[2], currentQuatPtr[3]);
                  phase2 = m_CrystalStructures[m_CellPhases[neighbor]];
                  OrientationF axisAngle = m_OrientationOps[phase1]->calculateMisorientation(q1, q2);

                  totalmisorientation = totalmisorientation + (axisAngle[3] * SIMPLib::Constants::k_180OverPiD);
                  numVoxel++;
                }
              }
            }
          }
          m_KernelAverageMisorientations[point] = totalmisorientation / (float)numVoxel;
          if(numVoxel == 0)
          {
            m_KernelAverageMisorientations[point] = 0.0f;
          }
        }
        if(m_FeatureIds[point] == 0 || m_CellPhases[point] == 0)
        {
          m_KernelAverageMisorientations[point] = 0.0f;
        }
      }
    }
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
AbstractFilter::Pointer FindKernelAvgMisorientations::newFilterInstance(bool copyFilterParameters) const
{
  FindKernelAvgMisorientations::Pointer filter = FindKernelAvgMisorientations::New();
  if(copyFilterParameters)
  {
    copyFilterParameterInstanceVariables(filter.get());
  }
  return filter;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString FindKernelAvgMisorientations::getCompiledLibraryName() const
{
  return OrientationAnalysisConstants::OrientationAnalysisBaseName;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString FindKernelAvgMisorientations::getBrandingString() const
{
  return "OrientationAnalysis";
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString FindKernelAvgMisorientations::getFilterVersion() const
{
  QString version;
  QTextStream vStream(&version);
  vStream << OrientationAnalysis::Version::Major() << "." << OrientationAnalysis::Version::Minor() << "." << OrientationAnalysis::Version::Patch();
  return version;
}
// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString FindKernelAvgMisorientations::getGroupName() const
{
  return SIMPL::FilterGroups::StatisticsFilters;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QUuid FindKernelAvgMisorientations::getUuid() const
{
  return QUuid("{88d332c1-cf6c-52d3-a38d-22f6eae19fa6}");
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString FindKernelAvgMisorientations::getSubGroupName() const
{
  return SIMPL::FilterSubGroups::CrystallographyFilters;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString FindKernelAvgMisorientations::getHumanLabel() const
{
  return "Find Kernel Average Misorientations";
}

// -----------------------------------------------------------------------------
FindKernelAvgMisorientations::Pointer FindKernelAvgMisorientations::NullPointer()
{
  return Pointer(static_cast<Self*>(nullptr));
}

// -----------------------------------------------------------------------------
std::shared_ptr<FindKernelAvgMisorientations> FindKernelAvgMisorientations::New()
{
  struct make_shared_enabler : public FindKernelAvgMisorientations
  {
  };
  std::shared_ptr<make_shared_enabler> val = std::make_shared<make_shared_enabler>();
  val->setupFilterParameters();
  return val;
}

// -----------------------------------------------------------------------------
QString FindKernelAvgMisorientations::getNameOfClass() const
{
  return QString("FindKernelAvgMisorientations");
}

// -----------------------------------------------------------------------------
QString FindKernelAvgMisorientations::ClassName()
{
  return QString("FindKernelAvgMisorientations");
}

// -----------------------------------------------------------------------------
void FindKernelAvgMisorientations::setFeatureIdsArrayPath(const DataArrayPath& value)
{
  m_FeatureIdsArrayPath = value;
}

// -----------------------------------------------------------------------------
DataArrayPath FindKernelAvgMisorientations::getFeatureIdsArrayPath() const
{
  return m_FeatureIdsArrayPath;
}

// -----------------------------------------------------------------------------
void FindKernelAvgMisorientations::setCellPhasesArrayPath(const DataArrayPath& value)
{
  m_CellPhasesArrayPath = value;
}

// -----------------------------------------------------------------------------
DataArrayPath FindKernelAvgMisorientations::getCellPhasesArrayPath() const
{
  return m_CellPhasesArrayPath;
}

// -----------------------------------------------------------------------------
void FindKernelAvgMisorientations::setCrystalStructuresArrayPath(const DataArrayPath& value)
{
  m_CrystalStructuresArrayPath = value;
}

// -----------------------------------------------------------------------------
DataArrayPath FindKernelAvgMisorientations::getCrystalStructuresArrayPath() const
{
  return m_CrystalStructuresArrayPath;
}

// -----------------------------------------------------------------------------
void FindKernelAvgMisorientations::setQuatsArrayPath(const DataArrayPath& value)
{
  m_QuatsArrayPath = value;
}

// -----------------------------------------------------------------------------
DataArrayPath FindKernelAvgMisorientations::getQuatsArrayPath() const
{
  return m_QuatsArrayPath;
}

// -----------------------------------------------------------------------------
void FindKernelAvgMisorientations::setKernelAverageMisorientationsArrayName(const QString& value)
{
  m_KernelAverageMisorientationsArrayName = value;
}

// -----------------------------------------------------------------------------
QString FindKernelAvgMisorientations::getKernelAverageMisorientationsArrayName() const
{
  return m_KernelAverageMisorientationsArrayName;
}

// -----------------------------------------------------------------------------
void FindKernelAvgMisorientations::setKernelSize(const IntVec3Type& value)
{
  m_KernelSize = value;
}

// -----------------------------------------------------------------------------
IntVec3Type FindKernelAvgMisorientations::getKernelSize() const
{
  return m_KernelSize;
}
