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
#include "MergeTwins.h"

#include <chrono>
#include <random>

#include <QtCore/QTextStream>

#include "SIMPLib/Common/Constants.h"
#include "SIMPLib/DataContainers/DataContainer.h"
#include "SIMPLib/DataContainers/DataContainerArray.h"
#include "SIMPLib/FilterParameters/AbstractFilterParametersReader.h"
#include "SIMPLib/FilterParameters/DataArraySelectionFilterParameter.h"
#include "SIMPLib/FilterParameters/FloatFilterParameter.h"
#include "SIMPLib/FilterParameters/LinkedPathCreationFilterParameter.h"
#include "SIMPLib/FilterParameters/SeparatorFilterParameter.h"

#include "SIMPLib/Math/SIMPLibMath.h"
#include "SIMPLib/Math/SIMPLibRandom.h"

#include "EbsdLib/Core/EbsdLibConstants.h"
#include "EbsdLib/Core/Orientation.hpp"
#include "EbsdLib/Core/Quaternion.hpp"
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

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
MergeTwins::MergeTwins()
: m_NewCellFeatureAttributeMatrixName(SIMPL::Defaults::NewCellFeatureAttributeMatrixName)
, m_AxisTolerance(1.0f)
, m_AngleTolerance(1.0f)
, m_RandomizeParentIds(true)
, m_FeatureIdsArrayPath("", "", "")
, m_FeaturePhasesArrayPath("", "", "")
, m_AvgQuatsArrayPath("", "", "")
, m_CrystalStructuresArrayPath("", "", "")
, m_CellParentIdsArrayName(SIMPL::CellData::ParentIds)
, m_FeatureParentIdsArrayName(SIMPL::FeatureData::ParentIds)
, m_ActiveArrayName(SIMPL::FeatureData::Active)
{
  initialize();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
MergeTwins::~MergeTwins() = default;

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void MergeTwins::setupFilterParameters()
{
  GroupFeatures::setupFilterParameters();
  FilterParameterVectorType parameters = getFilterParameters();

  parameters.push_back(SIMPL_NEW_FLOAT_FP("Axis Tolerance (Degrees)", AxisTolerance, FilterParameter::Category::Parameter, MergeTwins));
  parameters.push_back(SIMPL_NEW_FLOAT_FP("Angle Tolerance (Degrees)", AngleTolerance, FilterParameter::Category::Parameter, MergeTwins));
  {
    DataArraySelectionFilterParameter::RequirementType req = DataArraySelectionFilterParameter::CreateCategoryRequirement(SIMPL::TypeNames::Int32, 1, AttributeMatrix::Category::Feature);
    parameters.push_back(SIMPL_NEW_DA_SELECTION_FP("Phases", FeaturePhasesArrayPath, FilterParameter::Category::RequiredArray, MergeTwins, req));
  }
  {
    DataArraySelectionFilterParameter::RequirementType req = DataArraySelectionFilterParameter::CreateCategoryRequirement(SIMPL::TypeNames::Float, 4, AttributeMatrix::Category::Feature);
    parameters.push_back(SIMPL_NEW_DA_SELECTION_FP("Average Quaternions", AvgQuatsArrayPath, FilterParameter::Category::RequiredArray, MergeTwins, req));
  }
  parameters.push_back(SeparatorFilterParameter::Create("Element Data", FilterParameter::Category::RequiredArray));
  {
    DataArraySelectionFilterParameter::RequirementType req = DataArraySelectionFilterParameter::CreateCategoryRequirement(SIMPL::TypeNames::Int32, 1, AttributeMatrix::Category::Element);
    parameters.push_back(SIMPL_NEW_DA_SELECTION_FP("Feature Ids", FeatureIdsArrayPath, FilterParameter::Category::RequiredArray, MergeTwins, req));
  }
  parameters.push_back(SeparatorFilterParameter::Create("Ensemble Data", FilterParameter::Category::RequiredArray));
  {
    DataArraySelectionFilterParameter::RequirementType req = DataArraySelectionFilterParameter::CreateCategoryRequirement(SIMPL::TypeNames::UInt32, 1, AttributeMatrix::Category::Ensemble);
    parameters.push_back(SIMPL_NEW_DA_SELECTION_FP("Crystal Structures", CrystalStructuresArrayPath, FilterParameter::Category::RequiredArray, MergeTwins, req));
  }
  parameters.push_back(SeparatorFilterParameter::Create("Element Data", FilterParameter::Category::CreatedArray));
  parameters.push_back(SIMPL_NEW_DA_WITH_LINKED_AM_FP("Parent Ids", CellParentIdsArrayName, FeatureIdsArrayPath, FeatureIdsArrayPath, FilterParameter::Category::CreatedArray, MergeTwins));
  parameters.push_back(SeparatorFilterParameter::Create("Feature Data", FilterParameter::Category::CreatedArray));
  parameters.push_back(SIMPL_NEW_AM_WITH_LINKED_DC_FP("Feature Attribute Matrix", NewCellFeatureAttributeMatrixName, FeatureIdsArrayPath, FilterParameter::Category::CreatedArray, MergeTwins));
  parameters.push_back(SIMPL_NEW_DA_WITH_LINKED_AM_FP("Parent Ids", FeatureParentIdsArrayName, FeaturePhasesArrayPath, FeaturePhasesArrayPath, FilterParameter::Category::CreatedArray, MergeTwins));
  parameters.push_back(SIMPL_NEW_DA_WITH_LINKED_AM_FP("Active", ActiveArrayName, FeatureIdsArrayPath, NewCellFeatureAttributeMatrixName, FilterParameter::Category::CreatedArray, MergeTwins));
  setFilterParameters(parameters);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void MergeTwins::readFilterParameters(AbstractFilterParametersReader* reader, int index)
{
  GroupFeatures::readFilterParameters(reader, index);
  reader->openFilterGroup(this, index);
  setNewCellFeatureAttributeMatrixName(reader->readString("NewCellFeatureAttributeMatrixName", getNewCellFeatureAttributeMatrixName()));
  setActiveArrayName(reader->readString("ActiveArrayName", getActiveArrayName()));
  setFeatureParentIdsArrayName(reader->readString("FeatureParentIdsArrayName", getFeatureParentIdsArrayName()));
  setCellParentIdsArrayName(reader->readString("CellParentIdsArrayName", getCellParentIdsArrayName()));
  setCrystalStructuresArrayPath(reader->readDataArrayPath("CrystalStructuresArrayPath", getCrystalStructuresArrayPath()));
  setAvgQuatsArrayPath(reader->readDataArrayPath("AvgQuatsArrayPath", getAvgQuatsArrayPath()));
  setFeaturePhasesArrayPath(reader->readDataArrayPath("FeaturePhasesArrayPath", getFeaturePhasesArrayPath()));
  setFeatureIdsArrayPath(reader->readDataArrayPath("FeatureIdsArrayPath", getFeatureIdsArrayPath()));
  setAxisTolerance(reader->readValue("AxisTolerance", getAxisTolerance()));
  setAngleTolerance(reader->readValue("AngleTolerance", getAngleTolerance()));
  reader->closeFilterGroup();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void MergeTwins::updateFeatureInstancePointers()
{
  clearErrorCode();
  clearWarningCode();

  if(nullptr != m_ActivePtr.lock())
  {
    m_Active = m_ActivePtr.lock()->getPointer(0);
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void MergeTwins::initialize()
{
  m_AxisToleranceRad = 0.0f;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void MergeTwins::dataCheck()
{
  clearErrorCode();
  clearWarningCode();
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

  QVector<DataArrayPath> dataArrayPaths;

  // Cell Data
  m_FeatureIdsPtr = getDataContainerArray()->getPrereqArrayFromPath<DataArray<int32_t>>(this, getFeatureIdsArrayPath(), cDims);
  if(nullptr != m_FeatureIdsPtr.lock())
  {
    m_FeatureIds = m_FeatureIdsPtr.lock()->getPointer(0);
  }

  tempPath.update(m_FeatureIdsArrayPath.getDataContainerName(), m_FeatureIdsArrayPath.getAttributeMatrixName(), getCellParentIdsArrayName());
  m_CellParentIdsPtr = getDataContainerArray()->createNonPrereqArrayFromPath<DataArray<int32_t>>(this, tempPath, -1, cDims);
  if(nullptr != m_CellParentIdsPtr.lock())
  {
    m_CellParentIds = m_CellParentIdsPtr.lock()->getPointer(0);
  }

  // Feature Data
  m_FeaturePhasesPtr = getDataContainerArray()->getPrereqArrayFromPath<DataArray<int32_t>>(this, getFeaturePhasesArrayPath(), cDims);
  if(nullptr != m_FeaturePhasesPtr.lock())
  {
    m_FeaturePhases = m_FeaturePhasesPtr.lock()->getPointer(0);
  }
  if(getErrorCode() >= 0)
  {
    dataArrayPaths.push_back(getFeaturePhasesArrayPath());
  }

  tempPath.update(m_FeaturePhasesArrayPath.getDataContainerName(), m_FeaturePhasesArrayPath.getAttributeMatrixName(), getFeatureParentIdsArrayName());
  m_FeatureParentIdsPtr = getDataContainerArray()->createNonPrereqArrayFromPath<DataArray<int32_t>>(this, tempPath, -1, cDims);
  if(nullptr != m_FeatureParentIdsPtr.lock())
  {
    m_FeatureParentIds = m_FeatureParentIdsPtr.lock()->getPointer(0);
  }

  cDims[0] = 4;
  m_AvgQuatsPtr = getDataContainerArray()->getPrereqArrayFromPath<DataArray<float>>(this, getAvgQuatsArrayPath(), cDims);
  if(nullptr != m_AvgQuatsPtr.lock())
  {
    m_AvgQuats = m_AvgQuatsPtr.lock()->getPointer(0);
  }
  if(getErrorCode() >= 0)
  {
    dataArrayPaths.push_back(getAvgQuatsArrayPath());
  }

  // New Feature Data
  cDims[0] = 1;
  tempPath.update(m_FeatureIdsArrayPath.getDataContainerName(), getNewCellFeatureAttributeMatrixName(), getActiveArrayName());
  m_ActivePtr = getDataContainerArray()->createNonPrereqArrayFromPath<DataArray<bool>>(this, tempPath, true, cDims, "", DataArrayID31);
  if(nullptr != m_ActivePtr.lock())
  {
    m_Active = m_ActivePtr.lock()->getPointer(0);
  }

  // Ensemble Data
  m_CrystalStructuresPtr = getDataContainerArray()->getPrereqArrayFromPath<DataArray<unsigned int>>(this, getCrystalStructuresArrayPath(), cDims);
  if(nullptr != m_CrystalStructuresPtr.lock())
  {
    m_CrystalStructures = m_CrystalStructuresPtr.lock()->getPointer(0);
  }

  getDataContainerArray()->validateNumberOfTuples(this, dataArrayPaths);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int32_t MergeTwins::getSeed(int32_t newFid)
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
bool MergeTwins::determineGrouping(int32_t referenceFeature, int32_t neighborFeature, int32_t newFid)
{
  // float w = 0.0f;
  // float n1 = 0.0f, n2 = 0.0f, n3 = 0.0f;
  bool twin = false;
  std::vector<LaueOps::Pointer> m_OrientationOps = LaueOps::GetAllOrientationOps();
  float* currentAvgQuatPtr = nullptr;

  if(m_FeatureParentIds[neighborFeature] == -1 && m_FeaturePhases[referenceFeature] > 0 && m_FeaturePhases[neighborFeature] > 0)
  {
    uint32_t phase1 = m_CrystalStructures[m_FeaturePhases[referenceFeature]];

    currentAvgQuatPtr = m_AvgQuats + referenceFeature * 4;
    QuatF q1(currentAvgQuatPtr[0], currentAvgQuatPtr[1], currentAvgQuatPtr[2], currentAvgQuatPtr[3]);
    currentAvgQuatPtr = m_AvgQuats + neighborFeature * 4;
    QuatF q2(currentAvgQuatPtr[0], currentAvgQuatPtr[1], currentAvgQuatPtr[2], currentAvgQuatPtr[3]);

    uint32_t phase2 = m_CrystalStructures[m_FeaturePhases[neighborFeature]];
    if(phase1 == phase2 && (phase1 == EbsdLib::CrystalStructure::Cubic_High))
    {
      OrientationD axisAngle = m_OrientationOps[phase1]->calculateMisorientation(q1, q2);
      double w = axisAngle[3];
      w = w * (SIMPLib::Constants::k_180OverPiD);
      double axisdiff111 = acosf(fabs(axisAngle[0]) * 0.57735f + fabs(axisAngle[1]) * 0.57735f + fabs(axisAngle[2]) * 0.57735f);
      double angdiff60 = fabs(w - 60.0f);
      if(axisdiff111 < m_AxisToleranceRad && angdiff60 < m_AngleTolerance)
      {
        twin = true;
      }
      if(twin)
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
void MergeTwins::characterize_twins()
{
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void MergeTwins::execute()
{
  dataCheck();
  if(getErrorCode() < 0)
  {
    return;
  }

  /* Sanity check that each phase is Cubic High (m3m) Laue class. If not then warn the user.
   * There is code later on to ensure that only m3m Laue class is used.
   */
  UInt32ArrayType& laueClasses = *(m_CrystalStructuresPtr.lock().get());
  for(size_t i = 1; i < laueClasses.size(); i++)
  {
    if(i != EbsdLib::CrystalStructure::Cubic_High)
    {
      QString msg = QString("Phase %1 is NOT m3m crystal symmetry. Data from this phase will not be used in this filter.");
      setWarningCondition(-23501, msg);
    }
  }

  m_AxisToleranceRad = m_AxisTolerance * SIMPLib::Constants::k_PiOver180D;

  m_FeatureParentIds[0] = 0; // set feature 0 to be parent 0

  GroupFeatures::execute();

  size_t totalFeatures = m_ActivePtr.lock()->getNumberOfTuples();
  if(totalFeatures < 2)
  {
    setErrorCondition(-87000, "The number of grouped Features was 0 or 1 which means no grouped Features were detected. A grouping value may be set too high");
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

  notifyStatusMessage("Characterizing Twins Starting");
  characterize_twins();
  notifyStatusMessage("Characterizing Twins Complete");

  if(m_RandomizeParentIds)
  {
    notifyStatusMessage("Randomizing Parent Ids....");
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

    notifyStatusMessage("Shuffle elements ....");
    //--- Shuffle elements by randomly exchanging each with one other.
    for(size_t i = 1; i < static_cast<size_t>(numParents); i++)
    {
      size_t r = static_cast<size_t>(distribution(generator)); // Random remaining position.
      int32_t pid_i = rndNumbers->getValue(i);
      if(r >= numParents)
      {
        continue;
      }
      int32_t pid_r = rndNumbers->getValue(r);
      rndNumbers->setValue(i, pid_r);
      rndNumbers->setValue(r, pid_i);
    }

    notifyStatusMessage("Adjusting Feature Ids Array....");
    // Now adjust all the Feature Id values for each Voxel
    for(size_t i = 0; i < totalPoints; ++i)
    {
      m_CellParentIds[i] = pid[m_CellParentIds[i]];
      m_FeatureParentIds[m_FeatureIds[i]] = m_CellParentIds[i];
    }
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
AbstractFilter::Pointer MergeTwins::newFilterInstance(bool copyFilterParameters) const
{
  MergeTwins::Pointer filter = MergeTwins::New();
  if(copyFilterParameters)
  {
    copyFilterParameterInstanceVariables(filter.get());
  }
  return filter;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString MergeTwins::getCompiledLibraryName() const
{
  return ReconstructionConstants::ReconstructionBaseName;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString MergeTwins::getBrandingString() const
{
  return "Reconstruction";
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString MergeTwins::getFilterVersion() const
{
  QString version;
  QTextStream vStream(&version);
  vStream << Reconstruction::Version::Major() << "." << Reconstruction::Version::Minor() << "." << Reconstruction::Version::Patch();
  return version;
}
// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString MergeTwins::getGroupName() const
{
  return SIMPL::FilterGroups::ReconstructionFilters;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QUuid MergeTwins::getUuid() const
{
  return QUuid("{c9af506e-9ea1-5ff5-a882-fa561def5f52}");
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString MergeTwins::getSubGroupName() const
{
  return SIMPL::FilterSubGroups::GroupingFilters;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString MergeTwins::getHumanLabel() const
{
  return "Merge Twins";
}

// -----------------------------------------------------------------------------
MergeTwins::Pointer MergeTwins::NullPointer()
{
  return Pointer(static_cast<Self*>(nullptr));
}

// -----------------------------------------------------------------------------
std::shared_ptr<MergeTwins> MergeTwins::New()
{
  struct make_shared_enabler : public MergeTwins
  {
  };
  std::shared_ptr<make_shared_enabler> val = std::make_shared<make_shared_enabler>();
  val->setupFilterParameters();
  return val;
}

// -----------------------------------------------------------------------------
QString MergeTwins::getNameOfClass() const
{
  return QString("MergeTwins");
}

// -----------------------------------------------------------------------------
QString MergeTwins::ClassName()
{
  return QString("MergeTwins");
}

// -----------------------------------------------------------------------------
void MergeTwins::setNewCellFeatureAttributeMatrixName(const QString& value)
{
  m_NewCellFeatureAttributeMatrixName = value;
}

// -----------------------------------------------------------------------------
QString MergeTwins::getNewCellFeatureAttributeMatrixName() const
{
  return m_NewCellFeatureAttributeMatrixName;
}

// -----------------------------------------------------------------------------
void MergeTwins::setAxisTolerance(float value)
{
  m_AxisTolerance = value;
}

// -----------------------------------------------------------------------------
float MergeTwins::getAxisTolerance() const
{
  return m_AxisTolerance;
}

// -----------------------------------------------------------------------------
void MergeTwins::setAngleTolerance(float value)
{
  m_AngleTolerance = value;
}

// -----------------------------------------------------------------------------
float MergeTwins::getAngleTolerance() const
{
  return m_AngleTolerance;
}

// -----------------------------------------------------------------------------
void MergeTwins::setFeatureIdsArrayPath(const DataArrayPath& value)
{
  m_FeatureIdsArrayPath = value;
}

// -----------------------------------------------------------------------------
DataArrayPath MergeTwins::getFeatureIdsArrayPath() const
{
  return m_FeatureIdsArrayPath;
}

// -----------------------------------------------------------------------------
void MergeTwins::setFeaturePhasesArrayPath(const DataArrayPath& value)
{
  m_FeaturePhasesArrayPath = value;
}

// -----------------------------------------------------------------------------
DataArrayPath MergeTwins::getFeaturePhasesArrayPath() const
{
  return m_FeaturePhasesArrayPath;
}

// -----------------------------------------------------------------------------
void MergeTwins::setAvgQuatsArrayPath(const DataArrayPath& value)
{
  m_AvgQuatsArrayPath = value;
}

// -----------------------------------------------------------------------------
DataArrayPath MergeTwins::getAvgQuatsArrayPath() const
{
  return m_AvgQuatsArrayPath;
}

// -----------------------------------------------------------------------------
void MergeTwins::setCrystalStructuresArrayPath(const DataArrayPath& value)
{
  m_CrystalStructuresArrayPath = value;
}

// -----------------------------------------------------------------------------
DataArrayPath MergeTwins::getCrystalStructuresArrayPath() const
{
  return m_CrystalStructuresArrayPath;
}

// -----------------------------------------------------------------------------
void MergeTwins::setCellParentIdsArrayName(const QString& value)
{
  m_CellParentIdsArrayName = value;
}

// -----------------------------------------------------------------------------
QString MergeTwins::getCellParentIdsArrayName() const
{
  return m_CellParentIdsArrayName;
}

// -----------------------------------------------------------------------------
void MergeTwins::setFeatureParentIdsArrayName(const QString& value)
{
  m_FeatureParentIdsArrayName = value;
}

// -----------------------------------------------------------------------------
QString MergeTwins::getFeatureParentIdsArrayName() const
{
  return m_FeatureParentIdsArrayName;
}

// -----------------------------------------------------------------------------
void MergeTwins::setActiveArrayName(const QString& value)
{
  m_ActiveArrayName = value;
}

// -----------------------------------------------------------------------------
QString MergeTwins::getActiveArrayName() const
{
  return m_ActiveArrayName;
}
