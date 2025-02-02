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
#include "SIMPLib/DataArrays/StatsDataArray.h"
#include "SIMPLib/DataArrays/StringDataArray.h"
#include "SIMPLib/Filtering/AbstractFilter.h"

#include "SyntheticBuilding/SyntheticBuildingDLLExport.h"

/**
 * @brief The EstablishMatrixPhase class. See [Filter documentation](@ref establishmatrixphase) for details.
 */
class SyntheticBuilding_EXPORT EstablishMatrixPhase : public AbstractFilter
{
  Q_OBJECT

  // Start Python bindings declarations
  PYB11_BEGIN_BINDINGS(EstablishMatrixPhase SUPERCLASS AbstractFilter)
  PYB11_FILTER()
  PYB11_SHARED_POINTERS(EstablishMatrixPhase)
  PYB11_FILTER_NEW_MACRO(EstablishMatrixPhase)
  PYB11_PROPERTY(DataArrayPath OutputCellAttributeMatrixPath READ getOutputCellAttributeMatrixPath WRITE setOutputCellAttributeMatrixPath)
  PYB11_PROPERTY(QString FeatureIdsArrayName READ getFeatureIdsArrayName WRITE setFeatureIdsArrayName)
  PYB11_PROPERTY(QString CellPhasesArrayName READ getCellPhasesArrayName WRITE setCellPhasesArrayName)
  PYB11_PROPERTY(QString FeaturePhasesArrayName READ getFeaturePhasesArrayName WRITE setFeaturePhasesArrayName)
  PYB11_PROPERTY(QString NumFeaturesArrayName READ getNumFeaturesArrayName WRITE setNumFeaturesArrayName)
  PYB11_PROPERTY(DataArrayPath MaskArrayPath READ getMaskArrayPath WRITE setMaskArrayPath)
  PYB11_PROPERTY(bool UseMask READ getUseMask WRITE setUseMask)
  PYB11_PROPERTY(DataArrayPath InputStatsArrayPath READ getInputStatsArrayPath WRITE setInputStatsArrayPath)
  PYB11_PROPERTY(DataArrayPath InputPhaseTypesArrayPath READ getInputPhaseTypesArrayPath WRITE setInputPhaseTypesArrayPath)
  PYB11_PROPERTY(DataArrayPath InputPhaseNamesArrayPath READ getInputPhaseNamesArrayPath WRITE setInputPhaseNamesArrayPath)
  PYB11_END_BINDINGS()
  // End Python bindings declarations

public:
  using Self = EstablishMatrixPhase;
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
   * @brief Returns the name of the class for EstablishMatrixPhase
   */
  QString getNameOfClass() const override;
  /**
   * @brief Returns the name of the class for EstablishMatrixPhase
   */
  static QString ClassName();

  ~EstablishMatrixPhase() override;

  /**
   * @brief Setter property for OutputCellAttributeMatrixPath
   */
  void setOutputCellAttributeMatrixPath(const DataArrayPath& value);
  /**
   * @brief Getter property for OutputCellAttributeMatrixPath
   * @return Value of OutputCellAttributeMatrixPath
   */
  DataArrayPath getOutputCellAttributeMatrixPath() const;
  Q_PROPERTY(DataArrayPath OutputCellAttributeMatrixPath READ getOutputCellAttributeMatrixPath WRITE setOutputCellAttributeMatrixPath)

  /**
   * @brief Setter property for OutputCellFeatureAttributeMatrixName
   */
  void setOutputCellFeatureAttributeMatrixName(const QString& value);
  /**
   * @brief Getter property for OutputCellFeatureAttributeMatrixName
   * @return Value of OutputCellFeatureAttributeMatrixName
   */
  QString getOutputCellFeatureAttributeMatrixName() const;
  Q_PROPERTY(QString OutputCellFeatureAttributeMatrixName READ getOutputCellFeatureAttributeMatrixName WRITE setOutputCellFeatureAttributeMatrixName)

