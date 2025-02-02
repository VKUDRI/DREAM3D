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
#include "AlignSectionsMutualInformation.h"

#include <fstream>

#include <QtCore/QTextStream>

#include "SIMPLib/Common/Constants.h"
#include "SIMPLib/DataContainers/DataContainer.h"
#include "SIMPLib/DataContainers/DataContainerArray.h"
#include "SIMPLib/FilterParameters/AbstractFilterParametersReader.h"
#include "SIMPLib/FilterParameters/DataArraySelectionFilterParameter.h"
#include "SIMPLib/FilterParameters/FloatFilterParameter.h"
#include "SIMPLib/FilterParameters/LinkedBooleanFilterParameter.h"
#include "SIMPLib/FilterParameters/SeparatorFilterParameter.h"
#include "SIMPLib/Geometry/ImageGeom.h"
#include "SIMPLib/Math/SIMPLibMath.h"
#include "SIMPLib/Math/SIMPLibRandom.h"

#include "EbsdLib/LaueOps/LaueOps.h"

#include "Reconstruction/ReconstructionConstants.h"
#include "Reconstruction/ReconstructionVersion.h"

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
AlignSectionsMutualInformation::AlignSectionsMutualInformation()
: m_MisorientationTolerance(5.0f)
, m_UseGoodVoxels(true)
, m_QuatsArrayPath(SIMPL::Defaults::ImageDataContainerName, SIMPL::Defaults::CellAttributeMatrixName, SIMPL::CellData::Quats)
, m_CellPhasesArrayPath(SIMPL::Defaults::ImageDataContainerName, SIMPL::Defaults::CellAttributeMatrixName, SIMPL::CellData::Phases)
, m_GoodVoxelsArrayPath(SIMPL::Defaults::ImageDataContainerName, SIMPL::Defaults::CellAttributeMatrixName, SIMPL::CellData::Mask)
, m_CrystalStructuresArrayPath(SIMPL::Defaults::ImageDataContainerName, SIMPL::Defaults::CellEnsembleAttributeMatrixName, SIMPL::EnsembleData::CrystalStructures)
{
  m_RandomSeed = QDateTime::currentMSecsSinceEpoch();

  m_OrientationOps = LaueOps::GetAllOrientationOps();

  featurecounts = nullptr;

  // only setting up the child parameters because the parent constructor has already been called
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
AlignSectionsMutualInformation::~AlignSectionsMutualInformation() = default;

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void AlignSectionsMutualInformation::setupFilterParameters()
{
  // getting the current parameters that were set by the parent and adding to it before resetting it
  AlignSections::setupFilterParameters();
  FilterParameterVectorType parameters = getFilterParameters();
  parameters.push_back(SIMPL_NEW_FLOAT_FP("Misorientation Tolerance", MisorientationTolerance, FilterParameter::Category::Parameter, AlignSectionsMutualInformation));
  std::vector<QString> linkedProps = {"GoodVoxelsArrayPath"};
  parameters.push_back(SIMPL_NEW_LINKED_BOOL_FP("Use Mask Array", UseGoodVoxels, FilterParameter::Category::Parameter, AlignSectionsMutualInformation, linkedProps));
  parameters.push_back(SeparatorFilterParameter::Create("Cell Data", FilterParameter::Category::RequiredArray));
  {
    DataArraySelectionFilterParameter::RequirementType req = DataArraySelectionFilterParameter::CreateRequirement(SIMPL::TypeNames::Float, 4, AttributeMatrix::Type::Cell, IGeometry::Type::Image);
    parameters.push_back(SIMPL_NEW_DA_SELECTION_FP("Quaternions", QuatsArrayPath, FilterParameter::Category::RequiredArray, AlignSectionsMutualInformation, req));
  }
  {
    DataArraySelectionFilterParameter::RequirementType req = DataArraySelectionFilterParameter::CreateRequirement(SIMPL::TypeNames::Int32, 1, AttributeMatrix::Type::Cell, IGeometry::Type::Image);
    parameters.push_back(SIMPL_NEW_DA_SELECTION_FP("Phases", CellPhasesArrayPath, FilterParameter::Category::RequiredArray, AlignSectionsMutualInformation, req));
  }
  {
    DataArraySelectionFilterParameter::RequirementType req = DataArraySelectionFilterParameter::CreateRequirement(SIMPL::TypeNames::Bool, 1, AttributeMatrix::Type::Cell, IGeometry::Type::Image);
    parameters.push_back(SIMPL_NEW_DA_SELECTION_FP("Mask", GoodVoxelsArrayPath, FilterParameter::Category::RequiredArray, AlignSectionsMutualInformation, req));
  }
  parameters.push_back(SeparatorFilterParameter::Create("Cell Ensemble Data", FilterParameter::Category::RequiredArray));
  {
    DataArraySelectionFilterParameter::RequirementType req =
        DataArraySelectionFilterParameter::CreateRequirement(SIMPL::TypeNames::UInt32, 1, AttributeMatrix::Type::CellEnsemble, IGeometry::Type::Image);

    parameters.push_back(SIMPL_NEW_DA_SELECTION_FP("Crystal Structures", CrystalStructuresArrayPath, FilterParameter::Category::RequiredArray, AlignSectionsMutualInformation, req));
  }
  setFilterParameters(parameters);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void AlignSectionsMutualInformation::readFilterParameters(AbstractFilterParametersReader* reader, int index)
{
  AlignSections::readFilterParameters(reader, index);
  reader->openFilterGroup(this, index);
  setCrystalStructuresArrayPath(reader->readDataArrayPath("CrystalStructuresArrayPath", getCrystalStructuresArrayPath()));
  setUseGoodVoxels(reader->readValue("UseGoodVoxels", getUseGoodVoxels()));
  setGoodVoxelsArrayPath(reader->readDataArrayPath("GoodVoxelsArrayPath", getGoodVoxelsArrayPath()));
  setCellPhasesArrayPath(reader->readDataArrayPath("CellPhasesArrayPath", getCellPhasesArrayPath()));
  setQuatsArrayPath(reader->readDataArrayPath("QuatsArrayPath", getQuatsArrayPath()));
  setMisorientationTolerance(reader->readValue("MisorientationTolerance", getMisorientationTolerance()));
  reader->closeFilterGroup();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void AlignSectionsMutualInformation::initialize()
{
  m_RandomSeed = QDateTime::currentMSecsSinceEpoch();
  m_MIFeaturesPtr = Int32ArrayType::NullPointer();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void AlignSectionsMutualInformation::dataCheck()
{
  clearErrorCode();
  clearWarningCode();

  // Set the DataContainerName and AttributematrixName for the Parent Class (AlignSections) to Use.
  setDataContainerName(DataArrayPath(m_QuatsArrayPath.getDataContainerName(), "", ""));
  setCellAttributeMatrixName(m_QuatsArrayPath.getAttributeMatrixName());

  AlignSections::dataCheck();
  if(getErrorCode() < 0)
  {
    return;
  }

  m_FeatureCounts = DataArray<int32_t>::CreateArray(0, std::string("m_FeatureCounts"), true);

  QVector<DataArrayPath> dataArrayPaths;

  std::vector<size_t> cDims(1, 4);
  m_QuatsPtr = getDataContainerArray()->getPrereqArrayFromPath<DataArray<float>>(this, getQuatsArrayPath(), cDims);
  if(nullptr != m_QuatsPtr.lock())
  {
    m_Quats = m_QuatsPtr.lock()->getPointer(0);
  } /* Now assign the raw pointer to data from the DataArray<T> object */
  if(getErrorCode() >= 0)
  {
    dataArrayPaths.push_back(getQuatsArrayPath());
  }

  cDims[0] = 1;
  m_CellPhasesPtr = getDataContainerArray()->getPrereqArrayFromPath<DataArray<int32_t>>(this, getCellPhasesArrayPath(), cDims);
  if(nullptr != m_CellPhasesPtr.lock())
  {
    m_CellPhases = m_CellPhasesPtr.lock()->getPointer(0);
  } /* Now assign the raw pointer to data from the DataArray<T> object */
  if(getErrorCode() >= 0)
  {
    dataArrayPaths.push_back(getCellPhasesArrayPath());
  }
  if(m_UseGoodVoxels)
  {
    m_GoodVoxelsPtr = getDataContainerArray()->getPrereqArrayFromPath<DataArray<bool>>(this, getGoodVoxelsArrayPath(), cDims);
    if(nullptr != m_GoodVoxelsPtr.lock())
    {
      m_GoodVoxels = m_GoodVoxelsPtr.lock()->getPointer(0);
    } /* Now assign the raw pointer to data from the DataArray<T> object */
    if(getErrorCode() >= 0)
    {
      dataArrayPaths.push_back(getGoodVoxelsArrayPath());
    }
  }

  m_CrystalStructuresPtr = getDataContainerArray()->getPrereqArrayFromPath<DataArray<unsigned int>>(this, getCrystalStructuresArrayPath(), cDims);
  if(nullptr != m_CrystalStructuresPtr.lock())
  {
    m_CrystalStructures = m_CrystalStructuresPtr.lock()->getPointer(0);
  } /* Now assign the raw pointer to data from the DataArray<T> object */

  getDataContainerArray()->validateNumberOfTuples(this, dataArrayPaths);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void AlignSectionsMutualInformation::find_shifts(std::vector<int64_t>& xshifts, std::vector<int64_t>& yshifts)
{
  DataContainer::Pointer m = getDataContainerArray()->getDataContainer(getDataContainerName());

  int64_t totalPoints = m->getAttributeMatrix(getCellAttributeMatrixName())->getNumberOfTuples();
  m_MIFeaturesPtr = Int32ArrayType::CreateArray((totalPoints * 1), std::string("_INTERNAL_USE_ONLY_MIFeatureIds"), true);
  m_MIFeaturesPtr->initializeWithZeros();
  int32_t* miFeatureIds = m_MIFeaturesPtr->getPointer(0);

  std::ofstream outFile;
  if(getWriteAlignmentShifts())
  {
    outFile.open(getAlignmentShiftFileName().toLatin1().data());
  }

  SizeVec3Type udims = m->getGeometryAs<ImageGeom>()->getDimensions();

  int64_t dims[3] = {
      static_cast<int64_t>(udims[0]),
      static_cast<int64_t>(udims[1]),
      static_cast<int64_t>(udims[2]),
  };

  float disorientation = 0.0f;
  float mindisorientation = std::numeric_limits<float>::max();
  float** mutualinfo12 = nullptr;
  float* mutualinfo1 = nullptr;
  float* mutualinfo2 = nullptr;
  int32_t featurecount1 = 0, featurecount2 = 0;
  int64_t newxshift = 0;
  int64_t newyshift = 0;
  int64_t oldxshift = 0;
  int64_t oldyshift = 0;
  float count = 0.0f;
  int64_t slice = 0;

  int32_t refgnum = 0, curgnum = 0;
  int64_t refposition = 0;
  int64_t curposition = 0;

  form_features_sections();

  std::vector<std::vector<float>> misorients;
  misorients.resize(dims[0]);
  for(int64_t a = 0; a < dims[0]; a++)
  {
    misorients[a].assign(dims[1], 0.0f);
  }

  for(int64_t iter = 1; iter < dims[2]; iter++)
  {
    float prog = ((float)iter / dims[2]) * 100;
    QString ss = QObject::tr("Aligning Sections || Determining Shifts || %1% Complete").arg(QString::number(prog, 'f', 0));
    notifyStatusMessage(ss);
    mindisorientation = std::numeric_limits<float>::max();
    slice = (dims[2] - 1) - iter;
    featurecount1 = featurecounts[slice];
    featurecount2 = featurecounts[slice + 1];
    mutualinfo12 = new float*[featurecount1];
    mutualinfo1 = new float[featurecount1];
    mutualinfo2 = new float[featurecount2];

    for(int32_t a = 0; a < featurecount1; a++)
    {
      mutualinfo1[a] = 0.0f;
      mutualinfo12[a] = new float[featurecount2];
      for(int32_t b = 0; b < featurecount2; b++)
      {
        mutualinfo12[a][b] = 0.0f;
        mutualinfo2[b] = 0.0f;
      }
    }
    oldxshift = -1;
    oldyshift = -1;
    newxshift = 0;
    newyshift = 0;
    for(int64_t a = 0; a < dims[0]; a++)
    {
      for(int64_t b = 0; b < dims[1]; b++)
      {
        misorients[a][b] = 0;
      }
    }
    while(newxshift != oldxshift || newyshift != oldyshift)
    {
      oldxshift = newxshift;
      oldyshift = newyshift;
      for(int32_t j = -3; j < 4; j++)
      {
        for(int32_t k = -3; k < 4; k++)
        {
          disorientation = 0;
          count = 0;
          if(misorients[k + oldxshift + dims[0] / 2][j + oldyshift + dims[1] / 2] == 0 && llabs(k + oldxshift) < (dims[0] / 2) && (j + oldyshift) < (dims[1] / 2))
          {
            for(int64_t l = 0; l < dims[1]; l = l + 4)
            {
              for(int64_t n = 0; n < dims[0]; n = n + 4)
              {
                if((l + j + oldyshift) >= 0 && (l + j + oldyshift) < dims[1] && (n + k + oldxshift) >= 0 && (n + k + oldxshift) < dims[0])
                {
                  refposition = ((slice + 1) * dims[0] * dims[1]) + (l * dims[0]) + n;
                  curposition = (slice * dims[0] * dims[1]) + ((l + j + oldyshift) * dims[0]) + (n + k + oldxshift);
                  refgnum = miFeatureIds[refposition];
                  curgnum = miFeatureIds[curposition];
                  if(curgnum >= 0 && refgnum >= 0)
                  {
                    mutualinfo12[curgnum][refgnum]++;
                    mutualinfo1[curgnum]++;
                    mutualinfo2[refgnum]++;
                    count++;
                  }
                }
                else
                {
                  mutualinfo12[0][0]++;
                  mutualinfo1[0]++;
                  mutualinfo2[0]++;
                }
              }
            }
            float ha = 0.0f;
            float hb = 0.0f;
            float hab = 0.0f;
            for(int32_t b = 0; b < featurecount1; b++)
            {
              mutualinfo1[b] = mutualinfo1[b] / count;
              if(mutualinfo1[b] != 0)
              {
                ha = ha + mutualinfo1[b] * logf(mutualinfo1[b]);
              }
            }
            for(int32_t c = 0; c < featurecount2; c++)
            {
              mutualinfo2[c] = mutualinfo2[c] / float(count);
              if(mutualinfo2[c] != 0)
              {
                hb = hb + mutualinfo2[c] * logf(mutualinfo2[c]);
              }
            }
            for(int32_t b = 0; b < featurecount1; b++)
            {
              for(int32_t c = 0; c < featurecount2; c++)
              {
                mutualinfo12[b][c] = mutualinfo12[b][c] / count;
                if(mutualinfo12[b][c] != 0)
                {
                  hab = hab + mutualinfo12[b][c] * logf(mutualinfo12[b][c]);
                }
                float value = 0.0f;
                if(mutualinfo1[b] > 0 && mutualinfo2[c] > 0)
                {
                  value = (mutualinfo12[b][c] / (mutualinfo1[b] * mutualinfo2[c]));
                }
                if(value != 0)
                {
                  disorientation = disorientation + (mutualinfo12[b][c] * logf(value));
                }
              }
            }
            for(int32_t b = 0; b < featurecount1; b++)
            {
              for(int32_t c = 0; c < featurecount2; c++)
              {
                mutualinfo12[b][c] = 0.0f;
                mutualinfo1[b] = 0.0f;
                mutualinfo2[c] = 0.0f;
              }
            }
            disorientation = 1.0f / disorientation;
            misorients[k + oldxshift + dims[0] / 2][j + oldyshift + dims[1] / 2] = disorientation;
            if(disorientation < mindisorientation)
            {
              newxshift = k + oldxshift;
              newyshift = j + oldyshift;
              mindisorientation = disorientation;
            }
          }
        }
      }
    }
    xshifts[iter] = xshifts[iter - 1] + newxshift;
    yshifts[iter] = yshifts[iter - 1] + newyshift;
    if(getWriteAlignmentShifts())
    {
      outFile << slice << "	" << slice + 1 << "	" << newxshift << "	" << newyshift << "	" << xshifts[iter] << "	" << yshifts[iter] << "\n";
    }
    delete[] mutualinfo1;
    delete[] mutualinfo2;
    for(int32_t i = 0; i < featurecount1; i++)
    {
      delete mutualinfo12[i];
    }
    delete[] mutualinfo12;
    mutualinfo1 = nullptr;
    mutualinfo2 = nullptr;
    mutualinfo12 = nullptr;
  }

  m->getAttributeMatrix(getCellAttributeMatrixName())->removeAttributeArray(SIMPL::CellData::FeatureIds);

  if(getWriteAlignmentShifts())
  {
    outFile.close();
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void AlignSectionsMutualInformation::form_features_sections()
{
  SIMPL_RANDOMNG_NEW()
  DataContainer::Pointer m = getDataContainerArray()->getDataContainer(getDataContainerName());

  SizeVec3Type udims = m->getGeometryAs<ImageGeom>()->getDimensions();

  int64_t dims[3] = {
      static_cast<int64_t>(udims[0]),
      static_cast<int64_t>(udims[1]),
      static_cast<int64_t>(udims[2]),
  };

  int64_t point = 0;
  int64_t seed = 0;
  bool noseeds = false;
  int32_t featurecount = 1;
  int64_t neighbor = 0;

  FloatArrayType::Pointer quats = m_QuatsPtr.lock();

  float w = 0.0f;
  int64_t randx = 0;
  int64_t randy = 0;
  bool good = false;
  int64_t x = 0, y = 0, z = 0;
  int64_t col = 0, row = 0;
  size_t size = 0;
  size_t initialVoxelsListSize = 1000;

  float misorientationTolerance = m_MisorientationTolerance * SIMPLib::Constants::k_PiF / 180.0f;

  m_FeatureCounts->resizeTuples(dims[2]);
  featurecounts = m_FeatureCounts->getPointer(0);

  int32_t* miFeatureIds = m_MIFeaturesPtr->getPointer(0);

  std::vector<int64_t> voxelslist(initialVoxelsListSize, -1);
  int64_t neighpoints[4] = {0, 0, 0, 0};
  neighpoints[0] = -dims[0];
  neighpoints[1] = -1;
  neighpoints[2] = 1;
  neighpoints[3] = dims[0];

  uint32_t phase1 = 0, phase2 = 0;
  float* currentQuatPtr = nullptr;

  for(int64_t slice = 0; slice < dims[2]; slice++)
  {
    float prog = ((float)slice / dims[2]) * 100;
    QString ss = QObject::tr("Aligning Sections || Identifying Features on Sections || %1% Complete").arg(QString::number(prog, 'f', 0));
    notifyStatusMessage(ss);
    featurecount = 1;
    noseeds = false;
    while(!noseeds)
    {
      seed = -1;
      randx = static_cast<int64_t>(float(rg.genrand_res53()) * float(dims[0]));
      randy = static_cast<int64_t>(float(rg.genrand_res53()) * float(dims[1]));
      for(int64_t j = 0; j < dims[1]; ++j)
      {
        for(int64_t i = 0; i < dims[0]; ++i)
        {
          x = randx + i;
          y = randy + j;
          z = slice;
          if(x > dims[0] - 1)
          {
            x = x - dims[0];
          }
          if(y > dims[1] - 1)
          {
            y = y - dims[1];
          }
          point = (z * dims[0] * dims[1]) + (y * dims[0]) + x;
          if((!m_UseGoodVoxels || m_GoodVoxels[point]) && miFeatureIds[point] == 0 && m_CellPhases[point] > 0)
          {
            seed = point;
          }
          if(seed > -1)
          {
            break;
          }
        }
        if(seed > -1)
        {
          break;
        }
      }
      if(seed == -1)
      {
        noseeds = true;
      }
      if(seed >= 0)
      {
        size = 0;
        miFeatureIds[seed] = featurecount;
        voxelslist[size] = seed;
        size++;
        for(size_t j = 0; j < size; ++j)
        {
          int64_t currentpoint = voxelslist[j];
          col = currentpoint % dims[0];
          row = (currentpoint / dims[0]) % dims[1];

          currentQuatPtr = quats->getTuplePointer(currentpoint);
          QuatF q1(currentQuatPtr[0], currentQuatPtr[1], currentQuatPtr[2], currentQuatPtr[3]);
          phase1 = m_CrystalStructures[m_CellPhases[currentpoint]];
          for(int32_t i = 0; i < 4; i++)
          {
            good = true;
            neighbor = currentpoint + neighpoints[i];
            if((i == 0) && row == 0)
            {
              good = false;
            }
            if((i == 3) && row == (dims[1] - 1))
            {
              good = false;
            }
            if((i == 1) && col == 0)
            {
              good = false;
            }
            if((i == 2) && col == (dims[0] - 1))
            {
              good = false;
            }
            if(good && miFeatureIds[neighbor] <= 0 && m_CellPhases[neighbor] > 0)
            {
              w = std::numeric_limits<float>::max();
              currentQuatPtr = quats->getTuplePointer(neighbor);

              QuatF q2(currentQuatPtr[0], currentQuatPtr[1], currentQuatPtr[2], currentQuatPtr[3]);
              phase2 = m_CrystalStructures[m_CellPhases[neighbor]];
              if(phase1 == phase2)
              {
                OrientationF axisAngle = m_OrientationOps[phase1]->calculateMisorientation(q1, q2);
                w = axisAngle[3];
              }
              if(w < misorientationTolerance)
              {
                miFeatureIds[neighbor] = featurecount;
                voxelslist[size] = neighbor;
                size++;
                if(std::vector<int64_t>::size_type(size) >= voxelslist.size())
                {
                  size = voxelslist.size();
                  voxelslist.resize(size + initialVoxelsListSize);
                  for(std::vector<int64_t>::size_type v = size; v < voxelslist.size(); ++v)
                  {
                    voxelslist[v] = -1;
                  }
                }
              }
            }
          }
        }
        voxelslist.erase(std::remove(voxelslist.begin(), voxelslist.end(), -1), voxelslist.end());
        featurecount++;
        voxelslist.assign(initialVoxelsListSize, -1);
      }
    }
    featurecounts[slice] = featurecount;
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void AlignSectionsMutualInformation::execute()
{
  dataCheck();
  if(getErrorCode() < 0)
  {
    return;
  }

  AlignSections::execute();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
AbstractFilter::Pointer AlignSectionsMutualInformation::newFilterInstance(bool copyFilterParameters) const
{
  AlignSectionsMutualInformation::Pointer filter = AlignSectionsMutualInformation::New();
  if(copyFilterParameters)
  {
    copyFilterParameterInstanceVariables(filter.get());
  }
  return filter;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString AlignSectionsMutualInformation::getCompiledLibraryName() const
{
  return ReconstructionConstants::ReconstructionBaseName;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString AlignSectionsMutualInformation::getBrandingString() const
{
  return "Reconstruction";
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString AlignSectionsMutualInformation::getFilterVersion() const
{
  QString version;
  QTextStream vStream(&version);
  vStream << Reconstruction::Version::Major() << "." << Reconstruction::Version::Minor() << "." << Reconstruction::Version::Patch();
  return version;
}
// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString AlignSectionsMutualInformation::getGroupName() const
{
  return SIMPL::FilterGroups::ReconstructionFilters;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QUuid AlignSectionsMutualInformation::getUuid() const
{
  return QUuid("{61c5519b-5561-58b8-a522-2ce1324e244d}");
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString AlignSectionsMutualInformation::getSubGroupName() const
{
  return SIMPL::FilterSubGroups::AlignmentFilters;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString AlignSectionsMutualInformation::getHumanLabel() const
{
  return "Align Sections (Mutual Information)";
}

// -----------------------------------------------------------------------------
AlignSectionsMutualInformation::Pointer AlignSectionsMutualInformation::NullPointer()
{
  return Pointer(static_cast<Self*>(nullptr));
}

// -----------------------------------------------------------------------------
std::shared_ptr<AlignSectionsMutualInformation> AlignSectionsMutualInformation::New()
{
  struct make_shared_enabler : public AlignSectionsMutualInformation
  {
  };
  std::shared_ptr<make_shared_enabler> val = std::make_shared<make_shared_enabler>();
  val->setupFilterParameters();
  return val;
}

// -----------------------------------------------------------------------------
QString AlignSectionsMutualInformation::getNameOfClass() const
{
  return QString("AlignSectionsMutualInformation");
}

// -----------------------------------------------------------------------------
QString AlignSectionsMutualInformation::ClassName()
{
  return QString("AlignSectionsMutualInformation");
}

// -----------------------------------------------------------------------------
void AlignSectionsMutualInformation::setMisorientationTolerance(float value)
{
  m_MisorientationTolerance = value;
}

// -----------------------------------------------------------------------------
float AlignSectionsMutualInformation::getMisorientationTolerance() const
{
  return m_MisorientationTolerance;
}

// -----------------------------------------------------------------------------
void AlignSectionsMutualInformation::setUseGoodVoxels(bool value)
{
  m_UseGoodVoxels = value;
}

// -----------------------------------------------------------------------------
bool AlignSectionsMutualInformation::getUseGoodVoxels() const
{
  return m_UseGoodVoxels;
}

// -----------------------------------------------------------------------------
void AlignSectionsMutualInformation::setQuatsArrayPath(const DataArrayPath& value)
{
  m_QuatsArrayPath = value;
}

// -----------------------------------------------------------------------------
DataArrayPath AlignSectionsMutualInformation::getQuatsArrayPath() const
{
  return m_QuatsArrayPath;
}

// -----------------------------------------------------------------------------
void AlignSectionsMutualInformation::setCellPhasesArrayPath(const DataArrayPath& value)
{
  m_CellPhasesArrayPath = value;
}

// -----------------------------------------------------------------------------
DataArrayPath AlignSectionsMutualInformation::getCellPhasesArrayPath() const
{
  return m_CellPhasesArrayPath;
}

// -----------------------------------------------------------------------------
void AlignSectionsMutualInformation::setGoodVoxelsArrayPath(const DataArrayPath& value)
{
  m_GoodVoxelsArrayPath = value;
}

// -----------------------------------------------------------------------------
DataArrayPath AlignSectionsMutualInformation::getGoodVoxelsArrayPath() const
{
  return m_GoodVoxelsArrayPath;
}

// -----------------------------------------------------------------------------
void AlignSectionsMutualInformation::setCrystalStructuresArrayPath(const DataArrayPath& value)
{
  m_CrystalStructuresArrayPath = value;
}

// -----------------------------------------------------------------------------
DataArrayPath AlignSectionsMutualInformation::getCrystalStructuresArrayPath() const
{
  return m_CrystalStructuresArrayPath;
}

// -----------------------------------------------------------------------------
void AlignSectionsMutualInformation::setFeatureCounts(const std::shared_ptr<DataArray<int32_t>>& value)
{
  m_FeatureCounts = value;
}

// -----------------------------------------------------------------------------
std::shared_ptr<DataArray<int32_t>> AlignSectionsMutualInformation::getFeatureCounts() const
{
  return m_FeatureCounts;
}
