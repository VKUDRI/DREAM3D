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
#include "FindBoundingBoxFeatures.h"

#include <QtCore/QTextStream>

#include "SIMPLib/Common/Constants.h"
#include "SIMPLib/DataContainers/DataContainer.h"
#include "SIMPLib/DataContainers/DataContainerArray.h"
#include "SIMPLib/FilterParameters/AbstractFilterParametersReader.h"
#include "SIMPLib/FilterParameters/DataArraySelectionFilterParameter.h"
#include "SIMPLib/FilterParameters/LinkedBooleanFilterParameter.h"
#include "SIMPLib/FilterParameters/LinkedPathCreationFilterParameter.h"
#include "SIMPLib/FilterParameters/SeparatorFilterParameter.h"
#include "SIMPLib/FilterParameters/StringFilterParameter.h"
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
FindBoundingBoxFeatures::FindBoundingBoxFeatures() = default;

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
FindBoundingBoxFeatures::~FindBoundingBoxFeatures() = default;

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void FindBoundingBoxFeatures::setupFilterParameters()
{
  FilterParameterVectorType parameters;
  std::vector<QString> linkedProps = {"PhasesArrayPath"};
  parameters.push_back(SIMPL_NEW_LINKED_BOOL_FP("Apply Phase by Phase", CalcByPhase, FilterParameter::Category::Parameter, FindBoundingBoxFeatures, linkedProps));
  parameters.push_back(SeparatorFilterParameter::Create("Cell Feature Data", FilterParameter::Category::RequiredArray));
  {
    DataArraySelectionFilterParameter::RequirementType req =
        DataArraySelectionFilterParameter::CreateRequirement(SIMPL::TypeNames::Float, 3, AttributeMatrix::Type::CellFeature, IGeometry::Type::Image);
    parameters.push_back(SIMPL_NEW_DA_SELECTION_FP("Centroids", CentroidsArrayPath, FilterParameter::Category::RequiredArray, FindBoundingBoxFeatures, req));
  }
  {
    DataArraySelectionFilterParameter::RequirementType req =
        DataArraySelectionFilterParameter::CreateRequirement(SIMPL::TypeNames::Bool, 1, AttributeMatrix::Type::CellFeature, IGeometry::Type::Image);
    parameters.push_back(SIMPL_NEW_DA_SELECTION_FP("Surface Features", SurfaceFeaturesArrayPath, FilterParameter::Category::RequiredArray, FindBoundingBoxFeatures, req));
  }
  {
    DataArraySelectionFilterParameter::RequirementType req =
        DataArraySelectionFilterParameter::CreateRequirement(SIMPL::TypeNames::Int32, 1, AttributeMatrix::Type::CellFeature, IGeometry::Type::Image);
    parameters.push_back(SIMPL_NEW_DA_SELECTION_FP("Phases", PhasesArrayPath, FilterParameter::Category::RequiredArray, FindBoundingBoxFeatures, req));
  }
  parameters.push_back(SeparatorFilterParameter::Create("Cell Feature Data", FilterParameter::Category::CreatedArray));
  parameters.push_back(
      SIMPL_NEW_DA_WITH_LINKED_AM_FP("Biased Features", BiasedFeaturesArrayName, CentroidsArrayPath, CentroidsArrayPath, FilterParameter::Category::CreatedArray, FindBoundingBoxFeatures));
  setFilterParameters(parameters);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void FindBoundingBoxFeatures::readFilterParameters(AbstractFilterParametersReader* reader, int index)
{
  reader->openFilterGroup(this, index);
  setCalcByPhase(reader->readValue("CalcByPhase", getCalcByPhase()));
  setPhasesArrayPath(reader->readDataArrayPath("PhasesArrayPath", getPhasesArrayPath()));
  setBiasedFeaturesArrayName(reader->readString("BiasedFeaturesArrayName", getBiasedFeaturesArrayName()));
  setSurfaceFeaturesArrayPath(reader->readDataArrayPath("SurfaceFeaturesArrayPath", getSurfaceFeaturesArrayPath()));
  setCentroidsArrayPath(reader->readDataArrayPath("CentroidsArrayPath", getCentroidsArrayPath()));
  reader->closeFilterGroup();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void FindBoundingBoxFeatures::initialize()
{
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void FindBoundingBoxFeatures::dataCheck()
{
  clearErrorCode();
  clearWarningCode();

  DataArrayPath tempPath;

  getDataContainerArray()->getPrereqGeometryFromDataContainer<ImageGeom>(this, getCentroidsArrayPath().getDataContainerName());

  QVector<DataArrayPath> dataArrayPaths;

  std::vector<size_t> cDims(1, 3);
  m_CentroidsPtr = getDataContainerArray()->getPrereqArrayFromPath<DataArray<float>>(this, getCentroidsArrayPath(), cDims);
  if(nullptr != m_CentroidsPtr.lock())
  {
    m_Centroids = m_CentroidsPtr.lock()->getPointer(0);
  } /* Now assign the raw pointer to data from the DataArray<T> object */
  if(getErrorCode() >= 0)
  {
    dataArrayPaths.push_back(getCentroidsArrayPath());
  }

  cDims[0] = 1;
  m_SurfaceFeaturesPtr = getDataContainerArray()->getPrereqArrayFromPath<DataArray<bool>>(this, getSurfaceFeaturesArrayPath(), cDims);
  if(nullptr != m_SurfaceFeaturesPtr.lock())
  {
    m_SurfaceFeatures = m_SurfaceFeaturesPtr.lock()->getPointer(0);
  } /* Now assign the raw pointer to data from the DataArray<T> object */
  if(getErrorCode() >= 0)
  {
    dataArrayPaths.push_back(getSurfaceFeaturesArrayPath());
  }

  tempPath.update(getCentroidsArrayPath().getDataContainerName(), getCentroidsArrayPath().getAttributeMatrixName(), getBiasedFeaturesArrayName());
  m_BiasedFeaturesPtr = getDataContainerArray()->createNonPrereqArrayFromPath<DataArray<bool>>(this, tempPath, false, cDims, "", DataArrayID31);
  if(nullptr != m_BiasedFeaturesPtr.lock())
  {
    m_BiasedFeatures = m_BiasedFeaturesPtr.lock()->getPointer(0);
  } /* Now assign the raw pointer to data from the DataArray<T> object */

  if(getCalcByPhase())
  {
    m_PhasesPtr = getDataContainerArray()->getPrereqArrayFromPath<DataArray<int32_t>>(this, getPhasesArrayPath(), cDims);
    if(nullptr != m_PhasesPtr.lock())
    {
      m_Phases = m_PhasesPtr.lock()->getPointer(0);
    } /* Now assign the raw pointer to data from the DataArray<T> object */
    if(getErrorCode() >= 0)
    {
      dataArrayPaths.push_back(getPhasesArrayPath());
    }
  }

  getDataContainerArray()->validateNumberOfTuples(this, dataArrayPaths);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void FindBoundingBoxFeatures::find_boundingboxfeatures()
{
  DataContainer::Pointer m = getDataContainerArray()->getDataContainer(getCentroidsArrayPath().getDataContainerName());
  ImageGeom::Pointer imageGeom = m->getGeometryAs<ImageGeom>();

  size_t size = m_CentroidsPtr.lock()->getNumberOfTuples();
  float boundbox[6] = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
  float coords[6] = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
  float x = 0.0f;
  float y = 0.0f;
  float z = 0.0f;
  float dist[7] = {
      0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
  };
  float mindist = std::numeric_limits<float>::max();
  int32_t sidetomove = 0;
  int32_t move = 0;

  // loop first to determine number of phases if calcByPhase is being used
  int32_t numPhases = 1;
  if(m_CalcByPhase)
  {
    for(size_t i = 1; i < size; i++)
    {
      if(m_Phases[i] > numPhases)
      {
        numPhases = m_Phases[i];
      }
    }
  }
  for(int32_t iter = 1; iter <= numPhases; iter++)
  {
    if(m_CalcByPhase)
    {
      QString ss = QObject::tr("Working on Phase %1 of %2").arg(iter).arg(numPhases);
      notifyStatusMessage(ss);
    }
    // reset boundbox for each phase
    imageGeom->getBoundingBox(boundbox);

    for(size_t i = 1; i < size; i++)
    {
      if(m_SurfaceFeatures[i] && (!m_CalcByPhase || m_Phases[i] == iter))
      {
        sidetomove = 0;
        move = 1;
        mindist = std::numeric_limits<float>::max();
        x = m_Centroids[3 * i];
        y = m_Centroids[3 * i + 1];
        z = m_Centroids[3 * i + 2];
        coords[0] = x;
        coords[1] = x;
        coords[2] = y;
        coords[3] = y;
        coords[4] = z;
        coords[5] = z;
        for(int32_t j = 1; j < 7; j++)
        {
          dist[j] = std::numeric_limits<float>::max();
          if(j % 2 == 1)
          {
            if(coords[j - 1] > boundbox[j - 1])
            {
              dist[j] = (coords[j - 1] - boundbox[j - 1]);
            }
            if(coords[j - 1] <= boundbox[j - 1])
            {
              move = 0;
            }
          }
          if(j % 2 == 0)
          {
            if(coords[j - 1] < boundbox[j - 1])
            {
              dist[j] = (boundbox[j - 1] - coords[j - 1]);
            }
            if(coords[j - 1] >= boundbox[j - 1])
            {
              move = 0;
            }
          }
          if(dist[j] < mindist)
          {
            mindist = dist[j];
            sidetomove = j - 1;
          }
        }
        if(move == 1)
        {
          boundbox[sidetomove] = coords[sidetomove];
        }
      }
    }
    for(size_t j = 1; j < size; j++)
    {
      if(!m_CalcByPhase || m_Phases[j] == iter)
      {
        if(m_Centroids[3 * j] <= boundbox[0])
        {
          m_BiasedFeatures[j] = true;
        }
        if(m_Centroids[3 * j] >= boundbox[1])
        {
          m_BiasedFeatures[j] = true;
        }
        if(m_Centroids[3 * j + 1] <= boundbox[2])
        {
          m_BiasedFeatures[j] = true;
        }
        if(m_Centroids[3 * j + 1] >= boundbox[3])
        {
          m_BiasedFeatures[j] = true;
        }
        if(m_Centroids[3 * j + 2] <= boundbox[4])
        {
          m_BiasedFeatures[j] = true;
        }
        if(m_Centroids[3 * j + 2] >= boundbox[5])
        {
          m_BiasedFeatures[j] = true;
        }
      }
    }
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void FindBoundingBoxFeatures::find_boundingboxfeatures2D()
{
  DataContainer::Pointer m = getDataContainerArray()->getDataContainer(getCentroidsArrayPath().getDataContainerName());
  ImageGeom::Pointer imageGeom = m->getGeometryAs<ImageGeom>();

  size_t size = m_CentroidsPtr.lock()->getNumberOfTuples();
  float xOrigin = 0.0f;
  float yOrigin = 0.0f;
  // float zOrigin = 0.0f;

  float boundbox[6] = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
  float coords[4] = {0.0f, 0.0f, 0.0f, 0.0f};
  float x = 0.0f;
  float y = 0.0f;
  float dist[5] = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
  float mindist = std::numeric_limits<float>::max();
  int32_t sidetomove = 0;
  int32_t move = 0;

  int32_t xPoints = 0, yPoints = 0;
  FloatVec3Type spacing;

  if(imageGeom->getXPoints() == 1)
  {
    xPoints = imageGeom->getYPoints();
    yPoints = imageGeom->getZPoints();
    spacing = imageGeom->getSpacing();
  }
  if(imageGeom->getYPoints() == 1)
  {
    xPoints = imageGeom->getXPoints();
    yPoints = imageGeom->getZPoints();
    spacing = imageGeom->getSpacing();
  }
  if(imageGeom->getZPoints() == 1)
  {
    xPoints = imageGeom->getXPoints();
    yPoints = imageGeom->getYPoints();
    spacing = imageGeom->getSpacing();
  }

  boundbox[0] = xOrigin;
  boundbox[1] = xOrigin + xPoints * spacing[0];
  boundbox[2] = yOrigin;
  boundbox[3] = yOrigin + yPoints * spacing[1];

  for(size_t i = 1; i < size; i++)
  {
    if(m_SurfaceFeatures[i])
    {
      sidetomove = 0;
      move = 1;
      mindist = std::numeric_limits<float>::max();
      x = m_Centroids[3 * i];
      y = m_Centroids[3 * i + 1];
      coords[0] = x;
      coords[1] = x;
      coords[2] = y;
      coords[3] = y;
      for(int32_t j = 1; j < 5; j++)
      {
        dist[j] = std::numeric_limits<float>::max();
        if(j % 2 == 1)
        {
          if(coords[j - 1] > boundbox[j - 1])
          {
            dist[j] = (coords[j - 1] - boundbox[j - 1]);
          }
          if(coords[j - 1] <= boundbox[j - 1])
          {
            move = 0;
          }
        }
        if(j % 2 == 0)
        {
          if(coords[j - 1] < boundbox[j - 1])
          {
            dist[j] = (boundbox[j - 1] - coords[j - 1]);
          }
          if(coords[j - 1] >= boundbox[j - 1])
          {
            move = 0;
          }
        }
        if(dist[j] < mindist)
        {
          mindist = dist[j];
          sidetomove = j - 1;
        }
      }
      if(move == 1)
      {
        boundbox[sidetomove] = coords[sidetomove];
      }
    }
  }
  for(size_t j = 1; j < size; j++)
  {
    if(m_Centroids[3 * j] <= boundbox[0])
    {
      m_BiasedFeatures[j] = true;
    }
    if(m_Centroids[3 * j] >= boundbox[1])
    {
      m_BiasedFeatures[j] = true;
    }
    if(m_Centroids[3 * j + 1] <= boundbox[2])
    {
      m_BiasedFeatures[j] = true;
    }
    if(m_Centroids[3 * j + 1] >= boundbox[3])
    {
      m_BiasedFeatures[j] = true;
    }
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void FindBoundingBoxFeatures::execute()
{
  dataCheck();
  if(getErrorCode() < 0)
  {
    return;
  }

  DataContainer::Pointer m = getDataContainerArray()->getDataContainer(getCentroidsArrayPath().getDataContainerName());
  ImageGeom::Pointer imageGeom = m->getGeometryAs<ImageGeom>();

  if(imageGeom->getXPoints() > 1 && imageGeom->getYPoints() > 1 && imageGeom->getZPoints() > 1)
  {
    find_boundingboxfeatures();
  }
  if(imageGeom->getXPoints() == 1 || imageGeom->getYPoints() == 1 || imageGeom->getZPoints() == 1)
  {
    find_boundingboxfeatures2D();
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
AbstractFilter::Pointer FindBoundingBoxFeatures::newFilterInstance(bool copyFilterParameters) const
{
  FindBoundingBoxFeatures::Pointer filter = FindBoundingBoxFeatures::New();
  if(copyFilterParameters)
  {
    copyFilterParameterInstanceVariables(filter.get());
  }
  return filter;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString FindBoundingBoxFeatures::getCompiledLibraryName() const
{
  return GenericConstants::GenericBaseName;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString FindBoundingBoxFeatures::getBrandingString() const
{
  return "Generic";
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString FindBoundingBoxFeatures::getFilterVersion() const
{
  QString version;
  QTextStream vStream(&version);
  vStream << Generic::Version::Major() << "." << Generic::Version::Minor() << "." << Generic::Version::Patch();
  return version;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString FindBoundingBoxFeatures::getGroupName() const
{
  return SIMPL::FilterGroups::Generic;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QUuid FindBoundingBoxFeatures::getUuid() const
{
  return QUuid("{450c2f00-9ddf-56e1-b4c1-0e74e7ad2349}");
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString FindBoundingBoxFeatures::getSubGroupName() const
{
  return SIMPL::FilterSubGroups::SpatialFilters;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString FindBoundingBoxFeatures::getHumanLabel() const
{
  return "Find Biased Features (Bounding Box)";
}

// -----------------------------------------------------------------------------
FindBoundingBoxFeatures::Pointer FindBoundingBoxFeatures::NullPointer()
{
  return Pointer(static_cast<Self*>(nullptr));
}

// -----------------------------------------------------------------------------
std::shared_ptr<FindBoundingBoxFeatures> FindBoundingBoxFeatures::New()
{
  struct make_shared_enabler : public FindBoundingBoxFeatures
  {
  };
  std::shared_ptr<make_shared_enabler> val = std::make_shared<make_shared_enabler>();
  val->setupFilterParameters();
  return val;
}

// -----------------------------------------------------------------------------
QString FindBoundingBoxFeatures::getNameOfClass() const
{
  return QString("FindBoundingBoxFeatures");
}

// -----------------------------------------------------------------------------
QString FindBoundingBoxFeatures::ClassName()
{
  return QString("FindBoundingBoxFeatures");
}

// -----------------------------------------------------------------------------
void FindBoundingBoxFeatures::setCalcByPhase(bool value)
{
  m_CalcByPhase = value;
}

// -----------------------------------------------------------------------------
bool FindBoundingBoxFeatures::getCalcByPhase() const
{
  return m_CalcByPhase;
}

// -----------------------------------------------------------------------------
void FindBoundingBoxFeatures::setCentroidsArrayPath(const DataArrayPath& value)
{
  m_CentroidsArrayPath = value;
}

// -----------------------------------------------------------------------------
DataArrayPath FindBoundingBoxFeatures::getCentroidsArrayPath() const
{
  return m_CentroidsArrayPath;
}

// -----------------------------------------------------------------------------
void FindBoundingBoxFeatures::setPhasesArrayPath(const DataArrayPath& value)
{
  m_PhasesArrayPath = value;
}

// -----------------------------------------------------------------------------
DataArrayPath FindBoundingBoxFeatures::getPhasesArrayPath() const
{
  return m_PhasesArrayPath;
}

// -----------------------------------------------------------------------------
void FindBoundingBoxFeatures::setSurfaceFeaturesArrayPath(const DataArrayPath& value)
{
  m_SurfaceFeaturesArrayPath = value;
}

// -----------------------------------------------------------------------------
DataArrayPath FindBoundingBoxFeatures::getSurfaceFeaturesArrayPath() const
{
  return m_SurfaceFeaturesArrayPath;
}

// -----------------------------------------------------------------------------
void FindBoundingBoxFeatures::setBiasedFeaturesArrayName(const QString& value)
{
  m_BiasedFeaturesArrayName = value;
}

// -----------------------------------------------------------------------------
QString FindBoundingBoxFeatures::getBiasedFeaturesArrayName() const
{
  return m_BiasedFeaturesArrayName;
}
