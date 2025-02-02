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
#include "FindEuclideanDistMap.h"

#ifdef SIMPL_USE_PARALLEL_ALGORITHMS

#include <tbb/blocked_range.h>
#include <tbb/parallel_for.h>
#include <tbb/task_group.h>
#include <tbb/tick_count.h>
#endif

#include <QtCore/QTextStream>

#include "SIMPLib/Common/Constants.h"
#include "SIMPLib/DataContainers/DataContainer.h"
#include "SIMPLib/DataContainers/DataContainerArray.h"
#include "SIMPLib/FilterParameters/AbstractFilterParametersReader.h"
#include "SIMPLib/FilterParameters/BooleanFilterParameter.h"
#include "SIMPLib/FilterParameters/DataArraySelectionFilterParameter.h"
#include "SIMPLib/FilterParameters/LinkedBooleanFilterParameter.h"
#include "SIMPLib/FilterParameters/LinkedPathCreationFilterParameter.h"
#include "SIMPLib/FilterParameters/SeparatorFilterParameter.h"
#include "SIMPLib/FilterParameters/StringFilterParameter.h"
#include "SIMPLib/Geometry/ImageGeom.h"
#include "SIMPLib/Math/SIMPLibMath.h"

#include "StatsToolbox/StatsToolboxConstants.h"
#include "StatsToolbox/StatsToolboxVersion.h"

/* Create Enumerations to allow the created Attribute Arrays to take part in renaming */
enum createdPathID : RenameDataPath::DataID_t
{
  DataArrayID30 = 30,
  DataArrayID31 = 31,
  DataArrayID32 = 32,
  DataArrayID33 = 33,
  DataArrayID34 = 34,
  DataArrayID35 = 35,
  DataArrayID36 = 36,
};

/**
 * @brief The ComputeDistanceMapImpl class implements a threaded algorithm that computes the  distance map
 * for each point in the supplied volume
 */
template <typename T>
class ComputeDistanceMapImpl
{
  DataContainer::Pointer m_DataContainer;
  int32_t* m_FeatureIds;
  int32_t* m_NearestNeighbors;
  bool m_CalcManhattanDist;
  T* m_GBManhattanDistances;
  T* m_TJManhattanDistances;
  T* m_QPManhattanDistances;
  FindEuclideanDistMap::MapType m_MapType;

public:
  ComputeDistanceMapImpl(DataContainer::Pointer datacontainer, int32_t* fIds, int32_t* nearNeighs, bool calcManhattanDist, T* gbDists, T* tjDists, T* qpDists, FindEuclideanDistMap::MapType mapType)
  : m_DataContainer(datacontainer)
  , m_FeatureIds(fIds)
  , m_NearestNeighbors(nearNeighs)
  , m_CalcManhattanDist(calcManhattanDist)
  , m_GBManhattanDistances(gbDists)
  , m_TJManhattanDistances(tjDists)
  , m_QPManhattanDistances(qpDists)
  , m_MapType(mapType)
  {
  }

  virtual ~ComputeDistanceMapImpl() = default;

