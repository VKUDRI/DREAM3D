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
#include "NodesTrianglesToVtk.h"

#include <cstdio>

#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QTextStream>

#include "SIMPLib/FilterParameters/AbstractFilterParametersReader.h"
#include "SIMPLib/FilterParameters/BooleanFilterParameter.h"
#include "SIMPLib/FilterParameters/InputFileFilterParameter.h"
#include "SIMPLib/FilterParameters/OutputFileFilterParameter.h"
#include "SIMPLib/Utilities/FileSystemPathHelper.h"
#include "SIMPLib/Utilities/SIMPLibEndian.h"

#include "ImportExport/ImportExportConstants.h"
#include "ImportExport/ImportExportVersion.h"

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
NodesTrianglesToVtk::NodesTrianglesToVtk() = default;

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
NodesTrianglesToVtk::~NodesTrianglesToVtk() = default;

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void NodesTrianglesToVtk::setupFilterParameters()
{
  FilterParameterVectorType parameters;

  parameters.push_back(SIMPL_NEW_INPUT_FILE_FP("Nodes File", NodesFile, FilterParameter::Category::Parameter, NodesTrianglesToVtk));
  parameters.push_back(SIMPL_NEW_INPUT_FILE_FP("Triangles File", TrianglesFile, FilterParameter::Category::Parameter, NodesTrianglesToVtk));
  parameters.push_back(SIMPL_NEW_OUTPUT_FILE_FP("Output Vtk File", OutputVtkFile, FilterParameter::Category::Parameter, NodesTrianglesToVtk));
  parameters.push_back(SIMPL_NEW_BOOL_FP("Write Binary Vtk File", WriteBinaryFile, FilterParameter::Category::Parameter, NodesTrianglesToVtk));
  parameters.push_back(SIMPL_NEW_BOOL_FP("Write Conformal Mesh", WriteConformalMesh, FilterParameter::Category::Parameter, NodesTrianglesToVtk));

  setFilterParameters(parameters);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void NodesTrianglesToVtk::readFilterParameters(AbstractFilterParametersReader* reader, int index)
{
  reader->openFilterGroup(this, index);
  setNodesFile(reader->readString("NodesFile", getNodesFile()));
  setTrianglesFile(reader->readString("TrianglesFile", getTrianglesFile()));
  setOutputVtkFile(reader->readString("OutputVtkFile", getOutputVtkFile()));
  setWriteBinaryFile(reader->readValue("WriteBinaryFile", getWriteBinaryFile()));
  setWriteConformalMesh(reader->readValue("WriteConformalMesh", getWriteConformalMesh()));
  reader->closeFilterGroup();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void NodesTrianglesToVtk::initialize()
{
  m_NodeKind = nullptr;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void NodesTrianglesToVtk::dataCheck()
{
  clearErrorCode();
  clearWarningCode();

  QFileInfo fi(m_TrianglesFile);
  if(m_TrianglesFile.isEmpty())
  {
    setErrorCondition(-1001, "Triangles file is not set correctly");
  }
  else if(!fi.exists())
  {

    if(getInPreflight())
    {
      QString ss = "Triangles file does not exist currently.\nYou must have another filter that creates these files before this filter in your pipeline";
      setWarningCondition(-1002, ss);
    }
    else
    {
      setErrorCondition(-1003, "Triangles file does not exist currently.\nYou must have another filter that creates these files before this filter in your pipeline");
    }
  }

  QFileInfo fii(m_NodesFile);
  if(m_NodesFile.isEmpty())
  {
    setErrorCondition(-1004, "Nodes file path or name is emtpy");
  }
  else if(!fii.exists())
  {

    if(getInPreflight())
    {
      QString ss = "Nodes file does not exist currently. You must have another filter that creates these files before this filter in your pipeline";
      setWarningCondition(-1005, ss);
    }
    else
    {
      setErrorCondition(-1006, "Nodes file does not exist currently. You must have another filter that creates these files before this filter in your pipeline");
    }
  }

  FileSystemPathHelper::CheckOutputFile(this, "Output VTK File", getOutputVtkFile(), true);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void NodesTrianglesToVtk::execute()
{
  // int err = 0;

  dataCheck();
  if(getErrorCode() < 0)
  {
    return;
  }
  initialize();

  // Open the Nodes file for reading
  FILE* nodesFile = std::fopen(m_NodesFile.toLatin1().data(), "rb+");
  if(nodesFile == nullptr)
  {

    QString ss = QObject::tr("Error opening nodes file '%1'").arg(m_NodesFile);
    setErrorCondition(-666, ss);
    return;
  }

  //  how many nodes are in the file
  int nNodes = 0;
  if(std::fscanf(nodesFile, "%d", &nNodes) != 1)
  {
    QString ss = QObject::tr("Error reading number of Nodes from file").arg(m_NodesFile);
    setErrorCondition(-668, ss);
    return;
  }
  else
  {
    QString ss = QObject::tr("Node Count from %1 File: %2").arg(getNodesFile()).arg(nNodes);
    notifyStatusMessage(ss);
  }

  // Open the triangles file for reading
  FILE* triFile = std::fopen(m_TrianglesFile.toLatin1().data(), "rb+");
  if(triFile == nullptr)
  {

    QString ss = QObject::tr(": Error opening Triangles file '%1'").arg(m_TrianglesFile);
    setErrorCondition(-667, ss);
    return;
  }
  // how many triangles are in the file
  int nTriangles = 0;
  if(std::fscanf(triFile, "%d", &nTriangles) != 1)
  {
    QString ss = QObject::tr("Error reading number of Triangles from file").arg(m_NodesFile);
    setErrorCondition(-668, ss);
    return;
  }
  else
  {
    QString ss = QObject::tr("Triangle Count from %1 File: %2").arg(getTrianglesFile()).arg(nTriangles);
    notifyStatusMessage(ss);
  }
  // Open the output VTK File for writing
  FILE* vtkFile = nullptr;
  vtkFile = std::fopen(getOutputVtkFile().toLatin1().data(), "wb");
  if(nullptr == vtkFile)
  {

    QString ss = QObject::tr(": Error creating Triangles VTK Visualization '%1'").arg(getOutputVtkFile());
    setErrorCondition(-668, ss);
    return;
  }
  fprintf(vtkFile, "# vtk DataFile Version 2.0\n");
  fprintf(vtkFile, "Data set from DREAM.3D Surface Meshing Module\n");
  if(m_WriteBinaryFile)
  {
    fprintf(vtkFile, "BINARY\n");
  }
  else
  {
    fprintf(vtkFile, "ASCII\n");
  }
  fprintf(vtkFile, "DATASET POLYDATA\n");
  fprintf(vtkFile, "POINTS %d float\n", nNodes);

  int nodeId = 0;
  int nodeKind = 0;
  float pos[3] = {0.0f, 0.0f, 0.0f};

  size_t totalWritten = 0;
  size_t nread = 0;
  // Write the POINTS data (Vertex)
  for(int i = 0; i < nNodes; i++)
  {
    nread = std::fscanf(nodesFile, "%d %d %f %f %f", &nodeId, &nodeKind, pos, pos + 1, pos + 2); // Read one set of positions from the nodes file
    if(nread != 5)
    {
      break;
    }
    if(m_WriteBinaryFile)
    {
      SIMPLib::Endian::FromSystemToBig::convert(pos[0]);
      SIMPLib::Endian::FromSystemToBig::convert(pos[1]);
      SIMPLib::Endian::FromSystemToBig::convert(pos[2]);
      totalWritten = fwrite(pos, sizeof(float), 3, vtkFile);
      if(totalWritten != sizeof(float) * 3)
      {
      }
    }
    else
    {
      fprintf(vtkFile, "%f %f %f\n", pos[0], pos[1], pos[2]); // Write the positions to the output file
    }
  }
  fclose(nodesFile);

  // Write the triangle indices into the vtk File
  // column 1 = triangle id, starts from zero
  // column 2 to 4 = node1, node2 and node3 of individual triangles
  // column 5 to 7 = edge1 (from node1 and node2), edge2 (from node2 and node3) and edge3 (from node3 and node1) of individual triangle
  // column 8 and 9 = neighboring spins of individual triangles, column 8 = spins on the left side when following winding order using right hand.
  int tData[9];
  int triangleCount = nTriangles;
  if(!m_WriteConformalMesh)
  {
    triangleCount = nTriangles * 2;
  }
  // Write the CELLS Data
  fprintf(vtkFile, "POLYGONS %d %d\n", triangleCount, (triangleCount * 4));
  for(int i = 0; i < nTriangles; i++)
  {
    // Read from the Input Triangles Temp File
    nread = std::fscanf(triFile, "%d %d %d %d %d %d %d %d %d", tData, tData + 1, tData + 2, tData + 3, tData + 4, tData + 5, tData + 6, tData + 7, tData + 8);
    if(m_WriteBinaryFile)
    {
      tData[0] = 3; // Push on the total number of entries for this entry
      SIMPLib::Endian::FromSystemToBig::convert(tData[0]);
      SIMPLib::Endian::FromSystemToBig::convert(tData[1]);
      SIMPLib::Endian::FromSystemToBig::convert(tData[2]);
      SIMPLib::Endian::FromSystemToBig::convert(tData[3]);
      fwrite(tData, sizeof(int), 4, vtkFile);
      if(!m_WriteConformalMesh)
      {
        tData[0] = tData[1];
        tData[1] = tData[3];
        tData[3] = tData[0];
        tData[0] = 3;
        SIMPLib::Endian::FromSystemToBig::convert(tData[0]);
        fwrite(tData, sizeof(int), 4, vtkFile);
      }
    }
    else
    {
      fprintf(vtkFile, "3 %d %d %d\n", tData[1], tData[2], tData[3]);
      if(!m_WriteConformalMesh)
      {
        fprintf(vtkFile, "3 %d %d %d\n", tData[3], tData[2], tData[1]);
      }
    }
  }
  fclose(triFile);

  int err = 0;
  // Write the CELL_DATA section
  if(m_WriteBinaryFile)
  {
    err = writeBinaryCellData(m_TrianglesFile, vtkFile, nTriangles, m_WriteConformalMesh);
    if(err < 0)
    {
      setErrorCondition(err, tr("Could not write binary cell data to file '%1'").arg(getOutputVtkFile()));
    }
  }
  else
  {
    err = writeASCIICellData(m_TrianglesFile, vtkFile, nTriangles, m_WriteConformalMesh);
    if(err < 0)
    {
      setErrorCondition(err, tr("Could not write ASCII cell data to file '%1'").arg(getOutputVtkFile()));
    }
  }

  // Write the POINT_DATA section
  if(m_WriteBinaryFile)
  {
    err = writeBinaryPointData(m_NodesFile, vtkFile, nNodes);
    if(err < 0)
    {
      setErrorCondition(err, tr("Could not write binary point data to file '%1'").arg(getOutputVtkFile()));
    }
  }
  else
  {
    err = writeASCIIPointData(m_NodesFile, vtkFile, nNodes);
    if(err < 0)
    {
      setErrorCondition(err, tr("Could not write ASCII point data to file '%1'").arg(getOutputVtkFile()));
    }
  }

  fprintf(vtkFile, "\n");
  // Free the memory
  // Close the input and output files
  fclose(vtkFile);

  clearErrorCode();
  clearWarningCode();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int NodesTrianglesToVtk::writeBinaryPointData(const QString& NodesFile, FILE* vtkFile, int nNodes)
{
  //# first line = number of nodes
  //# column 1 = node id, starts from zero.
  //# column 2 = node type, 2 = on the feature boundary, 3 = on the triple line, 4 = quadruple point or on quadruple line
  //#                       if 10 is added, eg 12, 13, and 14, then they are on the surface of microstructure.
  //#                       For the nodes on surfaces of microstructure, negative wrapper spins are treated as neighboring features.
  //#                       12 = on the feature boundary trace area at the surface of microstructure
  //#                       13 = on the feature boundary trace line
  //#                 14 = on the triple point, and so on
  //# column 3 to 5 = coordinates of nodes, x, y, and z

  int err = 0;
  int nodeId = 0;
  int nodeKind = 0;
  float pos[3] = {0.0f, 0.0f, 0.0f};
  int swapped;
  int nread = 0;
  FILE* nodesFile = std::fopen(NodesFile.toLatin1().data(), "rb");
  fprintf(vtkFile, "\n");
  fprintf(vtkFile, "POINT_DATA %d\n", nNodes);
  fprintf(vtkFile, "SCALARS Node_Type int 1\n");
  fprintf(vtkFile, "LOOKUP_TABLE default\n");
  if(std::fscanf(nodesFile, "%d", &nodeId) != 1) // Read the number of nodes
  {
    QString ss = QObject::tr("Error reading number of Nodes from file").arg(m_NodesFile);
    setErrorCondition(-668, ss);
    std::ignore = fclose(nodesFile);
    return -668;
  }

  std::vector<int> data(nNodes, 0);
  for(int i = 0; i < nNodes; i++)
  {
    nread = std::fscanf(nodesFile, "%d %d %f %f %f", &nodeId, &nodeKind, pos, pos + 1, pos + 2); // Read one set of positions from the nodes file
    if(nread != 5)
    {
      break;
    }
    swapped = nodeKind;
    SIMPLib::Endian::FromSystemToBig::convert(swapped);

    data[i] = swapped;
  }
  int totalWritten = fwrite(&(data.front()), sizeof(int), nNodes, vtkFile);
  std::ignore = fclose(nodesFile);
  if(totalWritten != nNodes)
  {
    return -1;
  }
  return err;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int NodesTrianglesToVtk::writeASCIIPointData(const QString& NodesFile, FILE* vtkFile, int nNodes)
{
  int err = 0;
  int nodeId = 0;
  int nodeKind = 0;
  float pos[3] = {0.0f, 0.0f, 0.0f};
  int nread = 0;

  FILE* nodesFile = std::fopen(NodesFile.toLatin1().data(), "rb");
  fprintf(vtkFile, "\n");
  fprintf(vtkFile, "POINT_DATA %d\n", nNodes);
  fprintf(vtkFile, "SCALARS Node_Type int 1\n");
  fprintf(vtkFile, "LOOKUP_TABLE default\n");
  if(std::fscanf(nodesFile, "%d", &nodeId) != 1) // Read the number of nodes
  {
    QString ss = QObject::tr("Error reading number of Nodes from file").arg(m_NodesFile);
    setErrorCondition(-668, ss);
    std::ignore = fclose(nodesFile);
    return -668;
  }
  for(int i = 0; i < nNodes; i++)
  {
    nread = std::fscanf(nodesFile, "%d %d %f %f %f", &nodeId, &nodeKind, pos, pos + 1, pos + 2); // Read one set of positions from the nodes file
    if(nread != 5)
    {
      break;
    }
    fprintf(vtkFile, "%d\n", nodeKind); // Write the Node Kind to the output file
  }

  // Close the input files
  std::ignore = fclose(nodesFile);
  return err;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int NodesTrianglesToVtk::writeBinaryCellData(const QString& TrianglesFile, FILE* vtkFile, int nTriangles, bool conformalMesh)
{
  //# first line = number of triangles
  //# column 1 = triangle id, starts from zero
  //# column 2 to 4 = node1, node2 and node3 of individual triangles
  //# column 5 to 7 = edge1 (from node1 and node2), edge2 (from node2 and node3) and edge3 (from node3 and node1) of individual triangle
  //# column 8 and 9 = neighboring spins of individual triangles, column 8 = spins on the left side when following winding order using right hand.
  //
  int err = 0;
  size_t offset = 1;
  int nread = 0;
  // Open the triangles file for reading
  FILE* triFile = std::fopen(TrianglesFile.toLatin1().data(), "rb");
  if(std::fscanf(triFile, "%d", &nread) != 1) // Read the number of triangles and throw it away
  {
    QString ss = QObject::tr("Error reading number of Triangles from file").arg(m_NodesFile);
    setErrorCondition(-669, ss);
    std::ignore = fclose(triFile);
    return -668;
  }
  int tData[9];

  int triangleCount = nTriangles;
  if(!conformalMesh)
  {
    triangleCount = nTriangles * 2;
    offset = 2;
  }
  std::vector<int> tri_ids(triangleCount);
  // Write the FeatureId Data to the file
  fprintf(vtkFile, "\n");
  fprintf(vtkFile, "CELL_DATA %d\n", triangleCount);
  fprintf(vtkFile, "SCALARS FeatureID int 1\n");
  fprintf(vtkFile, "LOOKUP_TABLE default\n");

  std::vector<int> cell_data(triangleCount);
  for(int i = 0; i < nTriangles; i++)
  {
    nread = std::fscanf(triFile, "%d %d %d %d %d %d %d %d %d", tData, tData + 1, tData + 2, tData + 3, tData + 4, tData + 5, tData + 6, tData + 7, tData + 8);
    if(nread != 9)
    {
      return -1;
    }
    SIMPLib::Endian::FromSystemToBig::convert(tData[0]);
    tri_ids[i * offset] = tData[0];
    SIMPLib::Endian::FromSystemToBig::convert(tData[7]);
    cell_data[i * offset] = tData[7];
    if(!conformalMesh)
    {
      SIMPLib::Endian::FromSystemToBig::convert(tData[8]);
      cell_data[i * offset + 1] = tData[8];
      tri_ids[i * offset + 1] = tData[0];
    }
  }

  int totalWritten = fwrite(&(cell_data.front()), sizeof(int), triangleCount, vtkFile);
  if(totalWritten != triangleCount)
  {
    return -1;
  }

  // Now write the original Triangle Ids for Debugging in ParaView
  fprintf(vtkFile, "\n");
  fprintf(vtkFile, "SCALARS TriangleID int 1\n");
  fprintf(vtkFile, "LOOKUP_TABLE default\n");

  totalWritten = fwrite(&(tri_ids.front()), sizeof(int), triangleCount, vtkFile);
  if(totalWritten != triangleCount)
  {
    return -1;
  }

  fclose(triFile);

  return err;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int NodesTrianglesToVtk::writeASCIICellData(const QString& TrianglesFile, FILE* vtkFile, int nTriangles, bool conformalMesh)
{
  int nread = 0;
  // Open the triangles file for reading
  FILE* triFile = std::fopen(TrianglesFile.toLatin1().data(), "rb");
  if(std::fscanf(triFile, "%d", &nread) != 1) // Read the number of triangles and throw it away
  {
    QString ss = QObject::tr("Error reading number of Triangles from file").arg(m_NodesFile);
    setErrorCondition(-669, ss);
    std::ignore = fclose(triFile);
    return -669;
  }
  // Write the FeatureId Data to the file
  int triangleCount = nTriangles;
  if(!conformalMesh)
  {
    triangleCount = nTriangles * 2;
  }
  fprintf(vtkFile, "\n");
  fprintf(vtkFile, "CELL_DATA %d\n", triangleCount);
  fprintf(vtkFile, "SCALARS FeatureID int 1\n");
  fprintf(vtkFile, "LOOKUP_TABLE default\n");

  int tData[9];
  for(int i = 0; i < nTriangles; i++)
  {
    nread = std::fscanf(triFile, "%d %d %d %d %d %d %d %d %d", tData, tData + 1, tData + 2, tData + 3, tData + 4, tData + 5, tData + 6, tData + 7, tData + 8);
    if(nread != 9)
    {
      return -1;
    }
    fprintf(vtkFile, "%d\n", tData[7]);
    if(!conformalMesh)
    {
      fprintf(vtkFile, "%d\n", tData[8]);
    }
  }
  std::ignore = fclose(triFile);
  return 0;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
AbstractFilter::Pointer NodesTrianglesToVtk::newFilterInstance(bool copyFilterParameters) const
{
  /*
   * NodesFile
   * TrianglesFile
   * OutputVtkFile
   * WriteBinaryFile
   * WriteConformalMesh
   */
  NodesTrianglesToVtk::Pointer filter = NodesTrianglesToVtk::New();
  if(copyFilterParameters)
  {
    copyFilterParameterInstanceVariables(filter.get());
  }
  return filter;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString NodesTrianglesToVtk::getCompiledLibraryName() const
{
  return ImportExportConstants::ImportExportBaseName;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString NodesTrianglesToVtk::getBrandingString() const
{
  return "IO";
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString NodesTrianglesToVtk::getFilterVersion() const
{
  QString version;
  QTextStream vStream(&version);
  vStream << ImportExport::Version::Major() << "." << ImportExport::Version::Minor() << "." << ImportExport::Version::Patch();
  return version;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString NodesTrianglesToVtk::getGroupName() const
{
  return SIMPL::FilterGroups::IOFilters;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QUuid NodesTrianglesToVtk::getUuid() const
{
  return QUuid("{a1d2eddd-6420-53e3-823b-b44d4a5965bb}");
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString NodesTrianglesToVtk::getSubGroupName() const
{
  return SIMPL::FilterSubGroups::OutputFilters;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString NodesTrianglesToVtk::getHumanLabel() const
{
  return "Convert Nodes & Triangles To Vtk";
}

// -----------------------------------------------------------------------------
NodesTrianglesToVtk::Pointer NodesTrianglesToVtk::NullPointer()
{
  return Pointer(static_cast<Self*>(nullptr));
}

// -----------------------------------------------------------------------------
std::shared_ptr<NodesTrianglesToVtk> NodesTrianglesToVtk::New()
{
  struct make_shared_enabler : public NodesTrianglesToVtk
  {
  };
  std::shared_ptr<make_shared_enabler> val = std::make_shared<make_shared_enabler>();
  val->setupFilterParameters();
  return val;
}

// -----------------------------------------------------------------------------
QString NodesTrianglesToVtk::getNameOfClass() const
{
  return QString("NodesTrianglesToVtk");
}

// -----------------------------------------------------------------------------
QString NodesTrianglesToVtk::ClassName()
{
  return QString("NodesTrianglesToVtk");
}

// -----------------------------------------------------------------------------
void NodesTrianglesToVtk::setNodesFile(const QString& value)
{
  m_NodesFile = value;
}

// -----------------------------------------------------------------------------
QString NodesTrianglesToVtk::getNodesFile() const
{
  return m_NodesFile;
}

// -----------------------------------------------------------------------------
void NodesTrianglesToVtk::setTrianglesFile(const QString& value)
{
  m_TrianglesFile = value;
}

// -----------------------------------------------------------------------------
QString NodesTrianglesToVtk::getTrianglesFile() const
{
  return m_TrianglesFile;
}

// -----------------------------------------------------------------------------
void NodesTrianglesToVtk::setOutputVtkFile(const QString& value)
{
  m_OutputVtkFile = value;
}

// -----------------------------------------------------------------------------
QString NodesTrianglesToVtk::getOutputVtkFile() const
{
  return m_OutputVtkFile;
}

// -----------------------------------------------------------------------------
void NodesTrianglesToVtk::setWriteBinaryFile(bool value)
{
  m_WriteBinaryFile = value;
}

// -----------------------------------------------------------------------------
bool NodesTrianglesToVtk::getWriteBinaryFile() const
{
  return m_WriteBinaryFile;
}

// -----------------------------------------------------------------------------
void NodesTrianglesToVtk::setWriteConformalMesh(bool value)
{
  m_WriteConformalMesh = value;
}

// -----------------------------------------------------------------------------
bool NodesTrianglesToVtk::getWriteConformalMesh() const
{
  return m_WriteConformalMesh;
}
