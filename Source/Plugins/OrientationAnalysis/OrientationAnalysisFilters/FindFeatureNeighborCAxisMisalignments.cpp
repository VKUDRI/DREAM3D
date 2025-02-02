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

#include "FindFeatureNeighborCAxisMisalignments.h"

#include <QtCore/QTextStream>

#include "SIMPLib/Common/Constants.h"
#include "SIMPLib/DataContainers/DataContainerArray.h"
#include "SIMPLib/FilterParameters/AbstractFilterParametersReader.h"
#include "SIMPLib/FilterParameters/DataArraySelectionFilterParameter.h"
#include "SIMPLib/FilterParameters/LinkedBooleanFilterParameter.h"
#include "SIMPLib/FilterParameters/LinkedPathCreationFilterParameter.h"
#include "SIMPLib/FilterParameters/SeparatorFilterParameter.h"
#include "SIMPLib/FilterParameters/StringFilterParameter.h"
#include "SIMPLib/Math/GeometryMath.h"
#include "SIMPLib/Math/MatrixMath.h"
#include "SIMPLib/Math/SIMPLibMath.h"

#include "EbsdLib/Core/Orientation.hpp"
#include "EbsdLib/Core/OrientationTransformation.hpp"
#include "EbsdLib/Core/Quaternion.hpp"

#include "OrientationAnalysis/OrientationAnalysisConstants.h"
#include "OrientationAnalysis/OrientationAnalysisVersion.h"

#include "EbsdLib/Core/EbsdLibConstants.h"

using QuatF = Quaternion<float>;