  void operator()() const
  {
    ImageGeom::Pointer imageGeom = m_DataContainer->getGeometryAs<ImageGeom>();
    size_t totalPoints = imageGeom->getNumberOfElements();
    double Distance = 0.0;
    size_t count = 1;
    size_t changed = 1;
    size_t neighpoint = 0;
    int64_t nearestneighbor;
    int64_t neighbors[6] = {0, 0, 0, 0, 0, 0};
    int64_t xpoints = static_cast<int64_t>(imageGeom->getXPoints());
    int64_t ypoints = static_cast<int64_t>(imageGeom->getYPoints());
    int64_t zpoints = static_cast<int64_t>(imageGeom->getZPoints());
    FloatVec3Type spacing = imageGeom->getSpacing();

    neighbors[0] = -xpoints * ypoints;
    neighbors[1] = -xpoints;
    neighbors[2] = -1;
    neighbors[3] = 1;
    neighbors[4] = xpoints;
    neighbors[5] = xpoints * ypoints;

    // Use a std::vector to get an auto cleaned up array thus not needing the 'delete' keyword later on.
    std::vector<int32_t> voxNN(totalPoints, 0);
    int32_t* voxel_NearestNeighbor = &(voxNN.front());
    std::vector<double> voxEDist(totalPoints, 0.0);
    double* voxel_Distance = &(voxEDist.front());

    Distance = 0;
    for(size_t a = 0; a < totalPoints; ++a)
    {
      if(m_NearestNeighbors[a * 3 + static_cast<uint32_t>(m_MapType)] >= 0)
      {
        voxel_NearestNeighbor[a] = static_cast<int32_t>(a);
      } // if voxel is boundary voxel, then want to use itself as nearest boundary voxel
      else
      {
        voxel_NearestNeighbor[a] = -1;
      }
      if(m_MapType == FindEuclideanDistMap::MapType::FeatureBoundary)
      {
        voxel_Distance[a] = static_cast<double>(m_GBManhattanDistances[a]);
      }
      else if(m_MapType == FindEuclideanDistMap::MapType::TripleJunction)
      {
        voxel_Distance[a] = static_cast<double>(m_TJManhattanDistances[a]);
      }
      else if(m_MapType == FindEuclideanDistMap::MapType::QuadPoint)
      {
        voxel_Distance[a] = static_cast<double>(m_QPManhattanDistances[a]);
      }
    }

    // ------------- Calculate the Manhattan Distance ----------------
    count = 1;
    changed = 1;
    int64_t i = 0;
    int64_t zBlock = xpoints * ypoints;
    int64_t zStride = 0, yStride = 0;
    char mask[6] = {0, 0, 0, 0, 0, 0};
    while(count > 0 && changed > 0)
    {
      count = 0;
      changed = 0;
      Distance++;

      for(int64_t z = 0; z < zpoints; ++z)
      {
        zStride = z * zBlock;
        mask[0] = mask[5] = 1;
        if(z == 0)
        {
          mask[0] = 0;
        }
        if(z == zpoints - 1)
        {
          mask[5] = 0;
        }

        for(int64_t y = 0; y < ypoints; ++y)
        {
          yStride = y * xpoints;
          mask[1] = mask[4] = 1;
          if(y == 0)
          {
            mask[1] = 0;
          }
          if(y == ypoints - 1)
          {
            mask[4] = 0;
          }

          for(int64_t x = 0; x < xpoints; ++x)
          {
            mask[2] = mask[3] = 1;
            if(x == 0)
            {
              mask[2] = 0;
            }
            if(x == xpoints - 1)
            {
              mask[3] = 0;
            }

            i = zStride + yStride + x;
            if(voxel_NearestNeighbor[i] == -1 && m_FeatureIds[i] > 0)
            {
              count++;
              for(int32_t j = 0; j < 6; j++)
              {
                neighpoint = i + neighbors[j];
                if(mask[j] == 1)
                {
                  if(voxel_Distance[neighpoint] != -1.0)
                  {
                    voxel_NearestNeighbor[i] = voxel_NearestNeighbor[neighpoint];
                  }
                }
              }
            }
          }
        }
      }
      for(size_t j = 0; j < totalPoints; ++j)
      {
        if(voxel_NearestNeighbor[j] != -1 && voxel_Distance[j] == -1.0 && m_FeatureIds[j] > 0)
        {
          changed++;
          voxel_Distance[j] = Distance;
        }
      }
    }

    // ------------- Calculate the Euclidian Distance ----------------

    if(m_CalcManhattanDist == false)
    {
      double x1 = 0.0, x2 = 0.0, y1 = 0.0, y2 = 0.0, z1 = 0.0, z2 = 0.0;
      double dist = 0.0;
      double oneOverzBlock = 1.0 / double(zBlock);
      double oneOverxpoints = 1.0 / double(xpoints);
      for(int64_t m = 0; m < zpoints; m++)
      {
        zStride = m * zBlock;
        for(int64_t n = 0; n < ypoints; n++)
        {
          yStride = n * xpoints;
          for(int64_t p = 0; p < xpoints; p++)
          {
            x1 = static_cast<double>(p) * spacing[0];
            y1 = static_cast<double>(n) * spacing[1];
            z1 = static_cast<double>(m) * spacing[2];
            nearestneighbor = voxel_NearestNeighbor[zStride + yStride + p];
            if(nearestneighbor >= 0)
            {
              x2 = spacing[0] * double(nearestneighbor % xpoints);                           // find_xcoord(nearestneighbor);
              y2 = spacing[1] * double(int64_t(nearestneighbor * oneOverxpoints) % ypoints); // find_ycoord(nearestneighbor);
              z2 = spacing[2] * floor(nearestneighbor * oneOverzBlock);                      // find_zcoord(nearestneighbor);
              dist = ((x1 - x2) * (x1 - x2)) + ((y1 - y2) * (y1 - y2)) + ((z1 - z2) * (z1 - z2));
              dist = sqrt(dist);
              voxel_Distance[zStride + yStride + p] = dist;
            }
          }
        }
      }
    }
    for(size_t a = 0; a < totalPoints; ++a)
    {
      m_NearestNeighbors[a * 3 + static_cast<uint32_t>(m_MapType)] = voxel_NearestNeighbor[a];
      if(m_MapType == FindEuclideanDistMap::MapType::FeatureBoundary)
      {
        m_GBManhattanDistances[a] = static_cast<T>(voxel_Distance[a]);
      }
      else if(m_MapType == FindEuclideanDistMap::MapType::TripleJunction)
      {
        m_TJManhattanDistances[a] = static_cast<T>(voxel_Distance[a]);
      }
      else if(m_MapType == FindEuclideanDistMap::MapType::QuadPoint)
      {
        m_QPManhattanDistances[a] = static_cast<T>(voxel_Distance[a]);
      }
    }
  }
};

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
FindEuclideanDistMap::FindEuclideanDistMap() = default;

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
FindEuclideanDistMap::~FindEuclideanDistMap() = default;

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void FindEuclideanDistMap::setupFilterParameters()
{
  FilterParameterVectorType parameters;
  parameters.push_back(SIMPL_NEW_BOOL_FP("Calculate Manhattan Distance", CalcManhattanDist, FilterParameter::Category::Parameter, FindEuclideanDistMap));
  std::vector<QString> linkedProps = {"GBDistancesArrayName"};

  parameters.push_back(SIMPL_NEW_LINKED_BOOL_FP("Calculate Distance to Boundaries", DoBoundaries, FilterParameter::Category::Parameter, FindEuclideanDistMap, linkedProps));
  linkedProps.clear();
  linkedProps.push_back("TJDistancesArrayName");
  parameters.push_back(SIMPL_NEW_LINKED_BOOL_FP("Calculate Distance to Triple Lines", DoTripleLines, FilterParameter::Category::Parameter, FindEuclideanDistMap, linkedProps));
  linkedProps.clear();
  linkedProps.push_back("QPDistancesArrayName");
  parameters.push_back(SIMPL_NEW_LINKED_BOOL_FP("Calculate Distance to Quadruple Points", DoQuadPoints, FilterParameter::Category::Parameter, FindEuclideanDistMap, linkedProps));
  linkedProps.clear();
  linkedProps.push_back("NearestNeighborsArrayName");
  parameters.push_back(SIMPL_NEW_LINKED_BOOL_FP("Store the Nearest Boundary Cells", SaveNearestNeighbors, FilterParameter::Category::Parameter, FindEuclideanDistMap, linkedProps));

  parameters.push_back(SeparatorFilterParameter::Create("Cell Data", FilterParameter::Category::RequiredArray));
  {
    DataArraySelectionFilterParameter::RequirementType req = DataArraySelectionFilterParameter::CreateRequirement(SIMPL::TypeNames::Int32, 1, AttributeMatrix::Type::Cell, IGeometry::Type::Image);
    parameters.push_back(SIMPL_NEW_DA_SELECTION_FP("Feature Ids", FeatureIdsArrayPath, FilterParameter::Category::RequiredArray, FindEuclideanDistMap, req));
  }

  parameters.push_back(SeparatorFilterParameter::Create("Cell Data", FilterParameter::Category::CreatedArray));
  parameters.push_back(
      SIMPL_NEW_DA_WITH_LINKED_AM_FP("Boundary Distances", GBDistancesArrayName, FeatureIdsArrayPath, FeatureIdsArrayPath, FilterParameter::Category::CreatedArray, FindEuclideanDistMap));
  parameters.push_back(
      SIMPL_NEW_DA_WITH_LINKED_AM_FP("Triple Line Distances", TJDistancesArrayName, FeatureIdsArrayPath, FeatureIdsArrayPath, FilterParameter::Category::CreatedArray, FindEuclideanDistMap));
  parameters.push_back(
      SIMPL_NEW_DA_WITH_LINKED_AM_FP("Quadruple Point Distances", QPDistancesArrayName, FeatureIdsArrayPath, FeatureIdsArrayPath, FilterParameter::Category::CreatedArray, FindEuclideanDistMap));
  parameters.push_back(
      SIMPL_NEW_DA_WITH_LINKED_AM_FP("Nearest Neighbors", NearestNeighborsArrayName, FeatureIdsArrayPath, FeatureIdsArrayPath, FilterParameter::Category::CreatedArray, FindEuclideanDistMap));

  setFilterParameters(parameters);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void FindEuclideanDistMap::readFilterParameters(AbstractFilterParametersReader* reader, int index)
{
  reader->openFilterGroup(this, index);
  setNearestNeighborsArrayName(reader->readString("NearestNeighborsArrayName", getNearestNeighborsArrayName()));
  setQPDistancesArrayName(reader->readString("QPEuclideanDistancesArrayName", getQPDistancesArrayName()));
  setTJDistancesArrayName(reader->readString("TJEuclideanDistancesArrayName", getTJDistancesArrayName()));
  setGBDistancesArrayName(reader->readString("GBEuclideanDistancesArrayName", getGBDistancesArrayName()));
  setFeatureIdsArrayPath(reader->readDataArrayPath("FeatureIdsArrayPath", getFeatureIdsArrayPath()));
  setDoBoundaries(reader->readValue("DoBoundaries", getDoBoundaries()));
  setDoTripleLines(reader->readValue("DoTripleLines", getDoTripleLines()));
  setDoQuadPoints(reader->readValue("DoQuadPoints", getDoQuadPoints()));
  setSaveNearestNeighbors(reader->readValue("SaveNearestNeighbors", getSaveNearestNeighbors()));
  setCalcManhattanDist(reader->readValue("CalcOnlyManhattanDist", getCalcManhattanDist()));
  reader->closeFilterGroup();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void FindEuclideanDistMap::initialize()
{
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void FindEuclideanDistMap::dataCheck()
{
  clearErrorCode();
  clearWarningCode();
  DataArrayPath tempPath;

  getDataContainerArray()->getPrereqGeometryFromDataContainer<ImageGeom>(this, getFeatureIdsArrayPath().getDataContainerName());

  std::vector<size_t> cDims(1, 1);
  m_FeatureIdsPtr = getDataContainerArray()->getPrereqArrayFromPath<DataArray<int32_t>>(this, getFeatureIdsArrayPath(), cDims);
  if(nullptr != m_FeatureIdsPtr.lock())
  {
    m_FeatureIds = m_FeatureIdsPtr.lock()->getPointer(0);
  }

  if(m_DoBoundaries)
  {
    tempPath.update(getFeatureIdsArrayPath().getDataContainerName(), getFeatureIdsArrayPath().getAttributeMatrixName(), getGBDistancesArrayName());
    if(m_CalcManhattanDist)
    {
      m_GBManhattanDistancesPtr = getDataContainerArray()->createNonPrereqArrayFromPath<DataArray<int32_t>>(this, tempPath, -1, cDims, "", DataArrayID31);
      if(nullptr != m_GBManhattanDistancesPtr.lock())
      {
        m_GBManhattanDistances = m_GBManhattanDistancesPtr.lock()->getPointer(0);
      }
    }
    else
    {
      m_GBEuclideanDistancesPtr = getDataContainerArray()->createNonPrereqArrayFromPath<DataArray<float>>(this, tempPath, -1, cDims, "", DataArrayID32);
      if(nullptr != m_GBEuclideanDistancesPtr.lock())
      {
        m_GBEuclideanDistances = m_GBEuclideanDistancesPtr.lock()->getPointer(0);
      }
    }
  }

  if(m_DoTripleLines)
  {
    tempPath.update(getFeatureIdsArrayPath().getDataContainerName(), getFeatureIdsArrayPath().getAttributeMatrixName(), getTJDistancesArrayName());
    if(m_CalcManhattanDist)
    {
      m_TJManhattanDistancesPtr = getDataContainerArray()->createNonPrereqArrayFromPath<DataArray<int32_t>>(this, tempPath, -1, cDims, "", DataArrayID33);
      if(nullptr != m_TJManhattanDistancesPtr.lock())
      {
        m_TJManhattanDistances = m_TJManhattanDistancesPtr.lock()->getPointer(0);
      }
    }
    else
    {
      m_TJEuclideanDistancesPtr = getDataContainerArray()->createNonPrereqArrayFromPath<DataArray<float>>(this, tempPath, -1, cDims, "", DataArrayID34);
      if(nullptr != m_TJEuclideanDistancesPtr.lock())
      {
        m_TJEuclideanDistances = m_TJEuclideanDistancesPtr.lock()->getPointer(0);
      }
    }
  }

  if(m_DoQuadPoints)
  {
    tempPath.update(getFeatureIdsArrayPath().getDataContainerName(), getFeatureIdsArrayPath().getAttributeMatrixName(), getQPDistancesArrayName());
    if(m_CalcManhattanDist)
    {
      m_QPManhattanDistancesPtr = getDataContainerArray()->createNonPrereqArrayFromPath<DataArray<int32_t>>(this, tempPath, -1, cDims, "", DataArrayID35);
      if(nullptr != m_QPManhattanDistancesPtr.lock())
      {
        m_QPManhattanDistances = m_QPManhattanDistancesPtr.lock()->getPointer(0);
      }
    }
    else
    {
      m_QPEuclideanDistancesPtr = getDataContainerArray()->createNonPrereqArrayFromPath<DataArray<float>>(this, tempPath, -1, cDims, "", DataArrayID36);
      if(nullptr != m_QPEuclideanDistancesPtr.lock())
      {
        m_QPEuclideanDistances = m_QPEuclideanDistancesPtr.lock()->getPointer(0);
      }
    }
  }

  cDims[0] = 3;
  tempPath.update(getFeatureIdsArrayPath().getDataContainerName(), getFeatureIdsArrayPath().getAttributeMatrixName(), getNearestNeighborsArrayName());
  m_NearestNeighborsPtr = getDataContainerArray()->createNonPrereqArrayFromPath<DataArray<int32_t>>(this, tempPath, 0, cDims);
  if(nullptr != m_NearestNeighborsPtr.lock())
  {
    m_NearestNeighbors = m_NearestNeighborsPtr.lock()->getPointer(0);
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void FindEuclideanDistMap::preflight()
{
  setInPreflight(true);
  Q_EMIT preflightAboutToExecute();
  Q_EMIT updateFilterParameters(this);
  dataCheck();
  if(getErrorCode() < 0)
  {
    Q_EMIT preflightExecuted();
    setInPreflight(false);
    return;
  }
  if(!m_SaveNearestNeighbors)
  {
    DataContainer::Pointer m = getDataContainerArray()->getDataContainer(getFeatureIdsArrayPath().getDataContainerName());
    AttributeMatrix::Pointer attrMat = m->getAttributeMatrix(getFeatureIdsArrayPath().getAttributeMatrixName());
    attrMat->removeAttributeArray(getNearestNeighborsArrayName());
  }
  Q_EMIT preflightExecuted();
  setInPreflight(false);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void FindEuclideanDistMap::findDistanceMap()
{
  DataContainer::Pointer m = getDataContainerArray()->getDataContainer(m_FeatureIdsArrayPath.getDataContainerName());

  size_t totalPoints = m_FeatureIdsPtr.lock()->getNumberOfTuples();

  for(size_t i = 0; i < totalPoints; i++)
  {
    if(m_DoBoundaries)
    {
      if(m_CalcManhattanDist)
      {
        m_GBManhattanDistances[i] = -1;
      }
      else
      {
        m_GBEuclideanDistances[i] = -1;
      }
    }
    if(m_DoTripleLines)
    {
      if(m_CalcManhattanDist)
      {
        m_TJManhattanDistances[i] = -1;
      }
      else
      {
        m_TJEuclideanDistances[i] = -1;
      }
    }
    if(m_DoQuadPoints)
    {
      if(m_CalcManhattanDist)
      {
        m_QPManhattanDistances[i] = -1;
      }
      else
      {
        m_QPEuclideanDistances[i] = -1;
      }
    }
  }

  int64_t column = 0, row = 0, plane = 0;
  bool good = false, add = true;
  int32_t feature = 0;
  std::vector<int32_t> coordination;

  SizeVec3Type udims = m->getGeometryAs<ImageGeom>()->getDimensions();

  int64_t dims[3] = {
      static_cast<int64_t>(udims[0]),
      static_cast<int64_t>(udims[1]),
      static_cast<int64_t>(udims[2]),
  };

  int64_t neighbor = 0;
  int64_t neighbors[6] = {0, 0, 0, 0, 0, 0};
  neighbors[0] = -dims[0] * dims[1];
  neighbors[1] = -dims[0];
  neighbors[2] = -1;
  neighbors[3] = 1;
  neighbors[4] = dims[0];
  neighbors[5] = dims[0] * dims[1];

  size_t xPoints = (m->getGeometryAs<ImageGeom>()->getXPoints());
  size_t yPoints = (m->getGeometryAs<ImageGeom>()->getYPoints());
  size_t zPoints = (m->getGeometryAs<ImageGeom>()->getZPoints());

  for(size_t a = 0; a < totalPoints; ++a)
  {
    feature = m_FeatureIds[a];
    if(feature > 0)
    {
      column = static_cast<int64_t>(a % xPoints);
      row = static_cast<int64_t>((a / xPoints) % yPoints);
      plane = static_cast<int64_t>(a / (xPoints * yPoints));
      for(int32_t k = 0; k < 6; k++)
      {
        good = true;
        neighbor = static_cast<int64_t>(a + neighbors[k]);
        if(k == 0 && plane == 0)
        {
          good = false;
        }
        if(k == 5 && plane == static_cast<int64_t>(zPoints - 1))
        {
          good = false;
        }
        if(k == 1 && row == 0)
        {
          good = false;
        }
        if(k == 4 && row == static_cast<int64_t>(yPoints - 1))
        {
          good = false;
        }
        if(k == 2 && column == 0)
        {
          good = false;
        }
        if(k == 3 && column == static_cast<int64_t>(xPoints - 1))
        {
          good = false;
        }
        if(good && m_FeatureIds[neighbor] != feature && m_FeatureIds[neighbor] >= 0)
        {
          add = true;
          for(size_t i = 0; i < coordination.size(); i++)
          {
            if(m_FeatureIds[neighbor] == coordination[i])
            {
              add = false;
            }
          }
          if(add)
          {
            coordination.push_back(m_FeatureIds[neighbor]);
          }
        }
      }
      if(coordination.empty())
      {
        m_NearestNeighbors[a * 3 + 0] = -1;
        m_NearestNeighbors[a * 3 + 1] = -1;
        m_NearestNeighbors[a * 3 + 2] = -1;
      }
      if(!coordination.empty() && m_DoBoundaries)
      {
        if(m_CalcManhattanDist)
        {
          m_GBManhattanDistances[a] = 0;
        }
        else
        {
          m_GBEuclideanDistances[a] = 0.0f;
        }

        m_NearestNeighbors[a * 3 + 0] = coordination[0];
        m_NearestNeighbors[a * 3 + 1] = -1;
        m_NearestNeighbors[a * 3 + 2] = -1;
      }
      if(coordination.size() >= 2 && m_DoTripleLines)
      {
        if(m_CalcManhattanDist)
        {
          m_TJManhattanDistances[a] = 0;
        }
        else
        {
          m_TJEuclideanDistances[a] = 0.0f;
        }

        m_NearestNeighbors[a * 3 + 0] = coordination[0];
        m_NearestNeighbors[a * 3 + 1] = coordination[0];
        m_NearestNeighbors[a * 3 + 2] = -1;
      }
      if(coordination.size() > 2 && m_DoQuadPoints)
      {
        if(m_CalcManhattanDist)
        {
          m_QPManhattanDistances[a] = 0;
        }
        else
        {
          m_QPEuclideanDistances[a] = 0.0f;
        }

        m_NearestNeighbors[a * 3 + 0] = coordination[0];
        m_NearestNeighbors[a * 3 + 1] = coordination[0];
        m_NearestNeighbors[a * 3 + 2] = coordination[0];
      }
      coordination.resize(0);
    }
  }

#ifdef SIMPL_USE_PARALLEL_ALGORITHMS
  if(true)
  {
    std::shared_ptr<tbb::task_group> g(new tbb::task_group);
    if(m_DoBoundaries)
    {
      if(m_CalcManhattanDist)
      {
        g->run(ComputeDistanceMapImpl<int32_t>(m, m_FeatureIds, m_NearestNeighbors, m_CalcManhattanDist, m_GBManhattanDistances, m_TJManhattanDistances, m_QPManhattanDistances,
                                               MapType::FeatureBoundary));
      }
      else
      {
        g->run(
            ComputeDistanceMapImpl<float>(m, m_FeatureIds, m_NearestNeighbors, m_CalcManhattanDist, m_GBEuclideanDistances, m_TJEuclideanDistances, m_QPEuclideanDistances, MapType::FeatureBoundary));
      }
    }
    if(m_DoTripleLines)
    {
      if(m_CalcManhattanDist)
      {
        g->run(
            ComputeDistanceMapImpl<int32_t>(m, m_FeatureIds, m_NearestNeighbors, m_CalcManhattanDist, m_GBManhattanDistances, m_TJManhattanDistances, m_QPManhattanDistances, MapType::TripleJunction));
      }
      else
      {
        g->run(
            ComputeDistanceMapImpl<float>(m, m_FeatureIds, m_NearestNeighbors, m_CalcManhattanDist, m_GBEuclideanDistances, m_TJEuclideanDistances, m_QPEuclideanDistances, MapType::TripleJunction));
      }
    }
    if(m_DoQuadPoints)
    {
      if(m_CalcManhattanDist)
      {
        g->run(ComputeDistanceMapImpl<int32_t>(m, m_FeatureIds, m_NearestNeighbors, m_CalcManhattanDist, m_GBManhattanDistances, m_TJManhattanDistances, m_QPManhattanDistances, MapType::QuadPoint));
      }
      else
      {
        g->run(ComputeDistanceMapImpl<float>(m, m_FeatureIds, m_NearestNeighbors, m_CalcManhattanDist, m_GBEuclideanDistances, m_TJEuclideanDistances, m_QPEuclideanDistances, MapType::QuadPoint));
      }
    }
    g->wait();
  }
  else
#endif
  {
    for(int32_t i = 0; i < 3; i++)
    {
      FindEuclideanDistMap::MapType mapType = FindEuclideanDistMap::MapType::FeatureBoundary;
      if(i == 1)
      {
        mapType = FindEuclideanDistMap::MapType::TripleJunction;
      }
      else if(i == 2)
      {
        mapType = FindEuclideanDistMap::MapType::QuadPoint;
      }

      if((i == 0 && m_DoBoundaries) || (i == 1 && m_DoTripleLines) || (i == 2 && m_DoQuadPoints))
      {
        if(m_CalcManhattanDist)
        {
          ComputeDistanceMapImpl<int32_t> f(m, m_FeatureIds, m_NearestNeighbors, m_CalcManhattanDist, m_GBManhattanDistances, m_TJManhattanDistances, m_QPManhattanDistances, mapType);
          f();
        }
        else
        {
          ComputeDistanceMapImpl<float> f(m, m_FeatureIds, m_NearestNeighbors, m_CalcManhattanDist, m_GBEuclideanDistances, m_TJEuclideanDistances, m_QPEuclideanDistances, mapType);
          f();
        }
      }
    }
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void FindEuclideanDistMap::execute()
{
  dataCheck();
  if(getErrorCode() < 0)
  {
    return;
  }

  findDistanceMap();

  if(!m_SaveNearestNeighbors)
  {
    DataContainer::Pointer m = getDataContainerArray()->getDataContainer(getFeatureIdsArrayPath().getDataContainerName());
    AttributeMatrix::Pointer attrMat = m->getAttributeMatrix(getFeatureIdsArrayPath().getAttributeMatrixName());
    attrMat->removeAttributeArray(getNearestNeighborsArrayName());
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
AbstractFilter::Pointer FindEuclideanDistMap::newFilterInstance(bool copyFilterParameters) const
{
  FindEuclideanDistMap::Pointer filter = FindEuclideanDistMap::New();
  if(copyFilterParameters)
  {
    copyFilterParameterInstanceVariables(filter.get());
  }
  return filter;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString FindEuclideanDistMap::getCompiledLibraryName() const
{
  return StatsToolboxConstants::StatsToolboxBaseName;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString FindEuclideanDistMap::getBrandingString() const
{
  return "Statistics";
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString FindEuclideanDistMap::getFilterVersion() const
{
  QString version;
  QTextStream vStream(&version);
  vStream << StatsToolbox::Version::Major() << "." << StatsToolbox::Version::Minor() << "." << StatsToolbox::Version::Patch();
  return version;
}
// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString FindEuclideanDistMap::getGroupName() const
{
  return SIMPL::FilterGroups::StatisticsFilters;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QUuid FindEuclideanDistMap::getUuid() const
{
  return QUuid("{933e4b2d-dd61-51c3-98be-00548ba783a3}");
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString FindEuclideanDistMap::getSubGroupName() const
{
  return SIMPL::FilterSubGroups::MorphologicalFilters;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString FindEuclideanDistMap::getHumanLabel() const
{
  return "Find Euclidean Distance Map";
}

// -----------------------------------------------------------------------------
FindEuclideanDistMap::Pointer FindEuclideanDistMap::NullPointer()
{
  return Pointer(static_cast<Self*>(nullptr));
}

// -----------------------------------------------------------------------------
std::shared_ptr<FindEuclideanDistMap> FindEuclideanDistMap::New()
{
  struct make_shared_enabler : public FindEuclideanDistMap
  {
  };
  std::shared_ptr<make_shared_enabler> val = std::make_shared<make_shared_enabler>();
  val->setupFilterParameters();
  return val;
}

// -----------------------------------------------------------------------------
QString FindEuclideanDistMap::getNameOfClass() const
{
  return QString("FindEuclideanDistMap");
}

// -----------------------------------------------------------------------------
QString FindEuclideanDistMap::ClassName()
{
  return QString("FindEuclideanDistMap");
}

// -----------------------------------------------------------------------------
void FindEuclideanDistMap::setFeatureIdsArrayPath(const DataArrayPath& value)
{
  m_FeatureIdsArrayPath = value;
}

// -----------------------------------------------------------------------------
DataArrayPath FindEuclideanDistMap::getFeatureIdsArrayPath() const
{
  return m_FeatureIdsArrayPath;
}

// -----------------------------------------------------------------------------
void FindEuclideanDistMap::setGBDistancesArrayName(const QString& value)
{
  m_GBDistancesArrayName = value;
}

// -----------------------------------------------------------------------------
QString FindEuclideanDistMap::getGBDistancesArrayName() const
{
  return m_GBDistancesArrayName;
}

// -----------------------------------------------------------------------------
void FindEuclideanDistMap::setTJDistancesArrayName(const QString& value)
{
  m_TJDistancesArrayName = value;
}

// -----------------------------------------------------------------------------
QString FindEuclideanDistMap::getTJDistancesArrayName() const
{
  return m_TJDistancesArrayName;
}

// -----------------------------------------------------------------------------
void FindEuclideanDistMap::setQPDistancesArrayName(const QString& value)
{
  m_QPDistancesArrayName = value;
}

// -----------------------------------------------------------------------------
QString FindEuclideanDistMap::getQPDistancesArrayName() const
{
  return m_QPDistancesArrayName;
}

// -----------------------------------------------------------------------------
void FindEuclideanDistMap::setNearestNeighborsArrayName(const QString& value)
{
  m_NearestNeighborsArrayName = value;
}

// -----------------------------------------------------------------------------
QString FindEuclideanDistMap::getNearestNeighborsArrayName() const
{
  return m_NearestNeighborsArrayName;
}

// -----------------------------------------------------------------------------
void FindEuclideanDistMap::setDoBoundaries(bool value)
{
  m_DoBoundaries = value;
}

// -----------------------------------------------------------------------------
bool FindEuclideanDistMap::getDoBoundaries() const
{
  return m_DoBoundaries;
}

// -----------------------------------------------------------------------------
void FindEuclideanDistMap::setDoTripleLines(bool value)
{
  m_DoTripleLines = value;
}

// -----------------------------------------------------------------------------
bool FindEuclideanDistMap::getDoTripleLines() const
{
  return m_DoTripleLines;
}

// -----------------------------------------------------------------------------
void FindEuclideanDistMap::setDoQuadPoints(bool value)
{
  m_DoQuadPoints = value;
}

// -----------------------------------------------------------------------------
bool FindEuclideanDistMap::getDoQuadPoints() const
{
  return m_DoQuadPoints;
}

// -----------------------------------------------------------------------------
void FindEuclideanDistMap::setSaveNearestNeighbors(bool value)
{
  m_SaveNearestNeighbors = value;
}

// -----------------------------------------------------------------------------
bool FindEuclideanDistMap::getSaveNearestNeighbors() const
{
  return m_SaveNearestNeighbors;
}

// -----------------------------------------------------------------------------
void FindEuclideanDistMap::setCalcManhattanDist(bool value)
{
  m_CalcManhattanDist = value;
}

// -----------------------------------------------------------------------------
bool FindEuclideanDistMap::getCalcManhattanDist() const
{
  return m_CalcManhattanDist;
}
