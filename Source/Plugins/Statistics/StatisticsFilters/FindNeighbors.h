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
* The code contained herein was partially funded by the followig contracts:
*    United States Air Force Prime Contract FA8650-07-D-5800
*    United States Air Force Prime Contract FA8650-10-D-5210
*    United States Prime Contract Navy N00173-07-C-2068
*
* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

#ifndef _findneighbors_h_
#define _findneighbors_h_

#include "SIMPLib/Common/SIMPLibSetGetMacros.h"
#include "SIMPLib/DataArrays/NeighborList.hpp"
#include "SIMPLib/Filtering/AbstractFilter.h"
#include "SIMPLib/SIMPLib.h"

/**
 * @brief The FindNeighbors class. See [Filter documentation](@ref findneighbors) for details.
 */
class FindNeighbors : public AbstractFilter
{
  Q_OBJECT
public:
  SIMPL_SHARED_POINTERS(FindNeighbors)
  SIMPL_STATIC_NEW_MACRO(FindNeighbors)
   SIMPL_TYPE_MACRO_SUPER_OVERRIDE(FindNeighbors, AbstractFilter)

  virtual ~FindNeighbors();

  SIMPL_FILTER_PARAMETER(DataArrayPath, CellFeatureAttributeMatrixPath)
  Q_PROPERTY(DataArrayPath CellFeatureAttributeMatrixPath READ getCellFeatureAttributeMatrixPath WRITE setCellFeatureAttributeMatrixPath)

  SIMPL_FILTER_PARAMETER(QString, SharedSurfaceAreaListArrayName)
  Q_PROPERTY(QString SharedSurfaceAreaListArrayName READ getSharedSurfaceAreaListArrayName WRITE setSharedSurfaceAreaListArrayName)

  SIMPL_FILTER_PARAMETER(QString, NeighborListArrayName)
  Q_PROPERTY(QString NeighborListArrayName READ getNeighborListArrayName WRITE setNeighborListArrayName)

  SIMPL_FILTER_PARAMETER(DataArrayPath, FeatureIdsArrayPath)
  Q_PROPERTY(DataArrayPath FeatureIdsArrayPath READ getFeatureIdsArrayPath WRITE setFeatureIdsArrayPath)

  SIMPL_FILTER_PARAMETER(QString, BoundaryCellsArrayName)
  Q_PROPERTY(QString BoundaryCellsArrayName READ getBoundaryCellsArrayName WRITE setBoundaryCellsArrayName)

  SIMPL_FILTER_PARAMETER(QString, NumNeighborsArrayName)
  Q_PROPERTY(QString NumNeighborsArrayName READ getNumNeighborsArrayName WRITE setNumNeighborsArrayName)

  SIMPL_FILTER_PARAMETER(QString, SurfaceFeaturesArrayName)
  Q_PROPERTY(QString SurfaceFeaturesArrayName READ getSurfaceFeaturesArrayName WRITE setSurfaceFeaturesArrayName)

  SIMPL_FILTER_PARAMETER(bool, StoreBoundaryCells)
  Q_PROPERTY(bool StoreBoundaryCells READ getStoreBoundaryCells WRITE setStoreBoundaryCells)

  SIMPL_FILTER_PARAMETER(bool, StoreSurfaceFeatures)
  Q_PROPERTY(bool StoreSurfaceFeatures READ getStoreSurfaceFeatures WRITE setStoreSurfaceFeatures)

  /**
   * @brief getCompiledLibraryName Reimplemented from @see AbstractFilter class
   */
  virtual const QString getCompiledLibraryName() override;

  /**
   * @brief getBrandingString Returns the branding string for the filter, which is a tag
   * used to denote the filter's association with specific plugins
   * @return Branding string
  */
  virtual const QString getBrandingString() override;

  /**
   * @brief getFilterVersion Returns a version string for this filter. Default
   * value is an empty string.
   * @return
   */
  virtual const QString getFilterVersion() override;

  /**
   * @brief newFilterInstance Reimplemented from @see AbstractFilter class
   */
  virtual AbstractFilter::Pointer newFilterInstance(bool copyFilterParameters) override;

  /**
   * @brief getGroupName Reimplemented from @see AbstractFilter class
   */
  virtual const QString getGroupName() override;

  /**
   * @brief getSubGroupName Reimplemented from @see AbstractFilter class
   */
  virtual const QString getSubGroupName() override;

  /**
   * @brief getUuid Return the unique identifier for this filter.
   * @return A QUuid object.
   */
  virtual const QUuid getUuid() override;

  /**
   * @brief getHumanLabel Reimplemented from @see AbstractFilter class
   */
  virtual const QString getHumanLabel() override;

  /**
   * @brief setupFilterParameters Reimplemented from @see AbstractFilter class
   */
  virtual void setupFilterParameters() override;

  /**
   * @brief readFilterParameters Reimplemented from @see AbstractFilter class
   */
  virtual void readFilterParameters(AbstractFilterParametersReader* reader, int index);

  /**
   * @brief execute Reimplemented from @see AbstractFilter class
   */
  virtual void execute() override;

  /**
  * @brief preflight Reimplemented from @see AbstractFilter class
  */
  virtual void preflight() override;

signals:
  /**
   * @brief updateFilterParameters Emitted when the Filter requests all the latest Filter parameters
   * be pushed from a user-facing control (such as a widget)
   * @param filter Filter instance pointer
   */
  void updateFilterParameters(AbstractFilter* filter);

  /**
   * @brief parametersChanged Emitted when any Filter parameter is changed internally
   */
  void parametersChanged();

  /**
   * @brief preflightAboutToExecute Emitted just before calling dataCheck()
   */
  void preflightAboutToExecute();

  /**
   * @brief preflightExecuted Emitted just after calling dataCheck()
   */
  void preflightExecuted();

protected:
  FindNeighbors();
  /**
   * @brief dataCheck Checks for the appropriate parameter values and availability of arrays
   */
  void dataCheck();

  /**
   * @brief Initializes all the private instance variables.
   */
  void initialize();

private:
  DEFINE_DATAARRAY_VARIABLE(int32_t, FeatureIds)

  DEFINE_DATAARRAY_VARIABLE(int8_t, BoundaryCells)
  DEFINE_DATAARRAY_VARIABLE(bool, SurfaceFeatures)
  DEFINE_DATAARRAY_VARIABLE(int32_t, NumNeighbors)

  NeighborList<int32_t>::WeakPointer m_NeighborList;
  NeighborList<float>::WeakPointer m_SharedSurfaceAreaList;

  FindNeighbors(const FindNeighbors&);  // Copy Constructor Not Implemented
  void operator=(const FindNeighbors&) = delete; // Operator '=' Not Implemented
};

#endif /* FINDNEIGHBORS_H_ */