/* Create Enumerations to allow the created Attribute Arrays to take part in renaming */
enum createdPathID : RenameDataPath::DataID_t
{
  DataArrayID30 = 30,
  DataArrayID31 = 31,
};

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
FindFeatureNeighborCAxisMisalignments::FindFeatureNeighborCAxisMisalignments()
{
  m_NeighborList = NeighborList<int32_t>::NullPointer();
  m_CAxisMisalignmentList = NeighborList<float>::NullPointer();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
FindFeatureNeighborCAxisMisalignments::~FindFeatureNeighborCAxisMisalignments() = default;

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void FindFeatureNeighborCAxisMisalignments::setupFilterParameters()
{
  FilterParameterVectorType parameters;
  std::vector<QString> linkedProps = {"AvgCAxisMisalignmentsArrayName"};
  parameters.push_back(SIMPL_NEW_LINKED_BOOL_FP("Find Average Misalignment Per Feature", FindAvgMisals, FilterParameter::Category::Parameter, FindFeatureNeighborCAxisMisalignments, linkedProps));
  parameters.push_back(SeparatorFilterParameter::Create("Feature Data", FilterParameter::Category::RequiredArray));
  {
    DataArraySelectionFilterParameter::RequirementType req = DataArraySelectionFilterParameter::CreateCategoryRequirement(SIMPL::TypeNames::NeighborList, 1, AttributeMatrix::Category::Feature);
    parameters.push_back(SIMPL_NEW_DA_SELECTION_FP("Neighbor List", NeighborListArrayPath, FilterParameter::Category::RequiredArray, FindFeatureNeighborCAxisMisalignments, req));
  }
  {
    DataArraySelectionFilterParameter::RequirementType req = DataArraySelectionFilterParameter::CreateCategoryRequirement(SIMPL::TypeNames::Float, 4, AttributeMatrix::Category::Feature);
    parameters.push_back(SIMPL_NEW_DA_SELECTION_FP("Average Quaternions", AvgQuatsArrayPath, FilterParameter::Category::RequiredArray, FindFeatureNeighborCAxisMisalignments, req));
  }
  {
    DataArraySelectionFilterParameter::RequirementType req = DataArraySelectionFilterParameter::CreateCategoryRequirement(SIMPL::TypeNames::Int32, 1, AttributeMatrix::Category::Feature);
    parameters.push_back(SIMPL_NEW_DA_SELECTION_FP("Phases", FeaturePhasesArrayPath, FilterParameter::Category::RequiredArray, FindFeatureNeighborCAxisMisalignments, req));
  }
  parameters.push_back(SeparatorFilterParameter::Create("Ensemble Data", FilterParameter::Category::RequiredArray));
  {
    DataArraySelectionFilterParameter::RequirementType req = DataArraySelectionFilterParameter::CreateCategoryRequirement(SIMPL::TypeNames::UInt32, 1, AttributeMatrix::Category::Ensemble);
    parameters.push_back(SIMPL_NEW_DA_SELECTION_FP("Crystal Structures", CrystalStructuresArrayPath, FilterParameter::Category::RequiredArray, FindFeatureNeighborCAxisMisalignments, req));
  }
  parameters.push_back(SeparatorFilterParameter::Create("Feature Data", FilterParameter::Category::CreatedArray));
  parameters.push_back(SIMPL_NEW_DA_WITH_LINKED_AM_FP("C-Axis Misalignment List", CAxisMisalignmentListArrayName, NeighborListArrayPath, NeighborListArrayPath, FilterParameter::Category::CreatedArray,
                                                      FindFeatureNeighborCAxisMisalignments));
  parameters.push_back(SIMPL_NEW_DA_WITH_LINKED_AM_FP("Avgerage C-Axis Misalignments", AvgCAxisMisalignmentsArrayName, FeaturePhasesArrayPath, FeaturePhasesArrayPath,
                                                      FilterParameter::Category::CreatedArray, FindFeatureNeighborCAxisMisalignments));
  setFilterParameters(parameters);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void FindFeatureNeighborCAxisMisalignments::readFilterParameters(AbstractFilterParametersReader* reader, int index)
{
  reader->openFilterGroup(this, index);
  setNeighborListArrayPath(reader->readDataArrayPath("NeighborListArrayPath", getNeighborListArrayPath()));
  setCAxisMisalignmentListArrayName(reader->readString("CAxisMisalignmentListArrayName", getCAxisMisalignmentListArrayName()));
  setAvgCAxisMisalignmentsArrayName(reader->readString("AvgCAxisMisalignmentsArrayName", getAvgCAxisMisalignmentsArrayName()));
  setCrystalStructuresArrayPath(reader->readDataArrayPath("CrystalStructuresArrayPath", getCrystalStructuresArrayPath()));
  setFeaturePhasesArrayPath(reader->readDataArrayPath("FeaturePhasesArrayPath", getFeaturePhasesArrayPath()));
  setAvgQuatsArrayPath(reader->readDataArrayPath("AvgQuatsArrayPath", getAvgQuatsArrayPath()));
  setFindAvgMisals(reader->readValue("FindAvgMisals", getFindAvgMisals()));
  reader->closeFilterGroup();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void FindFeatureNeighborCAxisMisalignments::initialize()
{
  m_NeighborList = NeighborList<int32_t>::NullPointer();
  m_CAxisMisalignmentList = NeighborList<float>::NullPointer();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void FindFeatureNeighborCAxisMisalignments::dataCheck()
{
  clearErrorCode();
  clearWarningCode();
  initialize();

  DataArrayPath tempPath;

  QVector<DataArrayPath> dataArrayPaths;

  std::vector<size_t> cDims(1, 4);
  // Feature Data
  m_AvgQuatsPtr = getDataContainerArray()->getPrereqArrayFromPath<DataArray<float>>(this, getAvgQuatsArrayPath(), cDims);
  if(nullptr != m_AvgQuatsPtr.lock())
  {
    m_AvgQuats = m_AvgQuatsPtr.lock()->getPointer(0);
  } /* Now assign the raw pointer to data from the DataArray<T> object */
  if(getErrorCode() >= 0)
  {
    dataArrayPaths.push_back(getAvgQuatsArrayPath());
  }

  cDims[0] = 1;
  m_FeaturePhasesPtr = getDataContainerArray()->getPrereqArrayFromPath<DataArray<int32_t>>(this, getFeaturePhasesArrayPath(), cDims);
  if(nullptr != m_FeaturePhasesPtr.lock())
  {
    m_FeaturePhases = m_FeaturePhasesPtr.lock()->getPointer(0);
  } /* Now assign the raw pointer to data from the DataArray<T> object */
  if(getErrorCode() >= 0)
  {
    dataArrayPaths.push_back(getFeaturePhasesArrayPath());
  }

  // New Feature Data
  if(m_FindAvgMisals)
  {
    tempPath.update(m_FeaturePhasesArrayPath.getDataContainerName(), getFeaturePhasesArrayPath().getAttributeMatrixName(), getAvgCAxisMisalignmentsArrayName());
    m_AvgCAxisMisalignmentsPtr = getDataContainerArray()->createNonPrereqArrayFromPath<DataArray<float>>(this, tempPath, 0, cDims);
    if(nullptr != m_AvgCAxisMisalignmentsPtr.lock())
    {
      m_AvgCAxisMisalignments = m_AvgCAxisMisalignmentsPtr.lock()->getPointer(0);
    } /* Now assign the raw pointer to data from the DataArray<T> object */
  }

  // Ensemble Data
  m_CrystalStructuresPtr = getDataContainerArray()->getPrereqArrayFromPath<DataArray<uint32_t>>(this, getCrystalStructuresArrayPath(), cDims);
  if(nullptr != m_CrystalStructuresPtr.lock())
  {
    m_CrystalStructures = m_CrystalStructuresPtr.lock()->getPointer(0);
  } /* Now assign the raw pointer to data from the DataArray<T> object */

  // Now we are going to get a "Pointer" to the NeighborList object out of the DataContainer
  m_NeighborList = getDataContainerArray()->getPrereqArrayFromPath<NeighborList<int32_t>>(this, getNeighborListArrayPath(), cDims);
  if(getErrorCode() >= 0)
  {
    dataArrayPaths.push_back(getNeighborListArrayPath());
  }

  tempPath.update(m_NeighborListArrayPath.getDataContainerName(), getNeighborListArrayPath().getAttributeMatrixName(), getCAxisMisalignmentListArrayName());
  m_CAxisMisalignmentList = getDataContainerArray()->createNonPrereqArrayFromPath<NeighborList<float>>(this, tempPath, 0, cDims, "", DataArrayID31);

  getDataContainerArray()->validateNumberOfTuples(this, dataArrayPaths);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void FindFeatureNeighborCAxisMisalignments::execute()
{
  using OrientF = Orientation<float>;
  clearErrorCode();
  clearWarningCode();
  dataCheck();
  if(getErrorCode() < 0)
  {
    return;
  }

  size_t totalFeatures = m_FeaturePhasesPtr.lock()->getNumberOfTuples();

  // But since a pointer is difficult to use operators with we will now create a
  // reference variable to the pointer with the correct variable name that allows
  // us to use the same syntax as the "vector of vectors"
  NeighborList<int32_t>& neighborlist = *(m_NeighborList.lock());

  std::vector<std::vector<float>> misalignmentlists;

  float w = 0.0f;
  float g1[3][3] = {{0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}};
  float g2[3][3] = {{0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}};
  float g1t[3][3] = {{0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}};
  float g2t[3][3] = {{0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}};
  float c1[3] = {0.0f, 0.0f, 0.0f};
  float c2[3] = {0.0f, 0.0f, 0.0f};
  float caxis[3] = {0.0f, 0.0f, 1.0f};
  size_t hexneighborlistsize = 0;
  //  QuatF q1 = QuaternionMathF::New();
  //  QuatF q2 = QuaternionMathF::New();
  //  QuatF* avgQuats = reinterpret_cast<QuatF*>(m_AvgQuats);
  FloatArrayType::Pointer avgQuatsPtr = m_AvgQuatsPtr.lock();

  uint32_t phase1 = 0, phase2 = 0;
  size_t nname = 0;
  misalignmentlists.resize(totalFeatures);
  float* currentAvgQuatPtr = nullptr;
  for(size_t i = 1; i < totalFeatures; i++)
  {
    phase1 = m_CrystalStructures[m_FeaturePhases[i]];
    currentAvgQuatPtr = avgQuatsPtr->getTuplePointer(i);
    OrientationTransformation::qu2om<QuatF, OrientF>({currentAvgQuatPtr[0], currentAvgQuatPtr[1], currentAvgQuatPtr[2], currentAvgQuatPtr[3]}).toGMatrix(g1);

    // transpose the g matrix so when caxis is multiplied by it
    // it will give the sample direction that the caxis is along
    MatrixMath::Transpose3x3(g1, g1t);
    MatrixMath::Multiply3x3with3x1(g1t, caxis, c1);
    // normalize so that the dot product can be taken below without
    // dividing by the magnitudes (they would be 1)
    MatrixMath::Normalize3x1(c1);
    misalignmentlists[i].resize(neighborlist[i].size(), -1.0f);
    for(size_t j = 0; j < neighborlist[i].size(); j++)
    {
      w = std::numeric_limits<float>::max();
      nname = neighborlist[i][j];
      phase2 = m_CrystalStructures[m_FeaturePhases[nname]];
      hexneighborlistsize = neighborlist[i].size();
      if(phase1 == phase2 && (phase1 == EbsdLib::CrystalStructure::Hexagonal_High))
      {
        currentAvgQuatPtr = avgQuatsPtr->getTuplePointer(nname);
        OrientationTransformation::qu2om<QuatF, OrientF>({currentAvgQuatPtr[0], currentAvgQuatPtr[1], currentAvgQuatPtr[2], currentAvgQuatPtr[3]}).toGMatrix(g2);

        // transpose the g matrix so when caxis is multiplied by it
        // it will give the sample direction that the caxis is along
        MatrixMath::Transpose3x3(g2, g2t);
        MatrixMath::Multiply3x3with3x1(g2t, caxis, c2);
        // normalize so that the dot product can be taken below without
        // dividing by the magnitudes (they would be 1)
        MatrixMath::Normalize3x1(c2);

        w = GeometryMath::CosThetaBetweenVectors(c1, c2);
        SIMPLibMath::bound(w, -1.0f, 1.0f);
        w = acosf(w);
        if(w > (SIMPLib::Constants::k_PiD / 2))
        {
          w = SIMPLib::Constants::k_PiD - w;
        }

        misalignmentlists[i][j] = w * SIMPLib::Constants::k_180OverPiD;
        if(m_FindAvgMisals)
        {
          m_AvgCAxisMisalignments[i] += misalignmentlists[i][j];
        }
      }
      else
      {
        if(m_FindAvgMisals)
        {
          hexneighborlistsize--;
        }
        misalignmentlists[i][j] = NAN;
      }
    }
    if(m_FindAvgMisals)
    {
      if(hexneighborlistsize > 0)
      {
        m_AvgCAxisMisalignments[i] /= hexneighborlistsize;
      }
      else
      {
        m_AvgCAxisMisalignments[i] = NAN;
      }
      hexneighborlistsize = 0;
    }
  }

  for(size_t i = 1; i < totalFeatures; i++)
  {
    // Set the vector for each list into the NeighborList Object
    NeighborList<float>::SharedVectorType misaL(new std::vector<float>);
    misaL->assign(misalignmentlists[i].begin(), misalignmentlists[i].end());
    m_CAxisMisalignmentList.lock()->setList(static_cast<int32_t>(i), misaL);
  }
}
// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
AbstractFilter::Pointer FindFeatureNeighborCAxisMisalignments::newFilterInstance(bool copyFilterParameters) const
{
  FindFeatureNeighborCAxisMisalignments::Pointer filter = FindFeatureNeighborCAxisMisalignments::New();
  if(copyFilterParameters)
  {
    copyFilterParameterInstanceVariables(filter.get());
  }
  return filter;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString FindFeatureNeighborCAxisMisalignments::getCompiledLibraryName() const
{
  return OrientationAnalysisConstants::OrientationAnalysisBaseName;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString FindFeatureNeighborCAxisMisalignments::getBrandingString() const
{
  return "OrientationAnalysis";
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString FindFeatureNeighborCAxisMisalignments::getFilterVersion() const
{
  QString version;
  QTextStream vStream(&version);
  vStream << OrientationAnalysis::Version::Major() << "." << OrientationAnalysis::Version::Minor() << "." << OrientationAnalysis::Version::Patch();
  return version;
}
// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString FindFeatureNeighborCAxisMisalignments::getGroupName() const
{
  return SIMPL::FilterGroups::StatisticsFilters;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QUuid FindFeatureNeighborCAxisMisalignments::getUuid() const
{
  return QUuid("{cdd50b83-ea09-5499-b008-4b253cf4c246}");
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString FindFeatureNeighborCAxisMisalignments::getSubGroupName() const
{
  return SIMPL::FilterSubGroups::CrystallographyFilters;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString FindFeatureNeighborCAxisMisalignments::getHumanLabel() const
{
  return "Find Feature Neighbor C-Axis Misalignments";
}

// -----------------------------------------------------------------------------
FindFeatureNeighborCAxisMisalignments::Pointer FindFeatureNeighborCAxisMisalignments::NullPointer()
{
  return Pointer(static_cast<Self*>(nullptr));
}

// -----------------------------------------------------------------------------
std::shared_ptr<FindFeatureNeighborCAxisMisalignments> FindFeatureNeighborCAxisMisalignments::New()
{
  struct make_shared_enabler : public FindFeatureNeighborCAxisMisalignments
  {
  };
  std::shared_ptr<make_shared_enabler> val = std::make_shared<make_shared_enabler>();
  val->setupFilterParameters();
  return val;
}

// -----------------------------------------------------------------------------
QString FindFeatureNeighborCAxisMisalignments::getNameOfClass() const
{
  return QString("FindFeatureNeighborCAxisMisalignments");
}

// -----------------------------------------------------------------------------
QString FindFeatureNeighborCAxisMisalignments::ClassName()
{
  return QString("FindFeatureNeighborCAxisMisalignments");
}

// -----------------------------------------------------------------------------
void FindFeatureNeighborCAxisMisalignments::setFindAvgMisals(bool value)
{
  m_FindAvgMisals = value;
}

// -----------------------------------------------------------------------------
bool FindFeatureNeighborCAxisMisalignments::getFindAvgMisals() const
{
  return m_FindAvgMisals;
}

// -----------------------------------------------------------------------------
void FindFeatureNeighborCAxisMisalignments::setNeighborListArrayPath(const DataArrayPath& value)
{
  m_NeighborListArrayPath = value;
}

// -----------------------------------------------------------------------------
DataArrayPath FindFeatureNeighborCAxisMisalignments::getNeighborListArrayPath() const
{
  return m_NeighborListArrayPath;
}

// -----------------------------------------------------------------------------
void FindFeatureNeighborCAxisMisalignments::setCAxisMisalignmentListArrayName(const QString& value)
{
  m_CAxisMisalignmentListArrayName = value;
}

// -----------------------------------------------------------------------------
QString FindFeatureNeighborCAxisMisalignments::getCAxisMisalignmentListArrayName() const
{
  return m_CAxisMisalignmentListArrayName;
}

// -----------------------------------------------------------------------------
void FindFeatureNeighborCAxisMisalignments::setAvgQuatsArrayPath(const DataArrayPath& value)
{
  m_AvgQuatsArrayPath = value;
}

// -----------------------------------------------------------------------------
DataArrayPath FindFeatureNeighborCAxisMisalignments::getAvgQuatsArrayPath() const
{
  return m_AvgQuatsArrayPath;
}

// -----------------------------------------------------------------------------
void FindFeatureNeighborCAxisMisalignments::setFeaturePhasesArrayPath(const DataArrayPath& value)
{
  m_FeaturePhasesArrayPath = value;
}

// -----------------------------------------------------------------------------
DataArrayPath FindFeatureNeighborCAxisMisalignments::getFeaturePhasesArrayPath() const
{
  return m_FeaturePhasesArrayPath;
}

// -----------------------------------------------------------------------------
void FindFeatureNeighborCAxisMisalignments::setCrystalStructuresArrayPath(const DataArrayPath& value)
{
  m_CrystalStructuresArrayPath = value;
}

// -----------------------------------------------------------------------------
DataArrayPath FindFeatureNeighborCAxisMisalignments::getCrystalStructuresArrayPath() const
{
  return m_CrystalStructuresArrayPath;
}

// -----------------------------------------------------------------------------
void FindFeatureNeighborCAxisMisalignments::setAvgCAxisMisalignmentsArrayName(const QString& value)
{
  m_AvgCAxisMisalignmentsArrayName = value;
}

// -----------------------------------------------------------------------------
QString FindFeatureNeighborCAxisMisalignments::getAvgCAxisMisalignmentsArrayName() const
{
  return m_AvgCAxisMisalignmentsArrayName;
}
