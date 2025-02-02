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
#include "AlignSectionsFeature.h"

#include <fstream>

#include <QtCore/QTextStream>

#include "SIMPLib/Common/Constants.h"
#include "SIMPLib/DataContainers/DataContainer.h"
#include "SIMPLib/DataContainers/DataContainerArray.h"
#include "SIMPLib/FilterParameters/AbstractFilterParametersReader.h"
#include "SIMPLib/FilterParameters/DataArraySelectionFilterParameter.h"
#include "SIMPLib/FilterParameters/SeparatorFilterParameter.h"
#include "SIMPLib/Geometry/ImageGeom.h"

#include "Reconstruction/ReconstructionConstants.h"
#include "Reconstruction/ReconstructionVersion.h"

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
AlignSectionsFeature::AlignSectionsFeature()
: m_GoodVoxelsArrayPath(SIMPL::Defaults::ImageDataContainerName, SIMPL::Defaults::CellAttributeMatrixName, SIMPL::CellData::Mask)
{
  // only setting up the child parameters because the parent constructor has already been called
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
AlignSectionsFeature::~AlignSectionsFeature() = default;

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void AlignSectionsFeature::setupFilterParameters()
{
  // getting the current parameters that were set by the parent and adding to it before resetting it
  AlignSections::setupFilterParameters();
  FilterParameterVectorType parameters = getFilterParameters();
  parameters.push_back(SeparatorFilterParameter::Create("Cell Data", FilterParameter::Category::RequiredArray));
  {
    DataArraySelectionFilterParameter::RequirementType req = DataArraySelectionFilterParameter::CreateRequirement(SIMPL::TypeNames::Bool, 1, AttributeMatrix::Type::Cell, IGeometry::Type::Image);
    parameters.push_back(SIMPL_NEW_DA_SELECTION_FP("Mask", GoodVoxelsArrayPath, FilterParameter::Category::RequiredArray, AlignSectionsFeature, req));
  }
  setFilterParameters(parameters);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void AlignSectionsFeature::readFilterParameters(AbstractFilterParametersReader* reader, int index)
{
  AlignSections::readFilterParameters(reader, index);
  reader->openFilterGroup(this, index);
  setGoodVoxelsArrayPath(reader->readDataArrayPath("GoodVoxelsArrayPath", getGoodVoxelsArrayPath()));
  reader->closeFilterGroup();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void AlignSectionsFeature::initialize()
{
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void AlignSectionsFeature::dataCheck()
{
  clearErrorCode();
  clearWarningCode();

  // Set the DataContainerName and AttributematrixName for the Parent Class (AlignSections) to Use.
  // These are checked for validity in the Parent Class dataCheck
  setDataContainerName(DataArrayPath(m_GoodVoxelsArrayPath.getDataContainerName(), "", ""));
  setCellAttributeMatrixName(m_GoodVoxelsArrayPath.getAttributeMatrixName());

  AlignSections::dataCheck();
  if(getErrorCode() < 0)
  {
    return;
  }

  std::vector<size_t> cDims(1, 1);
  m_GoodVoxelsPtr = getDataContainerArray()->getPrereqArrayFromPath<DataArray<bool>>(this, getGoodVoxelsArrayPath(), cDims);
  if(nullptr != m_GoodVoxelsPtr.lock())
  {
    m_GoodVoxels = m_GoodVoxelsPtr.lock()->getPointer(0);
  } /* Now assign the raw pointer to data from the DataArray<T> object */
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void AlignSectionsFeature::find_shifts(std::vector<int64_t>& xshifts, std::vector<int64_t>& yshifts)
{
  DataContainer::Pointer m = getDataContainerArray()->getDataContainer(getDataContainerName());

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
  int32_t newxshift = 0;
  int32_t newyshift = 0;
  int32_t oldxshift = 0;
  int32_t oldyshift = 0;
  float count = 0.0f;
  int64_t slice = 0;
  int64_t refposition = 0;
  int64_t curposition = 0;
  std::vector<std::vector<float>> misorients(dims[0]);

  for(int64_t a = 0; a < dims[0]; a++)
  {
    misorients[a].assign(dims[1], 0.0f);
  }

  for(int64_t iter = 1; iter < dims[2]; iter++)
  {
    QString ss = QObject::tr("Aligning Sections || Determining Shifts || %1% Complete").arg(((float)iter / dims[2]) * 100);
    notifyStatusMessage(ss);
    mindisorientation = std::numeric_limits<float>::max();
    slice = (dims[2] - 1) - iter;
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
          disorientation = 0.0f;
          count = 0.0f;
          if(misorients[k + oldxshift + dims[0] / 2][j + oldyshift + dims[1] / 2] == 0.0f && abs(k + oldxshift) < (dims[0] / 2) && (j + oldyshift) < (dims[1] / 2))
          {
            for(int64_t l = 0; l < dims[1]; l = l + 4)
            {
              for(int64_t n = 0; n < dims[0]; n = n + 4)
              {
                if((l + j + oldyshift) >= 0 && (l + j + oldyshift) < dims[1] && (n + k + oldxshift) >= 0 && (n + k + oldxshift) < dims[0])
                {
                  refposition = ((slice + 1) * dims[0] * dims[1]) + (l * dims[0]) + n;
                  curposition = (slice * dims[0] * dims[1]) + ((l + j + oldyshift) * dims[0]) + (n + k + oldxshift);
                  if(m_GoodVoxels[refposition] != m_GoodVoxels[curposition])
                  {
                    disorientation++;
                  }
                  count++;
                }
                else
                {
                }
              }
            }
            disorientation = disorientation / count;
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
      outFile << slice << "	" << slice + 1 << "	" << newxshift << "	" << newyshift << "	" << xshifts[iter] << "	" << yshifts[iter] << std::endl;
    }
  }
  if(getWriteAlignmentShifts())
  {
    outFile.close();
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void AlignSectionsFeature::execute()
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
AbstractFilter::Pointer AlignSectionsFeature::newFilterInstance(bool copyFilterParameters) const
{
  AlignSectionsFeature::Pointer filter = AlignSectionsFeature::New();
  if(copyFilterParameters)
  {
    copyFilterParameterInstanceVariables(filter.get());
  }
  return filter;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString AlignSectionsFeature::getCompiledLibraryName() const
{
  return ReconstructionConstants::ReconstructionBaseName;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString AlignSectionsFeature::getBrandingString() const
{
  return "Reconstruction";
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString AlignSectionsFeature::getFilterVersion() const
{
  QString version;
  QTextStream vStream(&version);
  vStream << Reconstruction::Version::Major() << "." << Reconstruction::Version::Minor() << "." << Reconstruction::Version::Patch();
  return version;
}
// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString AlignSectionsFeature::getGroupName() const
{
  return SIMPL::FilterGroups::ReconstructionFilters;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QUuid AlignSectionsFeature::getUuid() const
{
  return QUuid("{2bb76fa9-934a-51df-bff1-b0c866971706}");
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString AlignSectionsFeature::getSubGroupName() const
{
  return SIMPL::FilterSubGroups::AlignmentFilters;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString AlignSectionsFeature::getHumanLabel() const
{
  return "Align Sections (Feature)";
}

// -----------------------------------------------------------------------------
AlignSectionsFeature::Pointer AlignSectionsFeature::NullPointer()
{
  return Pointer(static_cast<Self*>(nullptr));
}

// -----------------------------------------------------------------------------
std::shared_ptr<AlignSectionsFeature> AlignSectionsFeature::New()
{
  struct make_shared_enabler : public AlignSectionsFeature
  {
  };
  std::shared_ptr<make_shared_enabler> val = std::make_shared<make_shared_enabler>();
  val->setupFilterParameters();
  return val;
}

// -----------------------------------------------------------------------------
QString AlignSectionsFeature::getNameOfClass() const
{
  return QString("AlignSectionsFeature");
}

// -----------------------------------------------------------------------------
QString AlignSectionsFeature::ClassName()
{
  return QString("AlignSectionsFeature");
}

// -----------------------------------------------------------------------------
void AlignSectionsFeature::setGoodVoxelsArrayPath(const DataArrayPath& value)
{
  m_GoodVoxelsArrayPath = value;
}

// -----------------------------------------------------------------------------
DataArrayPath AlignSectionsFeature::getGoodVoxelsArrayPath() const
{
  return m_GoodVoxelsArrayPath;
}
