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
#include "WriteStlFile.h"

#include <QtCore/QTextStream>

#include "SIMPLib/Common/Constants.h"
#include "SIMPLib/DataContainers/DataContainerArray.h"
#include "SIMPLib/FilterParameters/AbstractFilterParametersReader.h"
#include "SIMPLib/FilterParameters/DataArraySelectionFilterParameter.h"
#include "SIMPLib/FilterParameters/LinkedBooleanFilterParameter.h"
#include "SIMPLib/FilterParameters/OutputPathFilterParameter.h"
#include "SIMPLib/FilterParameters/SeparatorFilterParameter.h"
#include "SIMPLib/FilterParameters/StringFilterParameter.h"
#include "SIMPLib/Geometry/TriangleGeom.h"
#include "SIMPLib/Math/SIMPLibMath.h"

#include <QtCore/QDir>

#include "ImportExport/ImportExportConstants.h"
#include "ImportExport/ImportExportVersion.h"

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
WriteStlFile::WriteStlFile() = default;

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
WriteStlFile::~WriteStlFile() = default;

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void WriteStlFile::setupFilterParameters()
{
  FilterParameterVectorType parameters;
  parameters.push_back(SIMPL_NEW_OUTPUT_PATH_FP("Output STL Directory", OutputStlDirectory, FilterParameter::Category::Parameter, WriteStlFile));
  parameters.push_back(SIMPL_NEW_STRING_FP("STL File Prefix", OutputStlPrefix, FilterParameter::Category::Parameter, WriteStlFile));
  // std::vector<QString> linkedProps = {"SurfaceMeshFacePhasesArrayPath"};
  // parameters.push_back(SIMPL_NEW_LINKED_BOOL_FP("Group Files by Ensemble", GroupByPhase, FilterParameter::Category::Parameter, WriteStlFile, linkedProps));
  parameters.push_back(SeparatorFilterParameter::Create("Face Data", FilterParameter::Category::RequiredArray));
  {
    DataArraySelectionFilterParameter::RequirementType req = DataArraySelectionFilterParameter::CreateRequirement(SIMPL::TypeNames::Int32, 2, AttributeMatrix::Type::Face, IGeometry::Type::Triangle);
    parameters.push_back(SIMPL_NEW_DA_SELECTION_FP("Face Labels", SurfaceMeshFaceLabelsArrayPath, FilterParameter::Category::RequiredArray, WriteStlFile, req));
  }
  // parameters.push_back(DataArraySelectionFilterParameter::Create("Face Phases", "SurfaceMeshFacePhasesArrayPath", getSurfaceMeshFacePhasesArrayPath(), FilterParameter::Category::RequiredArray,
  // SIMPL_BIND_SETTER(WriteStlFile, this, SurfaceMeshFacePhasesArrayPath), SIMPL_BIND_GETTER(WriteStlFile, this, SurfaceMeshFacePhasesArrayPath)));
  setFilterParameters(parameters);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void WriteStlFile::readFilterParameters(AbstractFilterParametersReader* reader, int index)
{
  reader->openFilterGroup(this, index);
  setSurfaceMeshFacePhasesArrayPath(reader->readDataArrayPath("SurfaceMeshFacePhasesArrayPath", getSurfaceMeshFacePhasesArrayPath()));
  setSurfaceMeshFaceLabelsArrayPath(reader->readDataArrayPath("SurfaceMeshFaceLabelsArrayPath", getSurfaceMeshFaceLabelsArrayPath()));
  setOutputStlDirectory(reader->readString("OutputStlDirectory", getOutputStlDirectory()));
  setOutputStlPrefix(reader->readString("OutputStlPrefix", getOutputStlPrefix()));
  reader->closeFilterGroup();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void WriteStlFile::readFilterParameters(QJsonObject& obj)
{
  AbstractFilter::readFilterParameters(obj);

  QJsonObject dataPathObj = obj["SurfaceMeshFacePhasesArrayPath"].toObject();
  DataArrayPath dap;
  dap.readJson(dataPathObj);
  setSurfaceMeshFacePhasesArrayPath(dap);
}

// FP: Check why these values are not connected to a filter parameter!

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void WriteStlFile::writeFilterParameters(QJsonObject& obj) const
{
  AbstractFilter::writeFilterParameters(obj);

  DataArrayPath dap = getSurfaceMeshFacePhasesArrayPath();
  QJsonObject dataPathObj;
  dap.writeJson(dataPathObj);
  obj["SurfaceMeshFacePhasesArrayPath"] = dataPathObj;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void WriteStlFile::initialize()
{
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void WriteStlFile::dataCheck()
{
  clearErrorCode();
  clearWarningCode();

  TriangleGeom::Pointer triangles = getDataContainerArray()->getPrereqGeometryFromDataContainer<TriangleGeom>(this, getSurfaceMeshFaceLabelsArrayPath().getDataContainerName());

  QVector<IDataArray::Pointer> dataArrays;

  if(getErrorCode() >= 0)
  {
    dataArrays.push_back(triangles->getTriangles());
  }

  if(m_OutputStlDirectory.isEmpty())
  {
    setErrorCondition(-1003, "The output directory must be set");
  }

  std::vector<size_t> cDims(1, 2);
  m_SurfaceMeshFaceLabelsPtr = getDataContainerArray()->getPrereqArrayFromPath<DataArray<int32_t>>(this, getSurfaceMeshFaceLabelsArrayPath(), cDims);
  if(nullptr != m_SurfaceMeshFaceLabelsPtr.lock())
  {
    m_SurfaceMeshFaceLabels = m_SurfaceMeshFaceLabelsPtr.lock()->getPointer(0);
  } /* Now assign the raw pointer to data from the DataArray<T> object */
  if(getErrorCode() >= 0)
  {
    dataArrays.push_back(m_SurfaceMeshFaceLabelsPtr.lock());
  }

  if(m_GroupByPhase)
  {
    m_SurfaceMeshFacePhasesPtr = getDataContainerArray()->getPrereqArrayFromPath<DataArray<int32_t>>(this, getSurfaceMeshFacePhasesArrayPath(), cDims);
    if(nullptr != m_SurfaceMeshFacePhasesPtr.lock())
    {
      m_SurfaceMeshFacePhases = m_SurfaceMeshFacePhasesPtr.lock()->getPointer(0);
    } /* Now assign the raw pointer to data from the DataArray<T> object */
    if(getErrorCode() >= 0)
    {
      dataArrays.push_back(m_SurfaceMeshFacePhasesPtr.lock());
    }
  }

  getDataContainerArray()->validateNumberOfTuples(this, dataArrays);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void WriteStlFile::execute()
{
  int32_t err = 0;
  clearErrorCode();
  clearWarningCode();
  dataCheck();
  if(getErrorCode() < 0)
  {
    return;
  }

  // Make sure any directory path is also available as the user may have just typed
  // in a path without actually creating the full path
  QDir stlDir(getOutputStlDirectory());
  if(!stlDir.mkpath("."))
  {
    QString ss = QObject::tr("Error creating parent path '%1'").arg(getOutputStlDirectory());
    setErrorCondition(-1, ss);
    return;
  }

  TriangleGeom::Pointer triangleGeom = getDataContainerArray()->getDataContainer(getSurfaceMeshFaceLabelsArrayPath().getDataContainerName())->getGeometryAs<TriangleGeom>();
  float* nodes = triangleGeom->getVertexPointer(0);
  MeshIndexType* triangles = triangleGeom->getTriPointer(0);
  MeshIndexType nTriangles = triangleGeom->getNumberOfTris();

  if(nTriangles > std::numeric_limits<int32_t>::max())
  {
    QString ss = QObject::tr("The number of triangles is %1, but the STL specification only supports triangle counts up to %2").arg(nTriangles).arg(std::numeric_limits<int32_t>::max());
    setErrorCondition(-1, ss);
    return;
  }

  // Store all the unique Spins
  QMap<int32_t, int32_t> uniqueGrainIdtoPhase;
  if(m_GroupByPhase)
  {
    for(MeshIndexType i = 0; i < nTriangles; i++)
    {
      uniqueGrainIdtoPhase.insert(m_SurfaceMeshFaceLabels[i * 2], m_SurfaceMeshFacePhases[i * 2]);
      uniqueGrainIdtoPhase.insert(m_SurfaceMeshFaceLabels[i * 2 + 1], m_SurfaceMeshFacePhases[i * 2 + 1]);
    }
  }
  else
  {
    for(MeshIndexType i = 0; i < nTriangles; i++)
    {
      uniqueGrainIdtoPhase.insert(m_SurfaceMeshFaceLabels[i * 2], 0);
      uniqueGrainIdtoPhase.insert(m_SurfaceMeshFaceLabels[i * 2 + 1], 0);
    }
  }

  unsigned char data[50];
  float* normal = reinterpret_cast<float*>(data);
  float* vert1 = reinterpret_cast<float*>(data + 12);
  float* vert2 = reinterpret_cast<float*>(data + 24);
  float* vert3 = reinterpret_cast<float*>(data + 36);
  uint16_t* attrByteCount = reinterpret_cast<uint16_t*>(data + 48);
  *attrByteCount = 0;

  size_t totalWritten = 0;
  float u[3] = {0.0f, 0.0f, 0.0f}, w[3] = {0.0f, 0.0f, 0.0f};
  float length = 0.0f;

  int32_t spin = 0;
  int32_t triCount = 0;

  // Loop over the unique Spins
  for(QMap<int32_t, int32_t>::iterator spinIter = uniqueGrainIdtoPhase.begin(); spinIter != uniqueGrainIdtoPhase.end(); ++spinIter)
  {
    spin = spinIter.key();

    // Generate the output file name
    QString filename = getOutputStlDirectory() + "/" + getOutputStlPrefix();
    if(m_GroupByPhase)
    {
      filename = filename + QString("Ensemble_") + QString::number(spinIter.value()) + QString("_");
    }
    filename = filename + QString("Feature_") + QString::number(spin) + ".stl";
    FILE* f = fopen(filename.toLatin1().data(), "wb");

    QString ss = QObject::tr("Writing STL for Feature Id %1").arg(spin);
    notifyStatusMessage(ss);

    QString header = "DREAM3D Generated For Feature ID " + QString::number(spin);
    if(m_GroupByPhase)
    {
      header = header + " Phase " + QString::number(spinIter.value());
    }
    err = writeHeader(f, header, 0);
    if(err < 0)
    {
    }
    triCount = 0; // Reset this to Zero. Increment for every triangle written

    // Loop over all the triangles for this spin
    for(MeshIndexType t = 0; t < nTriangles; ++t)
    {
      // Get the true indices of the 3 nodes
      MeshIndexType nId0 = triangles[t * 3];
      MeshIndexType nId1 = triangles[t * 3 + 1];
      MeshIndexType nId2 = triangles[t * 3 + 2];

      vert1[0] = static_cast<float>(nodes[nId0 * 3]);
      vert1[1] = static_cast<float>(nodes[nId0 * 3 + 1]);
      vert1[2] = static_cast<float>(nodes[nId0 * 3 + 2]);

      if(m_SurfaceMeshFaceLabels[t * 2] == spin)
      {
        // winding = 0; // 0 = Write it using forward spin
      }
      else if(m_SurfaceMeshFaceLabels[t * 2 + 1] == spin)
      {
        // winding = 1; // Write it using backward spin
        // Switch the 2 node indices
        MeshIndexType temp = nId1;
        nId1 = nId2;
        nId2 = temp;
      }
      else
      {
        continue; // We do not match either spin so move to the next triangle
      }

      vert2[0] = static_cast<float>(nodes[nId1 * 3]);
      vert2[1] = static_cast<float>(nodes[nId1 * 3 + 1]);
      vert2[2] = static_cast<float>(nodes[nId1 * 3 + 2]);

      vert3[0] = static_cast<float>(nodes[nId2 * 3]);
      vert3[1] = static_cast<float>(nodes[nId2 * 3 + 1]);
      vert3[2] = static_cast<float>(nodes[nId2 * 3 + 2]);

      // Compute the normal
      u[0] = vert2[0] - vert1[0];
      u[1] = vert2[1] - vert1[1];
      u[2] = vert2[2] - vert1[2];

      w[0] = vert3[0] - vert1[0];
      w[1] = vert3[1] - vert1[1];
      w[2] = vert3[2] - vert1[2];

      normal[0] = u[1] * w[2] - u[2] * w[1];
      normal[1] = u[2] * w[0] - u[0] * w[2];
      normal[2] = u[0] * w[1] - u[1] * w[0];

      length = sqrtf(normal[0] * normal[0] + normal[1] * normal[1] + normal[2] * normal[2]);
      normal[0] = normal[0] / length;
      normal[1] = normal[1] / length;
      normal[2] = normal[2] / length;

      totalWritten = fwrite(data, 1, 50, f);
      if(totalWritten != 50)
      {
        QString ss = QObject::tr("Error Writing STL File. Not enough elements written for Feature Id %1. Wrote %2 of 50.").arg(spin).arg(totalWritten);
        setErrorCondition(-1201, ss);
      }
      triCount++;
    }
    fclose(f);
    err = writeNumTrianglesToFile(filename, triCount);
  }

  clearErrorCode();
  clearWarningCode();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int32_t WriteStlFile::writeHeader(FILE* f, const QString& header, int32_t triCount)
{
  if(nullptr == f)
  {
    return -1;
  }
  char h[80];
  size_t headlength = 80;
  if(header.length() < 80)
  {
    headlength = static_cast<size_t>(header.length());
  }

  std::string c_str = header.toStdString();
  ::memset(h, 0, 80);
  ::memcpy(h, c_str.data(), headlength);
  // Return the number of bytes written - which should be 80
  fwrite(h, 1, 80, f);
  fwrite(&triCount, 1, 4, f);
  return 0;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int32_t WriteStlFile::writeNumTrianglesToFile(const QString& filename, int32_t triCount)
{
  // We need to update the number of triangles in the file
  int32_t err = 0;

  FILE* out = fopen(filename.toLatin1().data(), "r+b");
  fseek(out, 80L, SEEK_SET);
  fwrite(reinterpret_cast<char*>(&triCount), 1, 4, out);
  fclose(out);

  return err;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
AbstractFilter::Pointer WriteStlFile::newFilterInstance(bool copyFilterParameters) const
{
  WriteStlFile::Pointer filter = WriteStlFile::New();
  if(copyFilterParameters)
  {
    copyFilterParameterInstanceVariables(filter.get());
  }
  return filter;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString WriteStlFile::getCompiledLibraryName() const
{
  return ImportExportConstants::ImportExportBaseName;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString WriteStlFile::getBrandingString() const
{
  return "IO";
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString WriteStlFile::getFilterVersion() const
{
  QString version;
  QTextStream vStream(&version);
  vStream << ImportExport::Version::Major() << "." << ImportExport::Version::Minor() << "." << ImportExport::Version::Patch();
  return version;
}
// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString WriteStlFile::getGroupName() const
{
  return SIMPL::FilterGroups::IOFilters;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QUuid WriteStlFile::getUuid() const
{
  return QUuid("{b9134758-d5e5-59dd-9907-28d23e0e0143}");
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString WriteStlFile::getSubGroupName() const
{
  return SIMPL::FilterSubGroups::OutputFilters;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString WriteStlFile::getHumanLabel() const
{
  return "Export STL Files from Triangle Geometry";
}

// -----------------------------------------------------------------------------
WriteStlFile::Pointer WriteStlFile::NullPointer()
{
  return Pointer(static_cast<Self*>(nullptr));
}

// -----------------------------------------------------------------------------
std::shared_ptr<WriteStlFile> WriteStlFile::New()
{
  struct make_shared_enabler : public WriteStlFile
  {
  };
  std::shared_ptr<make_shared_enabler> val = std::make_shared<make_shared_enabler>();
  val->setupFilterParameters();
  return val;
}

// -----------------------------------------------------------------------------
QString WriteStlFile::getNameOfClass() const
{
  return QString("WriteStlFile");
}

// -----------------------------------------------------------------------------
QString WriteStlFile::ClassName()
{
  return QString("WriteStlFile");
}

// -----------------------------------------------------------------------------
void WriteStlFile::setOutputStlDirectory(const QString& value)
{
  m_OutputStlDirectory = value;
}

// -----------------------------------------------------------------------------
QString WriteStlFile::getOutputStlDirectory() const
{
  return m_OutputStlDirectory;
}

// -----------------------------------------------------------------------------
void WriteStlFile::setOutputStlPrefix(const QString& value)
{
  m_OutputStlPrefix = value;
}

// -----------------------------------------------------------------------------
QString WriteStlFile::getOutputStlPrefix() const
{
  return m_OutputStlPrefix;
}

// -----------------------------------------------------------------------------
void WriteStlFile::setGroupByPhase(bool value)
{
  m_GroupByPhase = value;
}

// -----------------------------------------------------------------------------
bool WriteStlFile::getGroupByPhase() const
{
  return m_GroupByPhase;
}

// -----------------------------------------------------------------------------
void WriteStlFile::setSurfaceMeshFaceLabelsArrayPath(const DataArrayPath& value)
{
  m_SurfaceMeshFaceLabelsArrayPath = value;
}

// -----------------------------------------------------------------------------
DataArrayPath WriteStlFile::getSurfaceMeshFaceLabelsArrayPath() const
{
  return m_SurfaceMeshFaceLabelsArrayPath;
}

// -----------------------------------------------------------------------------
void WriteStlFile::setSurfaceMeshFacePhasesArrayPath(const DataArrayPath& value)
{
  m_SurfaceMeshFacePhasesArrayPath = value;
}

// -----------------------------------------------------------------------------
DataArrayPath WriteStlFile::getSurfaceMeshFacePhasesArrayPath() const
{
  return m_SurfaceMeshFacePhasesArrayPath;
}
