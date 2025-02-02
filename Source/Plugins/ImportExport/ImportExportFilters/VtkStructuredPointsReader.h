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

#include <QtCore/QObject>
#include <QtCore/QString>

#include "SIMPLib/SIMPLib.h"
#include "SIMPLib/DataContainers/AttributeMatrix.h"
#include "SIMPLib/DataContainers/DataArrayPath.h"
#include "SIMPLib/Filtering/AbstractFilter.h"

#include "ImportExport/ImportExportDLLExport.h"

/**
 * @brief The VtkStructuredPointsReader class. See [Filter documentation](@ref vtkstructuredpointsreader) for details.
 */
class ImportExport_EXPORT VtkStructuredPointsReader : public AbstractFilter
{
  Q_OBJECT

  // Start Python bindings declarations
  PYB11_BEGIN_BINDINGS(VtkStructuredPointsReader SUPERCLASS AbstractFilter)
  PYB11_FILTER()
  PYB11_SHARED_POINTERS(VtkStructuredPointsReader)
  PYB11_FILTER_NEW_MACRO(VtkStructuredPointsReader)
  PYB11_PROPERTY(bool ReadCellData READ getReadCellData WRITE setReadCellData)
  PYB11_PROPERTY(DataArrayPath VolumeDataContainerName READ getVolumeDataContainerName WRITE setVolumeDataContainerName)
  PYB11_PROPERTY(QString CellAttributeMatrixName READ getCellAttributeMatrixName WRITE setCellAttributeMatrixName)
  PYB11_PROPERTY(bool ReadPointData READ getReadPointData WRITE setReadPointData)
  PYB11_PROPERTY(DataArrayPath VertexDataContainerName READ getVertexDataContainerName WRITE setVertexDataContainerName)
  PYB11_PROPERTY(QString VertexAttributeMatrixName READ getVertexAttributeMatrixName WRITE setVertexAttributeMatrixName)
  PYB11_PROPERTY(QString InputFile READ getInputFile WRITE setInputFile)
  PYB11_END_BINDINGS()
  // End Python bindings declarations

public:
  using Self = VtkStructuredPointsReader;
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
   * @brief Returns the name of the class for VtkStructuredPointsReader
   */
  QString getNameOfClass() const override;
  /**
   * @brief Returns the name of the class for VtkStructuredPointsReader
   */
  static QString ClassName();

  ~VtkStructuredPointsReader() override;

  /**
   * @brief Setter property for ReadCellData
   */
  void setReadCellData(bool value);
  /**
   * @brief Getter property for ReadCellData
   * @return Value of ReadCellData
   */
  bool getReadCellData() const;
  Q_PROPERTY(bool ReadCellData READ getReadCellData WRITE setReadCellData)

  /**
   * @brief Setter property for VolumeDataContainerName
   */
  void setVolumeDataContainerName(const DataArrayPath& value);
  /**
   * @brief Getter property for VolumeDataContainerName
   * @return Value of VolumeDataContainerName
   */
  DataArrayPath getVolumeDataContainerName() const;
  Q_PROPERTY(DataArrayPath VolumeDataContainerName READ getVolumeDataContainerName WRITE setVolumeDataContainerName)

  /**
   * @brief Setter property for CellAttributeMatrixName
   */
  void setCellAttributeMatrixName(const QString& value);
  /**
   * @brief Getter property for CellAttributeMatrixName
   * @return Value of CellAttributeMatrixName
   */
  QString getCellAttributeMatrixName() const;
  Q_PROPERTY(QString CellAttributeMatrixName READ getCellAttributeMatrixName WRITE setCellAttributeMatrixName)

  /**
   * @brief Setter property for ReadPointData
   */
  void setReadPointData(bool value);
  /**
   * @brief Getter property for ReadPointData
   * @return Value of ReadPointData
   */
  bool getReadPointData() const;
  Q_PROPERTY(bool ReadPointData READ getReadPointData WRITE setReadPointData)

  /**
   * @brief Setter property for VertexDataContainerName
   */
  void setVertexDataContainerName(const DataArrayPath& value);
  /**
   * @brief Getter property for VertexDataContainerName
   * @return Value of VertexDataContainerName
   */
  DataArrayPath getVertexDataContainerName() const;
  Q_PROPERTY(DataArrayPath VertexDataContainerName READ getVertexDataContainerName WRITE setVertexDataContainerName)

  /**
   * @brief Setter property for VertexAttributeMatrixName
   */
  void setVertexAttributeMatrixName(const QString& value);
  /**
   * @brief Getter property for VertexAttributeMatrixName
   * @return Value of VertexAttributeMatrixName
   */
  QString getVertexAttributeMatrixName() const;
  Q_PROPERTY(QString VertexAttributeMatrixName READ getVertexAttributeMatrixName WRITE setVertexAttributeMatrixName)

