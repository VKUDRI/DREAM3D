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

#pragma once

#include <memory>

#include "SIMPLib/SIMPLib.h"
#include "SIMPLib/DataArrays/DataArray.hpp"
#include "SIMPLib/Filtering/AbstractFilter.h"

#include "SyntheticBuilding/SyntheticBuildingDLLExport.h"

/**
 * @brief The AddBadData class. See [Filter documentation](@ref addbaddata) for details.
 */
class SyntheticBuilding_EXPORT AddBadData : public AbstractFilter
{
  Q_OBJECT

  // Start Python bindings declarations
  PYB11_BEGIN_BINDINGS(AddBadData SUPERCLASS AbstractFilter)
  PYB11_FILTER()
  PYB11_SHARED_POINTERS(AddBadData)
  PYB11_FILTER_NEW_MACRO(AddBadData)
  PYB11_PROPERTY(DataArrayPath GBEuclideanDistancesArrayPath READ getGBEuclideanDistancesArrayPath WRITE setGBEuclideanDistancesArrayPath)
  PYB11_PROPERTY(bool PoissonNoise READ getPoissonNoise WRITE setPoissonNoise)
  PYB11_PROPERTY(float PoissonVolFraction READ getPoissonVolFraction WRITE setPoissonVolFraction)
  PYB11_PROPERTY(bool BoundaryNoise READ getBoundaryNoise WRITE setBoundaryNoise)
  PYB11_PROPERTY(float BoundaryVolFraction READ getBoundaryVolFraction WRITE setBoundaryVolFraction)
  PYB11_END_BINDINGS()
  // End Python bindings declarations

public:
  using Self = AddBadData;
  using Pointer = std::shared_ptr<Self>;
  using ConstPointer = std::shared_ptr<const Self>;
  using WeakPointer = std::weak_ptr<Self>;
  using ConstWeakPointer = std::weak_ptr<const Self>;

  /**
   * @brief Returns a NullPointer wrapped by a shared_ptr<>
   * @return
   */
  static Pointer NullPointer();

  /**
   * @brief Creates a new object wrapped in a shared_ptr<>
   * @return
   */
  static Pointer New();

  /**
   * @brief Returns the name of the class for AddBadData
   */
  QString getNameOfClass() const override;
  /**
   * @brief Returns the name of the class for AddBadData
   */
  static QString ClassName();

  ~AddBadData() override;

  /**
   * @brief Setter property for GBEuclideanDistancesArrayPath
   */
  void setGBEuclideanDistancesArrayPath(const DataArrayPath& value);
  /**
   * @brief Getter property for GBEuclideanDistancesArrayPath
   * @return Value of GBEuclideanDistancesArrayPath
   */
  DataArrayPath getGBEuclideanDistancesArrayPath() const;
  Q_PROPERTY(DataArrayPath GBEuclideanDistancesArrayPath READ getGBEuclideanDistancesArrayPath WRITE setGBEuclideanDistancesArrayPath)

  /**
   * @brief Setter property for PoissonNoise
   */
  void setPoissonNoise(bool value);
  /**
   * @brief Getter property for PoissonNoise
   * @return Value of PoissonNoise
   */
  bool getPoissonNoise() const;
  Q_PROPERTY(bool PoissonNoise READ getPoissonNoise WRITE setPoissonNoise)

  /**
   * @brief Setter property for PoissonVolFraction
   */
  void setPoissonVolFraction(float value);
  /**
   * @brief Getter property for PoissonVolFraction
   * @return Value of PoissonVolFraction
   */
  float getPoissonVolFraction() const;
  Q_PROPERTY(float PoissonVolFraction READ getPoissonVolFraction WRITE setPoissonVolFraction)

  /**
   * @brief Setter property for BoundaryNoise
   */
  void setBoundaryNoise(bool value);
  /**
   * @brief Getter property for BoundaryNoise
   * @return Value of BoundaryNoise
   */
  bool getBoundaryNoise() const;
  Q_PROPERTY(bool BoundaryNoise READ getBoundaryNoise WRITE setBoundaryNoise)

  /**
   * @brief Setter property for BoundaryVolFraction
   */
  void setBoundaryVolFraction(float value);
  /**
   * @brief Getter property for BoundaryVolFraction
   * @return Value of BoundaryVolFraction
   */
  float getBoundaryVolFraction() const;
  Q_PROPERTY(float BoundaryVolFraction READ getBoundaryVolFraction WRITE setBoundaryVolFraction)

  /**
   * @brief getCompiledLibraryName Reimplemented from @see AbstractFilter class
   */
  QString getCompiledLibraryName() const override;

  /**
   * @brief getBrandingString Returns the branding string for the filter, which is a tag
   * used to denote the filter's association with specific plugins
   * @return Branding string
   */
  QString getBrandingString() const override;

  /**
   * @brief getFilterVersion Returns a version string for this filter. Default
   * value is an empty string.
   * @return
   */
  QString getFilterVersion() const override;

  /**
   * @brief newFilterInstance Reimplemented from @see AbstractFilter class
   */
  AbstractFilter::Pointer newFilterInstance(bool copyFilterParameters) const override;

  /**
   * @brief getGroupName Reimplemented from @see AbstractFilter class
   */
  QString getGroupName() const override;

  /**
   * @brief getSubGroupName Reimplemented from @see AbstractFilter class
   */
  QString getSubGroupName() const override;

  /**
   * @brief getUuid Return the unique identifier for this filter.
   * @return A QUuid object.
   */
  QUuid getUuid() const override;

  /**
   * @brief getHumanLabel Reimplemented from @see AbstractFilter class
   */
  QString getHumanLabel() const override;

  /**
   * @brief setupFilterParameters Reimplemented from @see AbstractFilter class
   */
  void setupFilterParameters() override;

  /**
   * @brief readFilterParameters Reimplemented from @see AbstractFilter class
   */
  void readFilterParameters(AbstractFilterParametersReader* reader, int index) override;

  /**
   * @brief execute Reimplemented from @see AbstractFilter class
   */
  void execute() override;

protected:
  AddBadData();
  /**
   * @brief dataCheck Checks for the appropriate parameter values and availability of arrays
   */
  void dataCheck() override;

  /**
   * @brief Initializes all the private instance variables.
   */
  void initialize();

  /**
   * @brief add_noise Adds "noise" to all arrays that belong to a particular Attribute Matrix.
   * "Noise" in this sense means a tuple value of 0.
   */
  void add_noise();

private:
  std::weak_ptr<DataArray<int32_t>> m_GBEuclideanDistancesPtr;
  int32_t* m_GBEuclideanDistances = nullptr;

  DataArrayPath m_GBEuclideanDistancesArrayPath = {SIMPL::Defaults::ImageDataContainerName, SIMPL::Defaults::ElementAttributeMatrixName, SIMPL::CellData::GBEuclideanDistances};
  bool m_PoissonNoise = {true};
  float m_PoissonVolFraction = {0.0f};
  bool m_BoundaryNoise = {false};
  float m_BoundaryVolFraction = {0.0f};

public:
  AddBadData(const AddBadData&) = delete;            // Copy Constructor Not Implemented
  AddBadData(AddBadData&&) = delete;                 // Move Constructor Not Implemented
  AddBadData& operator=(const AddBadData&) = delete; // Copy Assignment Not Implemented
  AddBadData& operator=(AddBadData&&) = delete;      // Move Assignment Not Implemented
};