  /**
   * @brief Setter property for OutputCellEnsembleAttributeMatrixName
   */
  void setOutputCellEnsembleAttributeMatrixName(const QString& value);
  /**
   * @brief Getter property for OutputCellEnsembleAttributeMatrixName
   * @return Value of OutputCellEnsembleAttributeMatrixName
   */
  QString getOutputCellEnsembleAttributeMatrixName() const;
  Q_PROPERTY(QString OutputCellEnsembleAttributeMatrixName READ getOutputCellEnsembleAttributeMatrixName WRITE setOutputCellEnsembleAttributeMatrixName)

  /**
   * @brief Setter property for FeatureIdsArrayName
   */
  void setFeatureIdsArrayName(const QString& value);
  /**
   * @brief Getter property for FeatureIdsArrayName
   * @return Value of FeatureIdsArrayName
   */
  QString getFeatureIdsArrayName() const;
  Q_PROPERTY(QString FeatureIdsArrayName READ getFeatureIdsArrayName WRITE setFeatureIdsArrayName)

  /**
   * @brief Setter property for CellPhasesArrayName
   */
  void setCellPhasesArrayName(const QString& value);
  /**
   * @brief Getter property for CellPhasesArrayName
   * @return Value of CellPhasesArrayName
   */
  QString getCellPhasesArrayName() const;
  Q_PROPERTY(QString CellPhasesArrayName READ getCellPhasesArrayName WRITE setCellPhasesArrayName)

  /**
   * @brief Setter property for FeaturePhasesArrayName
   */
  void setFeaturePhasesArrayName(const QString& value);
  /**
   * @brief Getter property for FeaturePhasesArrayName
   * @return Value of FeaturePhasesArrayName
   */
  QString getFeaturePhasesArrayName() const;
  Q_PROPERTY(QString FeaturePhasesArrayName READ getFeaturePhasesArrayName WRITE setFeaturePhasesArrayName)

  /**
   * @brief Setter property for NumFeaturesArrayName
   */
  void setNumFeaturesArrayName(const QString& value);
  /**
   * @brief Getter property for NumFeaturesArrayName
   * @return Value of NumFeaturesArrayName
   */
  QString getNumFeaturesArrayName() const;
  Q_PROPERTY(QString NumFeaturesArrayName READ getNumFeaturesArrayName WRITE setNumFeaturesArrayName)

  /**
   * @brief Setter property for MaskArrayPath
   */
  void setMaskArrayPath(const DataArrayPath& value);
  /**
   * @brief Getter property for MaskArrayPath
   * @return Value of MaskArrayPath
   */
  DataArrayPath getMaskArrayPath() const;
  Q_PROPERTY(DataArrayPath MaskArrayPath READ getMaskArrayPath WRITE setMaskArrayPath)

  /**
   * @brief Setter property for UseMask
   */
  void setUseMask(bool value);
  /**
   * @brief Getter property for UseMask
   * @return Value of UseMask
   */
  bool getUseMask() const;
  Q_PROPERTY(bool UseMask READ getUseMask WRITE setUseMask)

  /**
   * @brief Setter property for InputStatsArrayPath
   */
  void setInputStatsArrayPath(const DataArrayPath& value);
  /**
   * @brief Getter property for InputStatsArrayPath
   * @return Value of InputStatsArrayPath
   */
  DataArrayPath getInputStatsArrayPath() const;
  Q_PROPERTY(DataArrayPath InputStatsArrayPath READ getInputStatsArrayPath WRITE setInputStatsArrayPath)

  /**
   * @brief Setter property for InputPhaseTypesArrayPath
   */
  void setInputPhaseTypesArrayPath(const DataArrayPath& value);
  /**
   * @brief Getter property for InputPhaseTypesArrayPath
   * @return Value of InputPhaseTypesArrayPath
   */
  DataArrayPath getInputPhaseTypesArrayPath() const;
  Q_PROPERTY(DataArrayPath InputPhaseTypesArrayPath READ getInputPhaseTypesArrayPath WRITE setInputPhaseTypesArrayPath)

  /**
   * @brief Setter property for InputPhaseNamesArrayPath
   */
  void setInputPhaseNamesArrayPath(const DataArrayPath& value);
  /**
   * @brief Getter property for InputPhaseNamesArrayPath
   * @return Value of InputPhaseNamesArrayPath
   */
  DataArrayPath getInputPhaseNamesArrayPath() const;
  Q_PROPERTY(DataArrayPath InputPhaseNamesArrayPath READ getInputPhaseNamesArrayPath WRITE setInputPhaseNamesArrayPath)

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
  EstablishMatrixPhase();
  /**
   * @brief dataCheck Checks for the appropriate parameter values and availability of arrays
   */
  void dataCheck() override;

