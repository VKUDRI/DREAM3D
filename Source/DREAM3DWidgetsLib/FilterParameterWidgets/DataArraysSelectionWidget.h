/* ============================================================================
* Copyright (c) 2012 Michael A. Jackson (BlueQuartz Software)
* Copyright (c) 2012 Dr. Michael A. Groeber (US Air Force Research Laboratories)
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
* Neither the name of Michael A. Groeber, Michael A. Jackson, the US Air Force,
* BlueQuartz Software nor the names of its contributors may be used to endorse
* or promote products derived from this software without specific prior written
* permission.
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
*  This code was written under United States Air Force Contract number
*                           FA8650-10-D-5210
*
* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
#ifndef _DataArraysSelectionWidget_H_
#define _DataArraysSelectionWidget_H_


#include <QtCore/QObject>
#include <QtCore/QPointer>
#include <QtWidgets/QWidget>

#include "QtSupport/FaderWidget.h"


#include "DREAM3DLib/DataContainers/DataArrayPath.h"
#include "DREAM3DLib/DataContainers/DataContainerArrayProxy.h"

#include "DREAM3DWidgetsLib/DREAM3DWidgetsLib.h"
#include "DREAM3DWidgetsLib/FilterParameterWidgets/FilterParameterWidget.h"


#include "DREAM3DWidgetsLib/ui_DataArraysSelectionWidget.h"

class AbstractFilter;
class FilterParameter;

/**
* @brief
* @author
* @version
*/
class DREAM3DWidgetsLib_EXPORT DataArraysSelectionWidget : public FilterParameterWidget, private Ui::DataArraysSelectionWidget
{
	Q_OBJECT

public:
	/**
	* @brief Constructor
	* @param parameter The FilterParameter object that this widget represents
	* @param filter The instance of the filter that this parameter is a part of
	* @param parent The parent QWidget for this Widget
	*/
	DataArraysSelectionWidget(FilterParameter* parameter, AbstractFilter* filter = NULL, QWidget* parent = NULL);

	DataArraysSelectionWidget(QWidget* parent = NULL);

	virtual ~DataArraysSelectionWidget();

	/**
	* @brief This method does additional GUI widget connections
	*/
	void setupGui();

	/**
	* @brief checkStringValues
	* @param current
	* @param filt
	* @return
	*/
	QString checkStringValues(QString current, QString filtDcName);

	/**
	* @brief initializeWidget
	* @param parameter
	* @param filter
	*/
	void initializeWidget(FilterParameter* parameter, AbstractFilter* filter);


	public slots:
	void beforePreflight();
	void afterPreflight();
	void filterNeedsInputParameters(AbstractFilter* filter);

	void on_dataContainerList_currentIndexChanged(int index);

	void on_attributeMatrixList_currentIndexChanged(int index);



protected:
	void populateComboBoxes();
	void populateAttributeMatrixList();
	void populateAttributeArrayList();

	/**
	* @brief generateDCAProxy
	* @return
	*/
	DataContainerArrayProxy generateDCAProxy();

signals:
	void errorSettingFilterParameter(const QString& msg);
	void parametersChanged();

private:

	bool m_DidCausePreflight;


	DataContainerArrayProxy m_DcaProxy;
	DataArrayPath  m_DefaultPath;

	DataArraysSelectionWidget(const DataArraysSelectionWidget&); // Copy Constructor Not Implemented
	void operator=(const DataArraysSelectionWidget&); // Operator '=' Not Implemented

};

#endif /* _DataArraysSelectionWidget_H_ */


