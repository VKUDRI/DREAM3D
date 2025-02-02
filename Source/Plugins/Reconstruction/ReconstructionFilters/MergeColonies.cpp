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

#include "MergeColonies.h"

#include <chrono>
#include <cmath>
#include <random>

#include <QtCore/QTextStream>

#include "SIMPLib/Common/Constants.h"
#include "SIMPLib/DataContainers/DataContainer.h"
#include "SIMPLib/DataContainers/DataContainerArray.h"
#include "SIMPLib/FilterParameters/AbstractFilterParametersReader.h"
#include "SIMPLib/FilterParameters/DataArraySelectionFilterParameter.h"
#include "SIMPLib/FilterParameters/FloatFilterParameter.h"
#include "SIMPLib/FilterParameters/LinkedBooleanFilterParameter.h"
#include "SIMPLib/FilterParameters/LinkedPathCreationFilterParameter.h"
#include "SIMPLib/FilterParameters/SeparatorFilterParameter.h"
#include "SIMPLib/FilterParameters/StringFilterParameter.h"
#include "SIMPLib/Math/GeometryMath.h"
#include "SIMPLib/Math/MatrixMath.h"
#include "SIMPLib/Math/SIMPLibMath.h"
#include "SIMPLib/Math/SIMPLibRandom.h"

#include "EbsdLib/Core/EbsdLibConstants.h"
#include "EbsdLib/LaueOps/LaueOps.h"

#include "Reconstruction/ReconstructionConstants.h"
#include "Reconstruction/ReconstructionVersion.h"

/* Create Enumerations to allow the created Attribute Arrays to take part in renaming */
enum createdPathID : RenameDataPath::DataID_t
{
  AttributeMatrixID21 = 21,

  DataArrayID30 = 30,
  DataArrayID31 = 31,
};