  /**
   * @brief Setter property for InputFile
   */
  void setInputFile(const QString& value);
  /**
   * @brief Getter property for InputFile
   * @return Value of InputFile
   */
  QString getInputFile() const;
  Q_PROPERTY(QString InputFile READ getInputFile WRITE setInputFile)

  /**
   * @brief Setter property for Comment
   */
  void setComment(const QString& value);
  /**
   * @brief Getter property for Comment
   * @return Value of Comment
   */
  QString getComment() const;

  /**
   * @brief Setter property for DatasetType
   */
  void setDatasetType(const QString& value);
  /**
   * @brief Getter property for DatasetType
   * @return Value of DatasetType
   */
  QString getDatasetType() const;

  /**
   * @brief Setter property for FileIsBinary
   */
  void setFileIsBinary(bool value);
  /**
   * @brief Getter property for FileIsBinary
   * @return Value of FileIsBinary
   */
  bool getFileIsBinary() const;

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
  VtkStructuredPointsReader();
  /**
   * @brief dataCheck Checks for the appropriate parameter values and availability of arrays
   */
  void dataCheck() override;

  /**
   * @brief Initializes all the private instance variables.
   */
  void initialize();

  /**
   * @brief readHeader Reads the .vtk file header ; CURRENTLY NOT IMPLEMENTED
   * @return Integer error value
   */
  int32_t readHeader();

  /**
   * @brief readFile Handles the main reading of the .vtk file
   * @return Integer error value
   */
  virtual int32_t readFile();

  /**
   * @brief readData Reads a section of data from the .vtk file
   * @param instream Incoming file stream
   */
  virtual void readData(std::istream& instream);

  /**
   * @brief parseCoordinateLine Parses a line representing coordinates
   * @param input Incoming line to parse
   * @param value Coordinate value
   * @return Integer error value
   */
  int32_t parseCoordinateLine(const char* input, size_t& value);

  /**
   * @brief parseByteSize Parses the byte size from a data set declaration line
   * @param text Line to parse
   * @return Integer error value
   */
  size_t parseByteSize(QString text);

  /**
   * @brief readLine Reads a line from the .vtk file
   * @param in Incoming file stream
   * @param result Char pointer to store line
   * @param length Length of line
   * @return Integer error value
   */
  int32_t readLine(std::istream& in, char* result, size_t length);

  /**
   * @brief readString Reas a string from the .vtk file
   * @param in Incoming file stream
   * @param result Char pointer to store string
   * @param length Length of string
   * @return Integer error value
   */
  int32_t readString(std::istream& in, char* result, size_t length);

  /**
   * @brief lowerCase Converts a string to lower case
   * @param str Incoming string to convert
   * @param len Length of string
   * @return Integer error value
   */
  char* lowerCase(char* str, const size_t len);

  /**
   * @brief readDataTypeSection Determines the type of data to be read from the .vtk file
   * @param in Incoming file stream
   * @param numPts Number of points to read
   * @param nextKeyWord Keyword for data type
   * @return Integer error value
   */
  int32_t readDataTypeSection(std::istream& in, int numPts, const std::string& nextKeyWord);

  /**
   * @brief readScalarData Reads scalar data attribute types
   * @param in Incoming file stream
   * @return Integer error value
   */
  int32_t readScalarData(std::istream& in, int numPts);

  /**
   * @brief readVectorData Reads vector data attribute types
   * @param in Incoming file stream
   * @param numPts Number of points
   * @return Integer error value
   */
  int32_t readVectorData(std::istream& in, int numPts);

  /**
   * @brief DecodeString Decodes a binary string from the .vtk file
   * @param resname Resulting decoded string
   * @param name Binary string to decode
   * @return Integer error value
   */
  int32_t DecodeString(char* resname, const char* name);

private:
  bool m_ReadCellData = {true};
  DataArrayPath m_VolumeDataContainerName = {"ImageDataContainer_CellData", "", ""};
  QString m_CellAttributeMatrixName = {SIMPL::Defaults::CellAttributeMatrixName};
  bool m_ReadPointData = {true};
  DataArrayPath m_VertexDataContainerName = {"ImageDataContainer_PointData", "", ""};
  QString m_VertexAttributeMatrixName = {SIMPL::Defaults::CellAttributeMatrixName};
  QString m_InputFile = {""};
  QString m_Comment = {""};
  QString m_DatasetType = {""};
  bool m_FileIsBinary = {true};

  AttributeMatrix::Pointer m_CurrentAttrMat;

public:
  VtkStructuredPointsReader(const VtkStructuredPointsReader&) = delete;            // Copy Constructor Not Implemented
  VtkStructuredPointsReader(VtkStructuredPointsReader&&) = delete;                 // Move Constructor Not Implemented
  VtkStructuredPointsReader& operator=(const VtkStructuredPointsReader&) = delete; // Copy Assignment Not Implemented
  VtkStructuredPointsReader& operator=(VtkStructuredPointsReader&&) = delete;      // Move Assignment Not Implemented
};