  /**
   * @brief Initializes all the private instance variables.
   */
  void initialize();

  /**
   * @brief establish_matrix Fills a synthetic volume with the correct volume fraction
   * of matrix phase(s) based on the input statistics
   */
  void establish_matrix();

  std::vector<int32_t> matrixphases;
  std::vector<float> matrixphasefractions;

private:
  std::weak_ptr<DataArray<int32_t>> m_FeatureIdsPtr;
  int32_t* m_FeatureIds = nullptr;
  std::weak_ptr<DataArray<int32_t>> m_CellPhasesPtr;
  int32_t* m_CellPhases = nullptr;
  std::weak_ptr<DataArray<bool>> m_MaskPtr;
  bool* m_Mask = nullptr;
  std::weak_ptr<DataArray<int32_t>> m_FeaturePhasesPtr;
  int32_t* m_FeaturePhases = nullptr;
  std::weak_ptr<DataArray<int32_t>> m_NumFeaturesPtr;
  int32_t* m_NumFeatures = nullptr;
  std::weak_ptr<DataArray<PhaseType::EnumType>> m_PhaseTypesPtr;
  PhaseType::EnumType* m_PhaseTypes = nullptr;

  DataArrayPath m_OutputCellAttributeMatrixPath = {SIMPL::Defaults::SyntheticVolumeDataContainerName, SIMPL::Defaults::CellAttributeMatrixName, ""};
  QString m_OutputCellFeatureAttributeMatrixName = {SIMPL::Defaults::CellFeatureAttributeMatrixName};
  QString m_OutputCellEnsembleAttributeMatrixName = {SIMPL::Defaults::CellEnsembleAttributeMatrixName};
  QString m_FeatureIdsArrayName = {SIMPL::CellData::FeatureIds};
  QString m_CellPhasesArrayName = {SIMPL::CellData::Phases};
  QString m_FeaturePhasesArrayName = {SIMPL::FeatureData::Phases};
  QString m_NumFeaturesArrayName = {SIMPL::EnsembleData::NumFeatures};
  DataArrayPath m_MaskArrayPath = {SIMPL::Defaults::ImageDataContainerName, SIMPL::Defaults::CellAttributeMatrixName, SIMPL::CellData::Mask};
  bool m_UseMask = {false};
  DataArrayPath m_InputStatsArrayPath = {SIMPL::Defaults::StatsGenerator, SIMPL::Defaults::CellEnsembleAttributeMatrixName, SIMPL::EnsembleData::Statistics};
  DataArrayPath m_InputPhaseTypesArrayPath = {SIMPL::Defaults::StatsGenerator, SIMPL::Defaults::CellEnsembleAttributeMatrixName, SIMPL::EnsembleData::PhaseTypes};
  DataArrayPath m_InputPhaseNamesArrayPath = {SIMPL::Defaults::StatsGenerator, SIMPL::Defaults::CellEnsembleAttributeMatrixName, SIMPL::EnsembleData::PhaseName};

  size_t firstMatrixFeature;
  float sizex;
  float sizey;
  float sizez;
  float totalvol;

  StringDataArray::WeakPointer m_PhaseNamesPtr;
  StatsDataArray::WeakPointer m_StatsDataArray;

  /**
   * @brief updateFeatureInstancePointers Resets the raw pointers that belong to a
   * Feature Attribute Matrix
   */
  void updateFeatureInstancePointers();

public:
  EstablishMatrixPhase(const EstablishMatrixPhase&) = delete;            // Copy Constructor Not Implemented
  EstablishMatrixPhase(EstablishMatrixPhase&&) = delete;                 // Move Constructor Not Implemented
  EstablishMatrixPhase& operator=(const EstablishMatrixPhase&) = delete; // Copy Assignment Not Implemented
  EstablishMatrixPhase& operator=(EstablishMatrixPhase&&) = delete;      // Move Assignment Not Implemented
};
