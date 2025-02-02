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
#include "FindSurfaceFeatures.h"

#include <QtCore/QTextStream>

#include "SIMPLib/Common/Constants.h"
#include "SIMPLib/DataContainers/DataContainer.h"
#include "SIMPLib/DataContainers/DataContainerArray.h"
#include "SIMPLib/FilterParameters/AbstractFilterParametersReader.h"
#include "SIMPLib/FilterParameters/DataArrayCreationFilterParameter.h"
#include "SIMPLib/FilterParameters/DataArraySelectionFilterParameter.h"
#include "SIMPLib/FilterParameters/SeparatorFilterParameter.h"
#include "SIMPLib/Geometry/ImageGeom.h"

#include "Generic/GenericConstants.h"
#include "Generic/GenericVersion.h"

/* Create Enumerations to allow the created Attribute Arrays to take part in renaming */
enum createdPathID : RenameDataPath::DataID_t
{
  DataArrayID30 = 30,
  DataArrayID31 = 31,
};

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
FindSurfaceFeatures::FindSurfaceFeatures() = default;

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
FindSurfaceFeatures::~FindSurfaceFeatures() = default;

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void FindSurfaceFeatures::setupFilterParameters()
{
  FilterParameterVectorType parameters;
  parameters.push_back(SeparatorFilterParameter::Create("Cell Data", FilterParameter::Category::RequiredArray));
  {
    DataArraySelectionFilterParameter::RequirementType req = DataArraySelectionFilterParameter::CreateRequirement(SIMPL::TypeNames::Int32, 1, AttributeMatrix::Type::Cell, IGeometry::Type::Image);
    parameters.push_back(SIMPL_NEW_DA_SELECTION_FP("Feature Ids", FeatureIdsArrayPath, FilterParameter::Category::RequiredArray, FindSurfaceFeatures, req));
  }
  parameters.push_back(SeparatorFilterParameter::Create("Cell Feature Data", FilterParameter::Category::CreatedArray));
  {
    DataArrayCreationFilterParameter::RequirementType req = DataArrayCreationFilterParameter::CreateRequirement(AttributeMatrix::Category::Feature);
    req.dcGeometryTypes = IGeometry::Types(1, IGeometry::Type::Image);
    parameters.push_back(SIMPL_NEW_DA_CREATION_FP("Surface Features", SurfaceFeaturesArrayPath, FilterParameter::Category::CreatedArray, FindSurfaceFeatures, req));
  }
  setFilterParameters(parameters);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void FindSurfaceFeatures::readFilterParameters(AbstractFilterParametersReader* reader, int index)
{
  reader->openFilterGroup(this, index);
  setSurfaceFeaturesArrayPath(reader->readDataArrayPath("SurfaceFeaturesArrayPath", getSurfaceFeaturesArrayPath()));
  setFeatureIdsArrayPath(reader->readDataArrayPath("FeatureIdsArrayPath", getFeatureIdsArrayPath()));
  reader->closeFilterGroup();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void FindSurfaceFeatures::initialize()
{
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void FindSurfaceFeatures::dataCheck()
{
  clearErrorCode();
  clearWarningCode();

  getDataContainerArray()->getPrereqGeometryFromDataContainer<ImageGeom>(this, getFeatureIdsArrayPath().getDataContainerName());

  std::vector<size_t> cDims(1, 1);
  m_FeatureIdsPtr = getDataContainerArray()->getPrereqArrayFromPath<DataArray<int32_t>>(this, getFeatureIdsArrayPath(), cDims);
  if(nullptr != m_FeatureIdsPtr.lock())
  {
    m_FeatureIds = m_FeatureIdsPtr.lock()->getPointer(0);
  } /* Now assign the raw pointer to data from the DataArray<T> object */

  m_SurfaceFeaturesPtr = getDataContainerArray()->createNonPrereqArrayFromPath<DataArray<bool>>(this, getSurfaceFeaturesArrayPath(), false, cDims, "", DataArrayID31);
  if(nullptr != m_SurfaceFeaturesPtr.lock())
  {
    m_SurfaceFeatures = m_SurfaceFeaturesPtr.lock()->getPointer(0);
  } /* Now assign the raw pointer to data from the DataArray<T> object */
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void FindSurfaceFeatures::find_surfacefeatures()
{
  DataContainer::Pointer m = getDataContainerArray()->getDataContainer(getFeatureIdsArrayPath().getDataContainerName());

  int64_t xPoints = static_cast<int64_t>(m->getGeometryAs<ImageGeom>()->getXPoints());
  int64_t yPoints = static_cast<int64_t>(m->getGeometryAs<ImageGeom>()->getYPoints());
  int64_t zPoints = static_cast<int64_t>(m->getGeometryAs<ImageGeom>()->getZPoints());

  int64_t zStride = 0, yStride = 0;
  for(int64_t i = 0; i < zPoints; i++)
  {
    zStride = i * xPoints * yPoints;
    for(int64_t j = 0; j < yPoints; j++)
    {
      yStride = j * xPoints;
      for(int64_t k = 0; k < xPoints; k++)
      {
        int32_t gnum = m_FeatureIds[zStride + yStride + k];
        if(!m_SurfaceFeatures[gnum])
        {
          if(k <= 0)
          {
            m_SurfaceFeatures[gnum] = true;
          }
          if(k >= xPoints - 1)
          {
            m_SurfaceFeatures[gnum] = true;
          }
          if(j <= 0)
          {
            m_SurfaceFeatures[gnum] = true;
          }
          if(j >= yPoints - 1)
          {
            m_SurfaceFeatures[gnum] = true;
          }
          if(i <= 0)
          {
            m_SurfaceFeatures[gnum] = true;
          }
          if(i >= zPoints - 1)
          {
            m_SurfaceFeatures[gnum] = true;
          }
          if(!m_SurfaceFeatures[gnum])
          {
            if(m_FeatureIds[zStride + yStride + k - 1] == 0)
            {
              m_SurfaceFeatures[gnum] = true;
            }
            if(m_FeatureIds[zStride + yStride + k + 1] == 0)
            {
              m_SurfaceFeatures[gnum] = true;
            }
            if(m_FeatureIds[zStride + yStride + k - xPoints] == 0)
            {
              m_SurfaceFeatures[gnum] = true;
            }
            if(m_FeatureIds[zStride + yStride + k + xPoints] == 0)
            {
              m_SurfaceFeatures[gnum] = true;
            }
            if(m_FeatureIds[zStride + yStride + k - (xPoints * yPoints)] == 0)
            {
              m_SurfaceFeatures[gnum] = true;
            }
            if(m_FeatureIds[zStride + yStride + k + (xPoints * yPoints)] == 0)
            {
              m_SurfaceFeatures[gnum] = true;
            }
          }
        }
      }
    }
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void FindSurfaceFeatures::find_surfacefeatures2D()
{
  DataContainer::Pointer m = getDataContainerArray()->getDataContainer(getFeatureIdsArrayPath().getDataContainerName());

  int64_t xPoints = 0, yPoints = 0;

  if(m->getGeometryAs<ImageGeom>()->getXPoints() == 1)
  {
    xPoints = static_cast<int64_t>(m->getGeometryAs<ImageGeom>()->getYPoints());
    yPoints = static_cast<int64_t>(m->getGeometryAs<ImageGeom>()->getZPoints());
  }
  if(m->getGeometryAs<ImageGeom>()->getYPoints() == 1)
  {
    xPoints = static_cast<int64_t>(m->getGeometryAs<ImageGeom>()->getXPoints());
    yPoints = static_cast<int64_t>(m->getGeometryAs<ImageGeom>()->getZPoints());
  }
  if(m->getGeometryAs<ImageGeom>()->getZPoints() == 1)
  {
    xPoints = static_cast<int64_t>(m->getGeometryAs<ImageGeom>()->getXPoints());
    yPoints = static_cast<int64_t>(m->getGeometryAs<ImageGeom>()->getYPoints());
  }

  int64_t yStride;
  for(int64_t j = 0; j < yPoints; j++)
  {
    yStride = j * xPoints;
    for(int64_t k = 0; k < xPoints; k++)
    {
      int32_t gnum = m_FeatureIds[yStride + k];
      if(!m_SurfaceFeatures[gnum])
      {
        if(k <= 0)
        {
          m_SurfaceFeatures[gnum] = true;
        }
        if(k >= xPoints - 1)
        {
          m_SurfaceFeatures[gnum] = true;
        }
        if(j <= 0)
        {
          m_SurfaceFeatures[gnum] = true;
        }
        if(j >= yPoints - 1)
        {
          m_SurfaceFeatures[gnum] = true;
        }
        if(!m_SurfaceFeatures[gnum])
        {
          if(m_FeatureIds[yStride + k - 1] == 0)
          {
            m_SurfaceFeatures[gnum] = true;
          }
          if(m_FeatureIds[yStride + k + 1] == 0)
          {
            m_SurfaceFeatures[gnum] = true;
          }
          if(m_FeatureIds[yStride + k - static_cast<int64_t>(m->getGeometryAs<ImageGeom>()->getXPoints())] == 0)
          {
            m_SurfaceFeatures[gnum] = true;
          }
          if(m_FeatureIds[yStride + k + static_cast<int64_t>(m->getGeometryAs<ImageGeom>()->getXPoints())] == 0)
          {
            m_SurfaceFeatures[gnum] = true;
          }
        }
      }
    }
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void FindSurfaceFeatures::execute()
{
  dataCheck();
  if(getErrorCode() < 0)
  {
    return;
  }

  DataContainer::Pointer m = getDataContainerArray()->getDataContainer(getFeatureIdsArrayPath().getDataContainerName());

  if(m->getGeometryAs<ImageGeom>()->getXPoints() > 1 && m->getGeometryAs<ImageGeom>()->getYPoints() > 1 && m->getGeometryAs<ImageGeom>()->getZPoints() > 1)
  {
    find_surfacefeatures();
  }
  if(m->getGeometryAs<ImageGeom>()->getXPoints() == 1 || m->getGeometryAs<ImageGeom>()->getYPoints() == 1 || m->getGeometryAs<ImageGeom>()->getZPoints() == 1)
  {
    find_surfacefeatures2D();
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
AbstractFilter::Pointer FindSurfaceFeatures::newFilterInstance(bool copyFilterParameters) const
{
  FindSurfaceFeatures::Pointer filter = FindSurfaceFeatures::New();
  if(copyFilterParameters)
  {
    copyFilterParameterInstanceVariables(filter.get());
  }
  return filter;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString FindSurfaceFeatures::getCompiledLibraryName() const
{
  return GenericConstants::GenericBaseName;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString FindSurfaceFeatures::getBrandingString() const
{
  return "Generic";
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString FindSurfaceFeatures::getFilterVersion() const
{
  QString version;
  QTextStream vStream(&version);
  vStream << Generic::Version::Major() << "." << Generic::Version::Minor() << "." << Generic::Version::Patch();
  return version;
}
// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString FindSurfaceFeatures::getGroupName() const
{
  return SIMPL::FilterGroups::Generic;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QUuid FindSurfaceFeatures::getUuid() const
{
  return QUuid("{d2b0ae3d-686a-5dc0-a844-66bc0dc8f3cb}");
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString FindSurfaceFeatures::getSubGroupName() const
{
  return SIMPL::FilterSubGroups::SpatialFilters;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString FindSurfaceFeatures::getHumanLabel() const
{
  return "Find Surface Features";
}

// -----------------------------------------------------------------------------
FindSurfaceFeatures::Pointer FindSurfaceFeatures::NullPointer()
{
  return Pointer(static_cast<Self*>(nullptr));
}

// -----------------------------------------------------------------------------
std::shared_ptr<FindSurfaceFeatures> FindSurfaceFeatures::New()
{
  struct make_shared_enabler : public FindSurfaceFeatures
  {
  };
  std::shared_ptr<make_shared_enabler> val = std::make_shared<make_shared_enabler>();
  val->setupFilterParameters();
  return val;
}

// -----------------------------------------------------------------------------
QString FindSurfaceFeatures::getNameOfClass() const
{
  return QString("FindSurfaceFeatures");
}

// -----------------------------------------------------------------------------
QString FindSurfaceFeatures::ClassName()
{
  return QString("FindSurfaceFeatures");
}

// -----------------------------------------------------------------------------
void FindSurfaceFeatures::setFeatureIdsArrayPath(const DataArrayPath& value)
{
  m_FeatureIdsArrayPath = value;
}

// -----------------------------------------------------------------------------
DataArrayPath FindSurfaceFeatures::getFeatureIdsArrayPath() const
{
  return m_FeatureIdsArrayPath;
}

// -----------------------------------------------------------------------------
void FindSurfaceFeatures::setSurfaceFeaturesArrayPath(const DataArrayPath& value)
{
  m_SurfaceFeaturesArrayPath = value;
}

// -----------------------------------------------------------------------------
DataArrayPath FindSurfaceFeatures::getSurfaceFeaturesArrayPath() const
{
  return m_SurfaceFeaturesArrayPath;
}