namespace
{
const double unit110 = 1.0 / std::sqrt(2.0);
const double unit111 = 1.0 / std::sqrt(3.0);
const double unit112_1 = 1.0 / std::sqrt(6.0);
const double unit112_2 = 2.0 / std::sqrt(6.0);

double crystalDirections[12][3][3] = {{{unit111, unit112_1, unit110}, {-unit111, -unit112_1, unit110}, {unit111, -unit112_2, 0}},

                                      {{-unit111, unit112_1, unit110}, {unit111, -unit112_1, unit110}, {unit111, unit112_2, 0}},

                                      {{unit111, -unit112_1, unit110}, {unit111, -unit112_1, -unit110}, {unit111, unit112_2, 0}},

                                      {{unit111, unit112_1, unit110}, {unit111, unit112_1, -unit110}, {-unit111, unit112_2, 0}},

                                      {{unit111, unit112_1, unit110}, {unit111, -unit112_2, 0}, {unit111, unit112_1, -unit110}},

                                      {{unit111, -unit112_1, unit110}, {-unit111, -unit112_2, 0}, {unit111, -unit112_1, -unit110}},

                                      {{unit111, -unit112_1, unit110}, {unit111, unit112_2, 0}, {-unit111, unit112_1, unit110}},

                                      {{-unit111, -unit112_1, unit110}, {unit111, -unit112_2, 0}, {unit111, unit112_1, unit110}},

                                      {{unit111, -unit112_2, 0}, {unit111, unit112_1, unit110}, {-unit111, -unit112_1, unit110}},

                                      {{unit111, unit112_2, 0}, {-unit111, unit112_1, unit110}, {unit111, -unit112_1, unit110}},

                                      {{unit111, unit112_2, 0}, {unit111, -unit112_1, unit110}, {unit111, -unit112_1, -unit110}},

                                      {{-unit111, unit112_2, 0}, {unit111, unit112_1, unit110}, {unit111, unit112_1, -unit110}}};
} // namespace

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
MergeColonies::MergeColonies()
: m_NewCellFeatureAttributeMatrixName("NewFeatureData")
, m_FeatureIdsArrayPath("", "", "")
, m_CellPhasesArrayPath("", "", "")
, m_FeaturePhasesArrayPath("", "", "")
, m_AvgQuatsArrayPath("", "", "")
, m_CrystalStructuresArrayPath("", "", "")
, m_CellParentIdsArrayName(SIMPL::CellData::ParentIds)
, m_GlobAlphaArrayName(SIMPL::CellData::GlobAlpha)
, m_FeatureParentIdsArrayName(SIMPL::FeatureData::ParentIds)
, m_ActiveArrayName(SIMPL::FeatureData::Active)
, m_AxisTolerance(1.0f)
, m_AngleTolerance(1.0f)
, m_RandomizeParentIds(true)
, m_IdentifyGlobAlpha(false)
{
  m_OrientationOps = LaueOps::GetAllOrientationOps();

  m_AxisToleranceRad = 0.0f;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
MergeColonies::~MergeColonies() = default;

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void MergeColonies::setupFilterParameters()
{
  GroupFeatures::setupFilterParameters();

  FilterParameterVectorType parameters = getFilterParameters();
  parameters.push_back(SIMPL_NEW_FLOAT_FP("Axis Tolerance (Degrees)", AxisTolerance, FilterParameter::Category::Parameter, MergeColonies));
  parameters.push_back(SIMPL_NEW_FLOAT_FP("Angle Tolerance (Degrees)", AngleTolerance, FilterParameter::Category::Parameter, MergeColonies));
  std::vector<QString> linkedProps = {"GlobAlphaArrayName"};
  parameters.push_back(SIMPL_NEW_LINKED_BOOL_FP("Identify Glob Alpha", IdentifyGlobAlpha, FilterParameter::Category::Parameter, MergeColonies, linkedProps));
  {
    DataArraySelectionFilterParameter::RequirementType req = DataArraySelectionFilterParameter::CreateCategoryRequirement(SIMPL::TypeNames::Int32, 1, AttributeMatrix::Category::Feature);
    parameters.push_back(SIMPL_NEW_DA_SELECTION_FP("Feature Phases", FeaturePhasesArrayPath, FilterParameter::Category::RequiredArray, MergeColonies, req));
  }
  {
    DataArraySelectionFilterParameter::RequirementType req = DataArraySelectionFilterParameter::CreateCategoryRequirement(SIMPL::TypeNames::Float, 4, AttributeMatrix::Category::Feature);
    parameters.push_back(SIMPL_NEW_DA_SELECTION_FP("Average Quaternions", AvgQuatsArrayPath, FilterParameter::Category::RequiredArray, MergeColonies, req));
  }
  parameters.push_back(SeparatorFilterParameter::Create("Element Data", FilterParameter::Category::RequiredArray));
  {
    DataArraySelectionFilterParameter::RequirementType req = DataArraySelectionFilterParameter::CreateCategoryRequirement(SIMPL::TypeNames::Int32, 1, AttributeMatrix::Category::Element);
    parameters.push_back(SIMPL_NEW_DA_SELECTION_FP("Feature Ids", FeatureIdsArrayPath, FilterParameter::Category::RequiredArray, MergeColonies, req));
  }
  {
    DataArraySelectionFilterParameter::RequirementType req = DataArraySelectionFilterParameter::CreateCategoryRequirement(SIMPL::TypeNames::Int32, 1, AttributeMatrix::Category::Element);
    parameters.push_back(SIMPL_NEW_DA_SELECTION_FP("Phases", CellPhasesArrayPath, FilterParameter::Category::RequiredArray, MergeColonies, req));
  }
  parameters.push_back(SeparatorFilterParameter::Create("Ensemble Data", FilterParameter::Category::RequiredArray));
  {
    DataArraySelectionFilterParameter::RequirementType req = DataArraySelectionFilterParameter::CreateCategoryRequirement(SIMPL::TypeNames::UInt32, 1, AttributeMatrix::Category::Ensemble);
    parameters.push_back(SIMPL_NEW_DA_SELECTION_FP("Crystal Structures", CrystalStructuresArrayPath, FilterParameter::Category::RequiredArray, MergeColonies, req));
  }
  parameters.push_back(SeparatorFilterParameter::Create("Element Data", FilterParameter::Category::CreatedArray));
  parameters.push_back(SIMPL_NEW_DA_WITH_LINKED_AM_FP("Parent Ids", CellParentIdsArrayName, FeatureIdsArrayPath, FeatureIdsArrayPath, FilterParameter::Category::CreatedArray, MergeColonies));
  parameters.push_back(SIMPL_NEW_DA_WITH_LINKED_AM_FP("Glob Alpha", GlobAlphaArrayName, FeatureIdsArrayPath, FeatureIdsArrayPath, FilterParameter::Category::CreatedArray, MergeColonies));
  parameters.push_back(SeparatorFilterParameter::Create("Feature Data", FilterParameter::Category::CreatedArray));
  parameters.push_back(SIMPL_NEW_AM_WITH_LINKED_DC_FP("Feature Attribute Matrix", NewCellFeatureAttributeMatrixName, FeatureIdsArrayPath, FilterParameter::Category::CreatedArray, MergeColonies));
  parameters.push_back(SIMPL_NEW_DA_WITH_LINKED_AM_FP("Parent Ids", FeatureParentIdsArrayName, FeaturePhasesArrayPath, FeaturePhasesArrayPath, FilterParameter::Category::CreatedArray, MergeColonies));
  parameters.push_back(SIMPL_NEW_DA_WITH_LINKED_AM_FP("Active", ActiveArrayName, FeatureIdsArrayPath, NewCellFeatureAttributeMatrixName, FilterParameter::Category::CreatedArray, MergeColonies));
  setFilterParameters(parameters);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void MergeColonies::readFilterParameters(AbstractFilterParametersReader* reader, int index)
{
  GroupFeatures::readFilterParameters(reader, index);
  reader->openFilterGroup(this, index);
  setNewCellFeatureAttributeMatrixName(reader->readString("NewCellFeatureAttributeMatrixName", getNewCellFeatureAttributeMatrixName()));
  setActiveArrayName(reader->readString("ActiveArrayName", getActiveArrayName()));
  setFeatureParentIdsArrayName(reader->readString("FeatureParentIdsArrayName", getFeatureParentIdsArrayName()));
  setGlobAlphaArrayName(reader->readString("GlobAlphaArrayName", getGlobAlphaArrayName()));
  setCellParentIdsArrayName(reader->readString("CellParentIdsArrayName", getCellParentIdsArrayName()));
  setCrystalStructuresArrayPath(reader->readDataArrayPath("CrystalStructuresArrayPath", getCrystalStructuresArrayPath()));
  setAvgQuatsArrayPath(reader->readDataArrayPath("AvgQuatsArrayPath", getAvgQuatsArrayPath()));
  setFeaturePhasesArrayPath(reader->readDataArrayPath("FeaturePhasesArrayPath", getFeaturePhasesArrayPath()));
  setCellPhasesArrayPath(reader->readDataArrayPath("CellPhasesArrayPath", getCellPhasesArrayPath()));
  setFeatureIdsArrayPath(reader->readDataArrayPath("FeatureIdsArrayPath", getFeatureIdsArrayPath()));
  setAxisTolerance(reader->readValue("AxisTolerance", getAxisTolerance()));
  setAngleTolerance(reader->readValue("AngleTolerance", getAngleTolerance()));
  setIdentifyGlobAlpha(reader->readValue("IdentifyGlobAlpha", getIdentifyGlobAlpha()));
  reader->closeFilterGroup();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void MergeColonies::updateFeatureInstancePointers()
{
  clearErrorCode();
  clearWarningCode();

  if(nullptr != m_ActivePtr.lock())
  {
    m_Active = m_ActivePtr.lock()->getPointer(0);
  } /* Now assign the raw pointer to data from the DataArray<T> object */
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void MergeColonies::initialize()
{
  m_AxisToleranceRad = 0.0f;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void MergeColonies::dataCheck()
{
  clearErrorCode();
  clearWarningCode();
  initialize();

  DataArrayPath tempPath;

  GroupFeatures::dataCheck();
  if(getErrorCode() < 0)
  {
    return;
  }

  DataContainer::Pointer m = getDataContainerArray()->getPrereqDataContainer(this, m_FeatureIdsArrayPath.getDataContainerName(), false);
  if(getErrorCode() < 0 || nullptr == m.get())
  {
    return;
  }

  std::vector<size_t> tDims(1, 0);
  m->createNonPrereqAttributeMatrix(this, getNewCellFeatureAttributeMatrixName(), tDims, AttributeMatrix::Type::CellFeature, AttributeMatrixID21);

  std::vector<size_t> cDims(1, 1);

  QVector<DataArrayPath> cellDataArrayPaths;
  QVector<DataArrayPath> featureDataArrayPaths;

  // Cell Data
  m_FeatureIdsPtr = getDataContainerArray()->getPrereqArrayFromPath<DataArray<int32_t>>(this, getFeatureIdsArrayPath(), cDims);
  if(nullptr != m_FeatureIdsPtr.lock())
  {
    m_FeatureIds = m_FeatureIdsPtr.lock()->getPointer(0);
  } /* Now assign the raw pointer to data from the DataArray<T> object */
  if(getErrorCode() >= 0)
  {
    cellDataArrayPaths.push_back(getFeatureIdsArrayPath());
  }

  m_CellPhasesPtr = getDataContainerArray()->getPrereqArrayFromPath<DataArray<int32_t>>(this, getCellPhasesArrayPath(), cDims);
  if(nullptr != m_CellPhasesPtr.lock())
  {
    m_CellPhases = m_CellPhasesPtr.lock()->getPointer(0);
  } /* Now assign the raw pointer to data from the DataArray<T> object */
  if(getErrorCode() >= 0)
  {
    cellDataArrayPaths.push_back(getCellPhasesArrayPath());
  }

  tempPath.update(getFeatureIdsArrayPath().getDataContainerName(), getFeatureIdsArrayPath().getAttributeMatrixName(), getCellParentIdsArrayName());
  m_CellParentIdsPtr = getDataContainerArray()->createNonPrereqArrayFromPath<DataArray<int32_t>>(this, tempPath, -1, cDims);
  if(nullptr != m_CellParentIdsPtr.lock())
  {
    m_CellParentIds = m_CellParentIdsPtr.lock()->getPointer(0);
  } /* Now assign the raw pointer to data from the DataArray<T> object */

  if(m_IdentifyGlobAlpha)
  {
    tempPath.update(getFeatureIdsArrayPath().getDataContainerName(), getFeatureIdsArrayPath().getAttributeMatrixName(), getGlobAlphaArrayName());
    m_GlobAlphaPtr = getDataContainerArray()->createNonPrereqArrayFromPath<DataArray<int32_t>>(this, tempPath, 0, cDims);
    if(nullptr != m_GlobAlphaPtr.lock())
    {
      m_GlobAlpha = m_GlobAlphaPtr.lock()->getPointer(0);
    } /* Now assign the raw pointer to data from the DataArray<T> object */
  }

  // Feature Data
  m_FeaturePhasesPtr = getDataContainerArray()->getPrereqArrayFromPath<DataArray<int32_t>>(this, getFeaturePhasesArrayPath(), cDims);
  if(nullptr != m_FeaturePhasesPtr.lock())
  {
    m_FeaturePhases = m_FeaturePhasesPtr.lock()->getPointer(0);
  } /* Now assign the raw pointer to data from the DataArray<T> object */
  if(getErrorCode() >= 0)
  {
    featureDataArrayPaths.push_back(getFeaturePhasesArrayPath());
  }

  tempPath.update(getFeaturePhasesArrayPath().getDataContainerName(), getFeaturePhasesArrayPath().getAttributeMatrixName(), getFeatureParentIdsArrayName());
  m_FeatureParentIdsPtr = getDataContainerArray()->createNonPrereqArrayFromPath<DataArray<int32_t>>(this, tempPath, -1, cDims);
  if(nullptr != m_FeatureParentIdsPtr.lock())
  {
    m_FeatureParentIds = m_FeatureParentIdsPtr.lock()->getPointer(0);
  } /* Now assign the raw pointer to data from the DataArray<T> object */

  cDims[0] = 4;
  m_AvgQuatsPtr = getDataContainerArray()->getPrereqArrayFromPath<DataArray<float>>(this, getAvgQuatsArrayPath(), cDims);
  if(nullptr != m_AvgQuatsPtr.lock())
  {
    m_AvgQuats = m_AvgQuatsPtr.lock()->getPointer(0);
  } /* Now assign the raw pointer to data from the DataArray<T> object */
  if(getErrorCode() >= 0)
  {
    featureDataArrayPaths.push_back(getAvgQuatsArrayPath());
  }

  // NewFeature Data
  cDims[0] = 1;
  tempPath.update(m_FeatureIdsArrayPath.getDataContainerName(), getNewCellFeatureAttributeMatrixName(), getActiveArrayName());
  m_ActivePtr = getDataContainerArray()->createNonPrereqArrayFromPath<DataArray<bool>>(this, tempPath, true, cDims, "", DataArrayID31);
  if(nullptr != m_ActivePtr.lock())
  {
    m_Active = m_ActivePtr.lock()->getPointer(0);
  } /* Now assign the raw pointer to data from the DataArray<T> object */

  // Ensemble Data
  m_CrystalStructuresPtr = getDataContainerArray()->getPrereqArrayFromPath<DataArray<unsigned int>>(this, getCrystalStructuresArrayPath(), cDims);
  if(nullptr != m_CrystalStructuresPtr.lock())
  {
    m_CrystalStructures = m_CrystalStructuresPtr.lock()->getPointer(0);
  } /* Now assign the raw pointer to data from the DataArray<T> object */
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int32_t MergeColonies::getSeed(int32_t newFid)
{
  clearErrorCode();
  clearWarningCode();

  int32_t numfeatures = static_cast<int32_t>(m_FeaturePhasesPtr.lock()->getNumberOfTuples());

  SIMPL_RANDOMNG_NEW()
  int32_t seed = -1;
  int32_t randfeature = 0;

  // Precalculate some constants
  int32_t totalFMinus1 = numfeatures - 1;

  size_t counter = 0;
  randfeature = int32_t(float(rg.genrand_res53()) * float(totalFMinus1));
  while(seed == -1 && counter < numfeatures)
  {
    if(randfeature > totalFMinus1)
    {
      randfeature = randfeature - numfeatures;
    }
    if(m_FeatureParentIds[randfeature] == -1)
    {
      seed = randfeature;
    }
    randfeature++;
    counter++;
  }
  if(seed >= 0)
  {
    m_FeatureParentIds[seed] = newFid;
    std::vector<size_t> tDims(1, newFid + 1);
    getDataContainerArray()->getDataContainer(m_FeatureIdsArrayPath.getDataContainerName())->getAttributeMatrix(getNewCellFeatureAttributeMatrixName())->resizeAttributeArrays(tDims);
    updateFeatureInstancePointers();
  }
  return seed;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
bool MergeColonies::determineGrouping(int32_t referenceFeature, int32_t neighborFeature, int32_t newFid)
{
  double w = 0.0f;
  bool colony = false;

  // QuatF* avgQuats = reinterpret_cast<QuatF*>(m_AvgQuats);

  if(m_FeatureParentIds[neighborFeature] == -1 && m_FeaturePhases[referenceFeature] > 0 && m_FeaturePhases[neighborFeature] > 0)
  {
    w = std::numeric_limits<double>::max();
    float* avgQuatPtr = m_AvgQuats + referenceFeature * 4;
    QuatD q1(avgQuatPtr[0], avgQuatPtr[1], avgQuatPtr[2], avgQuatPtr[3]);
    avgQuatPtr = m_AvgQuats + neighborFeature * 4;
    QuatD q2(avgQuatPtr[0], avgQuatPtr[1], avgQuatPtr[2], avgQuatPtr[3]);

    uint32_t phase1 = m_CrystalStructures[m_FeaturePhases[referenceFeature]];
    uint32_t phase2 = m_CrystalStructures[m_FeaturePhases[neighborFeature]];
    if(phase1 == phase2 && (phase1 == EbsdLib::CrystalStructure::Hexagonal_High))
    {
      OrientationD ax = m_OrientationOps[phase1]->calculateMisorientation(q1, q2);

      OrientationD rod = OrientationTransformation::ax2ro<OrientationD, OrientationD>(ax);
      rod = m_OrientationOps[phase1]->getMDFFZRod(rod);
      ax = OrientationTransformation::ro2ax<OrientationD, OrientationD>(rod);

      w = ax[3] * (SIMPLib::Constants::k_180OverPiD);
      float angdiff1 = std::fabs(w - 10.53f);
      float axisdiff1 = std::acos(/*std::fabs(n1) * 0.0000f + std::fabs(n2) * 0.0000f +*/ std::fabs(ax[2]) /* * 1.0000f */);
      if(angdiff1 < m_AngleTolerance && axisdiff1 < m_AxisToleranceRad)
      {
        colony = true;
      }
      float angdiff2 = std::fabs(w - 90.00f);
      float axisdiff2 = std::acos(std::fabs(ax[0]) * 0.9958f + std::fabs(ax[1]) * 0.0917f /* + std::fabs(n3) * 0.0000f */);
      if(angdiff2 < m_AngleTolerance && axisdiff2 < m_AxisToleranceRad)
      {
        colony = true;
      }
      float angdiff3 = std::fabs(w - 60.00f);
      float axisdiff3 = std::acos(std::fabs(ax[0]) /* * 1.0000f + std::fabs(n2) * 0.0000f + std::fabs(n3) * 0.0000f*/);
      if(angdiff3 < m_AngleTolerance && axisdiff3 < m_AxisToleranceRad)
      {
        colony = true;
      }
      float angdiff4 = std::fabs(w - 60.83f);
      float axisdiff4 = std::acos(std::fabs(ax[0]) * 0.9834f + std::fabs(ax[1]) * 0.0905f + std::fabs(ax[2]) * 0.1570f);
      if(angdiff4 < m_AngleTolerance && axisdiff4 < m_AxisToleranceRad)
      {
        colony = true;
      }
      float angdiff5 = std::fabs(w - 63.26f);
      float axisdiff5 = std::acos(std::fabs(ax[0]) * 0.9549f /* + std::fabs(n2) * 0.0000f */ + std::fabs(ax[2]) * 0.2969f);
      if(angdiff5 < m_AngleTolerance && axisdiff5 < m_AxisToleranceRad)
      {
        colony = true;
      }
      if(colony)
      {
        m_FeatureParentIds[neighborFeature] = newFid;
        return true;
      }
    }
    else if(EbsdLib::CrystalStructure::Cubic_High == phase2 && EbsdLib::CrystalStructure::Hexagonal_High == phase1)
    {
      colony = check_for_burgers(q2, q1);
      if(colony)
      {
        m_FeatureParentIds[neighborFeature] = newFid;
        return true;
      }
    }
    else if(EbsdLib::CrystalStructure::Cubic_High == phase1 && EbsdLib::CrystalStructure::Hexagonal_High == phase2)
    {
      colony = check_for_burgers(q1, q2);
      if(colony)
      {
        m_FeatureParentIds[neighborFeature] = newFid;
        return true;
      }
    }
  }
  return false;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void MergeColonies::characterize_colonies()
{
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
bool MergeColonies::check_for_burgers(const QuatD& betaQuat, const QuatD& alphaQuat) const
{
  double dP = 0.0;
  double angle = 0.0;
  double radToDeg = 180.0 / SIMPLib::Constants::k_PiD;

  double gBeta[3][3] = {{0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}};
  double gBetaT[3][3] = {{0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}};
  OrientationTransformation::qu2om<QuatD, OrientationD>(betaQuat).toGMatrix(gBeta);
  // transpose gBeta so the sample direction is the output when
  // gBeta is multiplied by the crystal directions below
  MatrixMath::Transpose3x3(gBeta, gBetaT);

  double gAlpha[3][3] = {{0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}};
  double gAlphaT[3][3] = {{0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}};
  OrientationTransformation::qu2om<QuatD, OrientationD>(alphaQuat).toGMatrix(gAlpha);
  // transpose gBeta so the sample direction is the output when
  // gBeta is multiplied by the crystal directions below
  MatrixMath::Transpose3x3(gAlpha, gAlphaT);

  double mat[3][3] = {{0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}};
  double a[3] = {0.0, 0.0, 0.0};
  double b[3] = {0.0, 0.0, 0.0};
  for(int32_t i = 0; i < 12; i++)
  {
    MatrixMath::Multiply3x3with3x3(gBetaT, crystalDirections[i], mat);
    a[0] = mat[0][2];
    a[1] = mat[1][2];
    a[2] = mat[2][2];
    b[0] = gAlphaT[0][2];
    b[1] = gAlphaT[1][2];
    b[2] = gAlphaT[2][2];
    dP = GeometryMath::CosThetaBetweenVectors(a, b);
    angle = std::acos(dP);
    if((angle * radToDeg) < m_AngleTolerance || (180.0f - (angle * radToDeg)) < m_AngleTolerance)
    {
      a[0] = mat[0][0];
      a[1] = mat[1][0];
      a[2] = mat[2][0];
      b[0] = gAlphaT[0][0];
      b[1] = gAlphaT[1][0];
      b[2] = gAlphaT[2][0];
      dP = GeometryMath::CosThetaBetweenVectors(a, b);
      angle = std::acos(dP);
      if((angle * radToDeg) < m_AngleTolerance)
      {
        return true;
      }
      if((180.0 - (angle * radToDeg)) < m_AngleTolerance)
      {
        return true;
      }
      b[0] = -0.5 * gAlphaT[0][0] + 0.866025 * gAlphaT[0][1];
      b[1] = -0.5 * gAlphaT[1][0] + 0.866025 * gAlphaT[1][1];
      b[2] = -0.5 * gAlphaT[2][0] + 0.866025 * gAlphaT[2][1];
      dP = GeometryMath::CosThetaBetweenVectors(a, b);
      angle = std::acos(dP);
      if((angle * radToDeg) < m_AngleTolerance)
      {
        return true;
      }
      if((180.0 - (angle * radToDeg)) < m_AngleTolerance)
      {
        return true;
      }
      b[0] = -0.5 * gAlphaT[0][0] - 0.866025 * gAlphaT[0][1];
      b[1] = -0.5 * gAlphaT[1][0] - 0.866025 * gAlphaT[1][1];
      b[2] = -0.5 * gAlphaT[2][0] - 0.866025 * gAlphaT[2][1];
      dP = GeometryMath::CosThetaBetweenVectors(a, b);
      angle = std::acos(dP);
      if((angle * radToDeg) < m_AngleTolerance)
      {
        return true;
      }
      if((180.0 - (angle * radToDeg)) < m_AngleTolerance)
      {
        return true;
      }
    }
  }
  return false;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void MergeColonies::identify_globAlpha()
{
  // DataContainer::Pointer m = getDataContainerArray()->getDataContainer(getDataContainerName());

  // int64_t totalPoints = m->getAttributeMatrix(getCellAttributeMatrixName())->getNumberOfTuples();
  // QVector<int> betaSize(numParents, 0);
  // QVector<int> totalSize(numParents, 0);
  // for (int64_t i = 0; i < totalPoints; i++)
  //{
  //  int pnum = m_CellParentIds[i];
  //  totalSize[pnum]++;
  //  if(EbsdLib::CrystalStructure::Cubic_High == m_CrystalStructures[m_CellPhases[i]] )
  //  { betaSize[pnum]++; }
  //}
  // for (int64_t i = 0; i < totalPoints; i++)
  //{
  //  int pnum = m_CellParentIds[i];
  //  float ratio = float(betaSize[pnum]) / float(totalSize[pnum]);
  //  if(ratio > 0.0) { m_GlobAlpha[i] = 0; }
  //  else { m_GlobAlpha[i] = 1; }
  //}
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void MergeColonies::execute()
{
  dataCheck();
  if(getErrorCode() < 0)
  {
    return;
  }

  m_AxisToleranceRad = m_AxisTolerance * SIMPLib::Constants::k_PiD / 180.0f;

  GroupFeatures::execute();

  size_t totalFeatures = m_ActivePtr.lock()->getNumberOfTuples();
  if(totalFeatures < 2)
  {
    setErrorCondition(-87000, "The number of Grouped Features was 0 or 1 which means no grouped Features were detected. A grouping value may be set too high");
    return;
  }

  int32_t numParents = 0;
  size_t totalPoints = m_FeatureIdsPtr.lock()->getNumberOfTuples();
  for(size_t k = 0; k < totalPoints; k++)
  {
    int32_t featurename = m_FeatureIds[k];
    m_CellParentIds[k] = m_FeatureParentIds[featurename];
    if(m_FeatureParentIds[featurename] > numParents)
    {
      numParents = m_FeatureParentIds[featurename];
    }
  }
  numParents += 1;

  notifyStatusMessage("Characterizing Colonies");
  characterize_colonies();

  if(m_RandomizeParentIds)
  {
    // Generate all the numbers up front
    const int32_t rangeMin = 1;
    const int32_t rangeMax = numParents - 1;
    std::mt19937_64::result_type seed = static_cast<std::mt19937_64::result_type>(std::chrono::steady_clock::now().time_since_epoch().count());
    std::mt19937_64 generator(seed); // Standard mersenne_twister_engine seeded with milliseconds
    std::uniform_int_distribution<int32_t> distribution(rangeMin, rangeMax);

    DataArray<int32_t>::Pointer rndNumbers = DataArray<int32_t>::CreateArray(numParents, std::string("_INTERNAL_USE_ONLY_NewParentIds"), true);
    int32_t* pid = rndNumbers->getPointer(0);
    pid[0] = 0;
    QSet<int32_t> parentIdSet;
    parentIdSet.insert(0);
    for(int32_t i = 1; i < numParents; ++i)
    {
      pid[i] = i; // numberGenerator();
      parentIdSet.insert(pid[i]);
    }

    int32_t r = 0;
    int32_t temp = 0;

    //--- Shuffle elements by randomly exchanging each with one other.
    for(int32_t i = 1; i < numParents; i++)
    {
      r = distribution(generator); // Random remaining position.
      if(r >= numParents)
      {
        continue;
      }
      temp = pid[i];
      pid[i] = pid[r];
      pid[r] = temp;
    }

    // Now adjust all the Feature Id values for each Voxel
    for(size_t i = 0; i < totalPoints; ++i)
    {
      m_CellParentIds[i] = pid[m_CellParentIds[i]];
      m_FeatureParentIds[m_FeatureIds[i]] = m_CellParentIds[i];
    }
  }

  if(m_IdentifyGlobAlpha)
  {
    identify_globAlpha();
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
AbstractFilter::Pointer MergeColonies::newFilterInstance(bool copyFilterParameters) const
{
  MergeColonies::Pointer filter = MergeColonies::New();
  if(copyFilterParameters)
  {
    copyFilterParameterInstanceVariables(filter.get());
  }
  return filter;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString MergeColonies::getCompiledLibraryName() const
{
  return ReconstructionConstants::ReconstructionBaseName;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString MergeColonies::getBrandingString() const
{
  return "Reconstruction";
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString MergeColonies::getFilterVersion() const
{
  QString version;
  QTextStream vStream(&version);
  vStream << Reconstruction::Version::Major() << "." << Reconstruction::Version::Minor() << "." << Reconstruction::Version::Patch();
  return version;
}
// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString MergeColonies::getGroupName() const
{
  return SIMPL::FilterGroups::ReconstructionFilters;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QUuid MergeColonies::getUuid() const
{
  return QUuid("{2c4a6d83-6a1b-56d8-9f65-9453b28845b9}");
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString MergeColonies::getSubGroupName() const
{
  return SIMPL::FilterSubGroups::GroupingFilters;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString MergeColonies::getHumanLabel() const
{
  return "Merge Colonies";
}

// -----------------------------------------------------------------------------
MergeColonies::Pointer MergeColonies::NullPointer()
{
  return Pointer(static_cast<Self*>(nullptr));
}

// -----------------------------------------------------------------------------
std::shared_ptr<MergeColonies> MergeColonies::New()
{
  struct make_shared_enabler : public MergeColonies
  {
  };
  std::shared_ptr<make_shared_enabler> val = std::make_shared<make_shared_enabler>();
  val->setupFilterParameters();
  return val;
}

// -----------------------------------------------------------------------------
QString MergeColonies::getNameOfClass() const
{
  return QString("MergeColonies");
}

// -----------------------------------------------------------------------------
QString MergeColonies::ClassName()
{
  return QString("MergeColonies");
}

// -----------------------------------------------------------------------------
void MergeColonies::setNewCellFeatureAttributeMatrixName(const QString& value)
{
  m_NewCellFeatureAttributeMatrixName = value;
}

// -----------------------------------------------------------------------------
QString MergeColonies::getNewCellFeatureAttributeMatrixName() const
{
  return m_NewCellFeatureAttributeMatrixName;
}

// -----------------------------------------------------------------------------
void MergeColonies::setFeatureIdsArrayPath(const DataArrayPath& value)
{
  m_FeatureIdsArrayPath = value;
}

// -----------------------------------------------------------------------------
DataArrayPath MergeColonies::getFeatureIdsArrayPath() const
{
  return m_FeatureIdsArrayPath;
}

// -----------------------------------------------------------------------------
void MergeColonies::setCellPhasesArrayPath(const DataArrayPath& value)
{
  m_CellPhasesArrayPath = value;
}

// -----------------------------------------------------------------------------
DataArrayPath MergeColonies::getCellPhasesArrayPath() const
{
  return m_CellPhasesArrayPath;
}

// -----------------------------------------------------------------------------
void MergeColonies::setFeaturePhasesArrayPath(const DataArrayPath& value)
{
  m_FeaturePhasesArrayPath = value;
}

// -----------------------------------------------------------------------------
DataArrayPath MergeColonies::getFeaturePhasesArrayPath() const
{
  return m_FeaturePhasesArrayPath;
}

// -----------------------------------------------------------------------------
void MergeColonies::setAvgQuatsArrayPath(const DataArrayPath& value)
{
  m_AvgQuatsArrayPath = value;
}

// -----------------------------------------------------------------------------
DataArrayPath MergeColonies::getAvgQuatsArrayPath() const
{
  return m_AvgQuatsArrayPath;
}

// -----------------------------------------------------------------------------
void MergeColonies::setCrystalStructuresArrayPath(const DataArrayPath& value)
{
  m_CrystalStructuresArrayPath = value;
}

// -----------------------------------------------------------------------------
DataArrayPath MergeColonies::getCrystalStructuresArrayPath() const
{
  return m_CrystalStructuresArrayPath;
}

// -----------------------------------------------------------------------------
void MergeColonies::setCellParentIdsArrayName(const QString& value)
{
  m_CellParentIdsArrayName = value;
}

// -----------------------------------------------------------------------------
QString MergeColonies::getCellParentIdsArrayName() const
{
  return m_CellParentIdsArrayName;
}

// -----------------------------------------------------------------------------
void MergeColonies::setGlobAlphaArrayName(const QString& value)
{
  m_GlobAlphaArrayName = value;
}

// -----------------------------------------------------------------------------
QString MergeColonies::getGlobAlphaArrayName() const
{
  return m_GlobAlphaArrayName;
}

// -----------------------------------------------------------------------------
void MergeColonies::setFeatureParentIdsArrayName(const QString& value)
{
  m_FeatureParentIdsArrayName = value;
}

// -----------------------------------------------------------------------------
QString MergeColonies::getFeatureParentIdsArrayName() const
{
  return m_FeatureParentIdsArrayName;
}

// -----------------------------------------------------------------------------
void MergeColonies::setActiveArrayName(const QString& value)
{
  m_ActiveArrayName = value;
}

// -----------------------------------------------------------------------------
QString MergeColonies::getActiveArrayName() const
{
  return m_ActiveArrayName;
}

// -----------------------------------------------------------------------------
void MergeColonies::setAxisTolerance(float value)
{
  m_AxisTolerance = value;
}

// -----------------------------------------------------------------------------
float MergeColonies::getAxisTolerance() const
{
  return m_AxisTolerance;
}

// -----------------------------------------------------------------------------
void MergeColonies::setAngleTolerance(float value)
{
  m_AngleTolerance = value;
}

// -----------------------------------------------------------------------------
float MergeColonies::getAngleTolerance() const
{
  return m_AngleTolerance;
}

// -----------------------------------------------------------------------------
void MergeColonies::setRandomizeParentIds(bool value)
{
  m_RandomizeParentIds = value;
}

// -----------------------------------------------------------------------------
bool MergeColonies::getRandomizeParentIds() const
{
  return m_RandomizeParentIds;
}

// -----------------------------------------------------------------------------
void MergeColonies::setIdentifyGlobAlpha(bool value)
{
  m_IdentifyGlobAlpha = value;
}

// -----------------------------------------------------------------------------
bool MergeColonies::getIdentifyGlobAlpha() const
{
  return m_IdentifyGlobAlpha;
}
