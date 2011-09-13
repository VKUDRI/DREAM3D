/* ============================================================================
 * Copyright (c) 2011, Michael A. Jackson (BlueQuartz Software)
 * All rights reserved.
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
 * Neither the name of Michael A. Jackson nor the names of its contributors may
 * be used to endorse or promote products derived from this software without
 * specific prior written permission.
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
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

#ifndef VTKRECTILINEARGRIDREADER_H_
#define VTKRECTILINEARGRIDREADER_H_

#include <string>

#include "DREAM3D/DREAM3DLibTypes.h"
#include "DREAM3D/Common/DREAM3DSetGetMacros.h"
#include "DREAM3D/Common/AIMArray.hpp"

#include "VTKFileReader.h"

class DREAM3DLib_EXPORT VTKRectilinearGridFileReader : public VTKFileReader
{
  public:
    DREAM3D_SHARED_POINTERS(VTKRectilinearGridFileReader);
    DREAM3D_TYPE_MACRO(VTKRectilinearGridFileReader)
    DREAM3D_STATIC_NEW_MACRO(VTKRectilinearGridFileReader);
    virtual ~VTKRectilinearGridFileReader();

    DREAM3D_INSTANCE_STRING_PROPERTY(GrainIdScalarName)
    DREAM3D_INSTANCE_PROPERTY(AIMArray<int>::Pointer, GrainIds);

    int parseCoordinateLine(const char* input, int &value);

    /**
     * @brief Reads the VTK header and sets the values that are described in the header
     * @return Error Condition. Negative is Error.
     */
    int readHeader();

    int readFile();

  protected:
    VTKRectilinearGridFileReader();

  private:

    VTKRectilinearGridFileReader(const VTKRectilinearGridFileReader&); // Copy Constructor Not Implemented
    void operator=(const VTKRectilinearGridFileReader&); // Operator '=' Not Implemented
};

#endif /* VTKRECTILINEARGRIDREADER_H_ */
