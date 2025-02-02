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

#include "StatsGeneratorWidget.h"

#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QJsonDocument>
#include <QtCore/QString>
#include <QtCore/QVector>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QProgressDialog>

#include "H5Support/H5Lite.h"
#include "H5Support/H5ScopedSentinel.h"
#include "H5Support/H5Utilities.h"
#include "H5Support/QH5Utilities.h"

#include "SIMPLib/CoreFilters/DataContainerReader.h"
#include "SIMPLib/CoreFilters/DataContainerWriter.h"
#include "SIMPLib/DataArrays/StatsDataArray.h"
#include "SIMPLib/DataContainers/DataContainer.h"
#include "SIMPLib/DataContainers/DataContainerArray.h"
#include "SIMPLib/Filtering/FilterPipeline.h"
#include "SIMPLib/StatsData/BoundaryStatsData.h"
#include "SIMPLib/StatsData/MatrixStatsData.h"
#include "SIMPLib/StatsData/PrecipitateStatsData.h"
#include "SIMPLib/StatsData/PrimaryStatsData.h"
#include "SIMPLib/StatsData/TransformationStatsData.h"
#include "SIMPLib/Utilities/FilterCompatibility.hpp"

#include "SVWidgetsLib/Widgets/SVStyle.h"

#include "SyntheticBuilding/FilterParameters/StatsGeneratorFilterParameter.h"
#include "SyntheticBuilding/Gui/Widgets/EditPhaseDialog.h"
#include "SyntheticBuilding/Gui/Widgets/StatsProgressWidget.h"
#include "SyntheticBuilding/SyntheticBuildingFilters/StatsGeneratorFilter.h"

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
StatsGeneratorWidget::StatsGeneratorWidget(FilterParameter* parameter, AbstractFilter* filter, QWidget* parent)
: FilterParameterWidget(parameter, filter, parent)
{
  m_FilterParameter = SIMPL_FILTER_PARAMETER_COMPATIBILITY_CHECK(filter, parameter, StatsGeneratorWidget, StatsGeneratorFilterParameter);
  m_Filter = SIMPL_FILTER_COMPATIBILITY_CHECK(filter, parameter, StatsGeneratorWidget, StatsGeneratorFilter);

  m_OpenDialogLastFilePath = QDir::homePath();
  setWidgetIsExpanding(true);
  setupUi(this);
  setupGui();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
StatsGeneratorWidget::~StatsGeneratorWidget() = default;

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void StatsGeneratorWidget::setupGui()
{
  phaseTabs->clear();

  m_NeedDataLoad = false;
  m_LoadDataWidgets.clear();
  m_LoadDataWidgets.push_back(nullptr);

  StatsDataArray::Pointer sda = m_Filter->getStatsDataArray();
  if((sda && sda->getNumberOfTuples() == 0) || !sda)
  {
    PrimaryPhaseWidget* ppw = new PrimaryPhaseWidget;
    ppw->setPhaseIndex(1);
    ppw->setPhaseType(PhaseType::Type::Primary);
    ppw->setCrystalStructure(EbsdLib::CrystalStructure::Cubic_High);
    ppw->setPhaseName("Primary (1)");
    ppw->setPhaseFraction(1.0);
    ppw->setTotalPhaseFraction(1.0);
    phaseTabs->addTab(ppw, "Primary");

    connect(ppw, SIGNAL(dataChanged()), this, SIGNAL(parametersChanged()));
  }
  else
  {
    m_NeedDataLoad = true;
    size_t ensembles = sda->getNumberOfTuples();

    std::vector<size_t> tDims(1, ensembles);
    m_CellEnsembleAttrMat = AttributeMatrix::New(tDims, SIMPL::Defaults::CellEnsembleAttributeMatrixName, AttributeMatrix::Type::CellEnsemble);
    m_CellEnsembleAttrMat->insertOrAssign(sda);
    UInt32ArrayType::Pointer phaseTypes = m_Filter->getPhaseTypes();
    m_CellEnsembleAttrMat->insertOrAssign(phaseTypes);
    UInt32ArrayType::Pointer crystalStructures = m_Filter->getCrystalStructures();
    m_CellEnsembleAttrMat->insertOrAssign(crystalStructures);

    for(size_t phase = 1; phase < ensembles; ++phase)
    {

      StatsData::Pointer statsData = sda->getStatsData(phase);

      if(phaseTypes->getValue(phase) == static_cast<PhaseType::EnumType>(PhaseType::Type::Boundary))
      {
        BoundaryPhaseWidget* w = new BoundaryPhaseWidget(this);
        phaseTabs->addTab(w, w->getTabTitle());
        connect(w, SIGNAL(dataChanged()), this, SIGNAL(parametersChanged()));
        m_LoadDataWidgets.push_back(w);
      }
      else if(phaseTypes->getValue(phase) == static_cast<PhaseType::EnumType>(PhaseType::Type::Matrix))
      {
        MatrixPhaseWidget* w = new MatrixPhaseWidget(this);
        phaseTabs->addTab(w, w->getTabTitle());
        connect(w, SIGNAL(dataChanged()), this, SIGNAL(parametersChanged()));
        m_LoadDataWidgets.push_back(w);
      }
      if(phaseTypes->getValue(phase) == static_cast<PhaseType::EnumType>(PhaseType::Type::Precipitate))
      {
        PrecipitatePhaseWidget* w = new PrecipitatePhaseWidget(this);
        phaseTabs->addTab(w, w->getTabTitle());
        connect(w, SIGNAL(dataChanged()), this, SIGNAL(parametersChanged()));
        m_LoadDataWidgets.push_back(w);
      }
      if(phaseTypes->getValue(phase) == static_cast<PhaseType::EnumType>(PhaseType::Type::Primary))
      {
        PrimaryPhaseWidget* w = new PrimaryPhaseWidget(this);
        phaseTabs->addTab(w, w->getTabTitle());
        connect(w, SIGNAL(dataChanged()), this, SIGNAL(parametersChanged()));
        m_LoadDataWidgets.push_back(w);
      }
      if(phaseTypes->getValue(phase) == static_cast<PhaseType::EnumType>(PhaseType::Type::Transformation))
      {
        TransformationPhaseWidget* w = new TransformationPhaseWidget(this);
        phaseTabs->addTab(w, w->getTabTitle());
        connect(w, SIGNAL(dataChanged()), this, SIGNAL(parametersChanged()));
        m_LoadDataWidgets.push_back(w);
      }
      else
      {
      }
    }
    // progress.setValue(ensembles);

    // Now delete the first Phase from the Combo which was left over from something else
    phaseTabs->setCurrentIndex(0);
  }

  // Catch when the filter is about to execute the preflight
  connect(getFilter(), &AbstractFilter::preflightAboutToExecute, this, &StatsGeneratorWidget::beforePreflight);

  // Catch when the filter is finished running the preflight
  connect(getFilter(), &AbstractFilter::preflightExecuted, this, &StatsGeneratorWidget::afterPreflight);

  // Catch when the filter wants its values updated
  connect(getFilter(), &AbstractFilter::updateFilterParameters, this, &StatsGeneratorWidget::filterNeedsInputParameters);

  // Hide the debugging buttons
  updatePipelineBtn->hide();
  saveJsonBtn->hide();
  saveH5Btn->hide();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void StatsGeneratorWidget::loadData()
{
  if(m_NeedDataLoad)
  {
    size_t ensembles = m_LoadDataWidgets.size();
    StatsProgressWidget progress("Opening Stats File....", "Cancel", nullptr);
    progress.setVisible(true);
    progress.show();
    qApp->processEvents();

    int phase = 0;
    for(auto w : m_LoadDataWidgets)
    {
      progress.setValue(phase);

      if(progress.wasCanceled())
      {
        return;
      }

      if(nullptr != w)
      {
        progress.setLabelText("Opening Phase...");
        connect(w, SIGNAL(progressText(const QString&)), &progress, SLOT(setLabelText(const QString&)));
        QString progTitle = QString("%1/%2: Extracting Phase Type: %3").arg(phase).arg(ensembles - 1).arg(w->getTabTitle());
        progress.setProgTitle(progTitle);
        w->extractStatsData(m_CellEnsembleAttrMat, static_cast<int>(phase));
      }
      phase++;
    }
    m_NeedDataLoad = false;
    m_LoadDataWidgets.clear();
    m_CellEnsembleAttrMat = AttributeMatrix::NullPointer();
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void StatsGeneratorWidget::on_updatePipelineBtn_clicked()
{
  Q_EMIT parametersChanged();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void StatsGeneratorWidget::filterNeedsInputParameters(AbstractFilter* filter)
{
  StatsGeneratorFilter* statsGenFilter = dynamic_cast<StatsGeneratorFilter*>(filter);

  if(nullptr != statsGenFilter)
  {
    DataContainerArray::Pointer dca = generateDataContainerArray();
    if(nullptr == dca)
    {
      return;
    }
    DataContainer::Pointer dc = dca->getDataContainer(SIMPL::Defaults::StatsGenerator);
    AttributeMatrix::Pointer cellEnsembleAttrMat = dc->getAttributeMatrix(SIMPL::Defaults::CellEnsembleAttributeMatrixName);
    if(nullptr == cellEnsembleAttrMat)
    {
      return;
    }
    IDataArray::Pointer iDataArray = cellEnsembleAttrMat->getAttributeArray(SIMPL::EnsembleData::Statistics);

    StatsDataArray::Pointer statsDataArray = std::dynamic_pointer_cast<StatsDataArray>(iDataArray);
    if(nullptr == statsDataArray)
    {
      return;
    }
    iDataArray = cellEnsembleAttrMat->getAttributeArray(SIMPL::EnsembleData::CrystalStructures);
    UInt32ArrayType::Pointer crystalStructures = std::dynamic_pointer_cast<UInt32ArrayType>(iDataArray);

    iDataArray = cellEnsembleAttrMat->getAttributeArray(SIMPL::EnsembleData::PhaseTypes);
    UInt32ArrayType::Pointer phaseTypes = std::dynamic_pointer_cast<UInt32ArrayType>(iDataArray);

    iDataArray = cellEnsembleAttrMat->getAttributeArray(SIMPL::EnsembleData::PhaseName);
    StringDataArray::Pointer phaseNames = std::dynamic_pointer_cast<StringDataArray>(iDataArray);

    statsGenFilter->setStatsDataArray(statsDataArray);
    statsGenFilter->setCrystalStructures(crystalStructures);
    statsGenFilter->setPhaseTypes(phaseTypes);
    statsGenFilter->setPhaseNames(phaseNames);
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void StatsGeneratorWidget::beforePreflight()
{
  for(int i = 0; i < phaseTabs->count(); i++)
  {
    StatsGenWidget* sgwidget = qobject_cast<StatsGenWidget*>(phaseTabs->widget(i));
    if((qobject_cast<MatrixPhaseWidget*>(sgwidget) == nullptr) && (qobject_cast<BoundaryPhaseWidget*>(sgwidget) == nullptr) && (qobject_cast<TransformationPhaseWidget*>(sgwidget) == nullptr))
    {
      if(!sgwidget->getDataHasBeenGenerated())
      {
        QString ss = QObject::tr("Statistics data needs to be generated for phase %1 (%2)\n"
                                 "Click the Create Data button to generate the statistics data")
                         .arg(sgwidget->getPhaseIndex())
                         .arg(sgwidget->getTabTitle());
        m_Filter->setErrorCondition(-1, ss);
      }
      if(sgwidget->getBulkLoadFailure())
      {
        QString ss = QObject::tr("A valid angles file is needed to bulk load orientaiton weights and spreads for phase %1 (%2)\n"
                                 "Select an angles file and click the Load Data button to load the orientations")
                         .arg(sgwidget->getPhaseIndex())
                         .arg(sgwidget->getTabTitle());
        m_Filter->setErrorCondition(-1, ss);
      }
    }
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void StatsGeneratorWidget::afterPreflight()
{
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void StatsGeneratorWidget::on_addPhase_clicked()
{
  // Ensure the Current StatsGenWidget has generated its data first:
  StatsGenWidget* sgwidget = qobject_cast<StatsGenWidget*>(phaseTabs->currentWidget());
  if(!sgwidget->getDataHasBeenGenerated())
  {
    int r = QMessageBox::warning(this, tr("StatsGenerator"), tr("Data for the current phase has NOT been generated.\nDo you want to generate it now?"), QMessageBox::Ok | QMessageBox::Cancel);
    if(r == QMessageBox::Ok)
    {
      // The user wants to generate the data. Generate it and move on
      sgwidget->generateDefaultData();
    }
    else if(r == QMessageBox::Cancel)
    {
      return;
    }
  }

  double phaseFractionTotal = 0.0;
  for(int p = 0; p < phaseTabs->count(); ++p)
  {
    StatsGenWidget* sgwidget = qobject_cast<StatsGenWidget*>(phaseTabs->widget(p));
    phaseFractionTotal += sgwidget->getPhaseFraction();
  }

  EditPhaseDialog dialog;
  dialog.setEditFlag(true);
  dialog.setOtherPhaseFractionTotal(static_cast<float>(phaseFractionTotal));
  int r = dialog.exec();
  if(r == QDialog::Accepted)
  {
    if(dialog.getPhaseType() == PhaseType::Type::Primary)
    {
      PrimaryPhaseWidget* ppw = new PrimaryPhaseWidget();
      phaseTabs->addTab(ppw, dialog.getPhaseName());

      connect(ppw, SIGNAL(dataChanged()), this, SIGNAL(parametersChanged()));

      ppw->setPhaseIndex(phaseTabs->count());
      ppw->setPhaseType(PhaseType::Type::Primary);
      ppw->setCrystalStructure(dialog.getCrystalStructure());
      ppw->setPhaseFraction(dialog.getPhaseFraction());
      ppw->setPhaseType(dialog.getPhaseType());
      ppw->setPhaseName(dialog.getPhaseName());

      QString cName = ppw->getComboString();
      ppw->setObjectName(cName);
      ppw->updatePlots();
      setWindowModified(true);
    }
    else if(dialog.getPhaseType() == PhaseType::Type::Precipitate)
    {
      PrecipitatePhaseWidget* ppw = new PrecipitatePhaseWidget();
      phaseTabs->addTab(ppw, dialog.getPhaseName());

      connect(ppw, SIGNAL(dataChanged()), this, SIGNAL(parametersChanged()));

      ppw->setPhaseIndex(phaseTabs->count());
      ppw->setPhaseType(PhaseType::Type::Precipitate);
      ppw->setCrystalStructure(dialog.getCrystalStructure());
      ppw->setPhaseFraction(dialog.getPhaseFraction());
      ppw->setPhaseType(dialog.getPhaseType());
      ppw->setPptFraction(dialog.getPptFraction());
      ppw->setPhaseName(dialog.getPhaseName());
      QString cName = ppw->getComboString();
      ppw->setObjectName(cName);
      ppw->updatePlots();
      setWindowModified(true);
    }
    else if(dialog.getPhaseType() == PhaseType::Type::Transformation)
    {
      TransformationPhaseWidget* tpw = new TransformationPhaseWidget();
      phaseTabs->addTab(tpw, dialog.getPhaseName());

      connect(tpw, SIGNAL(dataChanged()), this, SIGNAL(parametersChanged()));

      tpw->setPhaseIndex(phaseTabs->count());
      tpw->setPhaseType(PhaseType::Type::Transformation);
      tpw->setCrystalStructure(dialog.getCrystalStructure());
      tpw->setPhaseFraction(dialog.getPhaseFraction());
      tpw->setPhaseType(dialog.getPhaseType());
      tpw->setPhaseName(dialog.getPhaseName());
      QString cName = tpw->getComboString();
      tpw->setObjectName(cName);
      tpw->updatePlots();
      setWindowModified(true);
    }
    else if(dialog.getPhaseType() == PhaseType::Type::Matrix)
    {
      MatrixPhaseWidget* mpw = new MatrixPhaseWidget();
      phaseTabs->addTab(mpw, dialog.getPhaseName());

      connect(mpw, SIGNAL(dataChanged()), this, SIGNAL(parametersChanged()));

      mpw->setPhaseIndex(phaseTabs->count());
      mpw->setPhaseType(PhaseType::Type::Matrix);
      mpw->setCrystalStructure(dialog.getCrystalStructure());
      mpw->setPhaseFraction(dialog.getPhaseFraction());
      mpw->setPhaseType(dialog.getPhaseType());
      mpw->setPhaseName(dialog.getPhaseName());
      QString cName = mpw->getComboString();
      mpw->setObjectName(cName);
      setWindowModified(true);
    }
    else if(dialog.getPhaseType() == PhaseType::Type::Boundary)
    {
      BoundaryPhaseWidget* bpw = new BoundaryPhaseWidget();
      phaseTabs->addTab(bpw, dialog.getPhaseName());

      connect(bpw, SIGNAL(dataChanged()), this, SIGNAL(parametersChanged()));

      bpw->setPhaseIndex(phaseTabs->count());
      bpw->setPhaseType(PhaseType::Type::Boundary);
      bpw->setCrystalStructure(dialog.getCrystalStructure());
      bpw->setPhaseFraction(dialog.getPhaseFraction());
      bpw->setPhaseType(dialog.getPhaseType());
      bpw->setPhaseName(dialog.getPhaseName());
      QString cName = bpw->getComboString();
      bpw->setObjectName(cName);
      setWindowModified(true);
    }

    // Make sure the new tab is the active tab
    phaseTabs->setCurrentIndex(phaseTabs->count() - 1);
  }
  Q_EMIT parametersChanged();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void StatsGeneratorWidget::on_editPhase_clicked()
{
  double phaseFractionTotal = 0.0;
  EditPhaseDialog dialog;
  dialog.setEditFlag(false);

  StatsGenWidget* sgwidget = qobject_cast<StatsGenWidget*>(phaseTabs->currentWidget());

  for(int p = 0; p < phaseTabs->count(); ++p)
  {
    StatsGenWidget* currentWidget = qobject_cast<StatsGenWidget*>(phaseTabs->widget(p));
    if(sgwidget != currentWidget)
    {
      phaseFractionTotal += currentWidget->getPhaseFraction();
    }
    else
    {
      dialog.setPhaseFraction(currentWidget->getPhaseFraction());
    }
  }
  dialog.setPhaseType(sgwidget->getPhaseType());
  dialog.setOtherPhaseFractionTotal(static_cast<float>(phaseFractionTotal));
  dialog.setCrystalStructure(sgwidget->getCrystalStructure());
  QString phaseName = sgwidget->getPhaseName();
  dialog.setPhaseName(phaseName);

  if(dialog.getPhaseType() == PhaseType::Type::Primary)
  {
    //    PrimaryPhaseWidget* ppw = qobject_cast<PrimaryPhaseWidget*>(sgwidget);
  }
  else if(dialog.getPhaseType() == PhaseType::Type::Precipitate)
  {
    PrecipitatePhaseWidget* ppw = qobject_cast<PrecipitatePhaseWidget*>(sgwidget);
    if(ppw != nullptr)
    {
      dialog.setPptFraction(ppw->getPptFraction());
    }
  }
  else if(dialog.getPhaseType() == PhaseType::Type::Transformation)
  {
    TransformationPhaseWidget* tpw = qobject_cast<TransformationPhaseWidget*>(sgwidget);
    if(tpw != nullptr)
    {
      // dialog.setParentPhase(tpw->getParentPhase());
    }
  }
  else if(dialog.getPhaseType() == PhaseType::Type::Matrix)
  {
    //    MatrixPhaseWidget* mpw = qobject_cast<MatrixPhaseWidget*>(sgwidget);
  }
  else if(dialog.getPhaseType() == PhaseType::Type::Boundary)
  {
    //    BoundaryPhaseWidget* bpw = qobject_cast<BoundaryPhaseWidget*>(sgwidget);
  }
  int r = dialog.exec();
  if(r == QDialog::Accepted)
  {
    if(dialog.getPhaseType() == PhaseType::Type::Primary)
    {
      PrimaryPhaseWidget* ppw = qobject_cast<PrimaryPhaseWidget*>(sgwidget);
      ppw->setCrystalStructure(dialog.getCrystalStructure());
      ppw->setPhaseFraction(dialog.getPhaseFraction());
      ppw->setPhaseType(dialog.getPhaseType());
      ppw->setPhaseName(dialog.getPhaseName());
      phaseTabs->setTabText(phaseTabs->currentIndex(), dialog.getPhaseName());
      QString cName = ppw->getComboString();
      setWindowModified(true);
      Q_EMIT parametersChanged();
      // ppw->updatePlots();
    }
    if(dialog.getPhaseType() == PhaseType::Type::Precipitate)
    {
      PrecipitatePhaseWidget* ppw = qobject_cast<PrecipitatePhaseWidget*>(sgwidget);
      ppw->setCrystalStructure(dialog.getCrystalStructure());
      ppw->setPhaseFraction(dialog.getPhaseFraction());
      ppw->setPhaseType(dialog.getPhaseType());
      ppw->setPptFraction(dialog.getPptFraction());
      ppw->setPhaseName(dialog.getPhaseName());
      phaseTabs->setTabText(phaseTabs->currentIndex(), dialog.getPhaseName());
      QString cName = ppw->getComboString();
      setWindowModified(true);
      Q_EMIT parametersChanged();
      // ppw->updatePlots();
    }
    if(dialog.getPhaseType() == PhaseType::Type::Transformation)
    {
      TransformationPhaseWidget* tpw = qobject_cast<TransformationPhaseWidget*>(sgwidget);
      tpw->setCrystalStructure(dialog.getCrystalStructure());
      tpw->setPhaseFraction(dialog.getPhaseFraction());
      tpw->setPhaseType(dialog.getPhaseType());
      // tpw->setParentPhase(dialog.getParentPhase());
      tpw->setPhaseName(dialog.getPhaseName());
      phaseTabs->setTabText(phaseTabs->currentIndex(), dialog.getPhaseName());
      QString cName = tpw->getComboString();
      setWindowModified(true);
      Q_EMIT parametersChanged();
      // tpw->updatePlots();
    }
    if(dialog.getPhaseType() == PhaseType::Type::Matrix)
    {
      MatrixPhaseWidget* mpw = qobject_cast<MatrixPhaseWidget*>(sgwidget);
      mpw->setCrystalStructure(dialog.getCrystalStructure());
      mpw->setPhaseFraction(dialog.getPhaseFraction());
      mpw->setPhaseType(dialog.getPhaseType());
      mpw->setPhaseName(dialog.getPhaseName());
      phaseTabs->setTabText(phaseTabs->currentIndex(), dialog.getPhaseName());
      QString cName = mpw->getComboString();
      setWindowModified(true);
      Q_EMIT parametersChanged();
    }
    if(dialog.getPhaseType() == PhaseType::Type::Boundary)
    {
      BoundaryPhaseWidget* bpw = qobject_cast<BoundaryPhaseWidget*>(sgwidget);
      bpw->setCrystalStructure(dialog.getCrystalStructure());
      bpw->setPhaseFraction(dialog.getPhaseFraction());
      bpw->setPhaseType(dialog.getPhaseType());
      bpw->setPhaseName(dialog.getPhaseName());
      phaseTabs->setTabText(phaseTabs->currentIndex(), dialog.getPhaseName());
      QString cName = bpw->getComboString();
      setWindowModified(true);
      Q_EMIT parametersChanged();
    }
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void StatsGeneratorWidget::on_phaseTabs_tabCloseRequested(int index)
{
  if(phaseTabs->count() > 1)
  {
    // Remove the SGPhase object from the vector
    StatsGenWidget* currentWidget = qobject_cast<StatsGenWidget*>(phaseTabs->widget(index));
    phaseTabs->removeTab(index);
    currentWidget->deleteLater(); // Actually delete it

    // Reset the phase index for each SGPhase object
    for(int p = 0; p < phaseTabs->count(); ++p)
    {
      StatsGenWidget* sgwidget = qobject_cast<StatsGenWidget*>(phaseTabs->widget(p));
      sgwidget->setPhaseIndex(p + 1);
      sgwidget->setObjectName(sgwidget->getComboString());
    }
  }
  setWindowModified(true);
  Q_EMIT parametersChanged();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void StatsGeneratorWidget::on_deletePhase_clicked()
{
  on_phaseTabs_tabCloseRequested(phaseTabs->currentIndex());
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
bool StatsGeneratorWidget::verifyOutputPathParentExists(QString outFilePath, QLineEdit* lineEdit)
{
  QFileInfo fileinfo(outFilePath);
  QDir parent(fileinfo.dir());
  return parent.exists();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void StatsGeneratorWidget::on_actionSaveAs_triggered()
{
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
DataContainerArray::Pointer StatsGeneratorWidget::generateDataContainerArray()
{
  int err = 0;
  int nPhases = phaseTabs->count() + 1;
  DataContainerArray::Pointer dca = DataContainerArray::New();
  DataContainer::Pointer m = DataContainer::New(SIMPL::Defaults::StatsGenerator);
  dca->addOrReplaceDataContainer(m);

  std::vector<size_t> tDims(1, nPhases);
  AttributeMatrix::Pointer cellEnsembleAttrMat = AttributeMatrix::New(tDims, SIMPL::Defaults::CellEnsembleAttributeMatrixName, AttributeMatrix::Type::CellEnsemble);
  m->addOrReplaceAttributeMatrix(cellEnsembleAttrMat);

  StatsDataArray::Pointer statsDataArray = StatsDataArray::New();
  statsDataArray->resizeTuples(nPhases);
  cellEnsembleAttrMat->insertOrAssign(statsDataArray);

  std::vector<size_t> cDims(1, 1);
  UInt32ArrayType::Pointer crystalStructures = UInt32ArrayType::CreateArray(tDims, cDims, SIMPL::EnsembleData::CrystalStructures, true);
  crystalStructures->setValue(0, EbsdLib::CrystalStructure::UnknownCrystalStructure);
  cellEnsembleAttrMat->insertOrAssign(crystalStructures);

  UInt32ArrayType::Pointer phaseTypes = UInt32ArrayType::CreateArray(tDims, cDims, SIMPL::EnsembleData::PhaseTypes, true);
  phaseTypes->setValue(0, static_cast<PhaseType::EnumType>(PhaseType::Type::Unknown));
  cellEnsembleAttrMat->insertOrAssign(phaseTypes);

  StringDataArray::Pointer phaseNames = StringDataArray::CreateArray(tDims[0], SIMPL::EnsembleData::PhaseName, true);
  phaseNames->setValue(0, PhaseType::UnknownStr());
  cellEnsembleAttrMat->insertOrAssign(phaseNames);

  double phaseFractionTotal = 0.0;
  for(int p = 0; p < phaseTabs->count(); ++p)
  {
    StatsGenWidget* sgwidget = qobject_cast<StatsGenWidget*>(phaseTabs->widget(p));
    phaseFractionTotal += sgwidget->getPhaseFraction();
  }

  // Loop on all the phases
  for(int i = 0; i < phaseTabs->count(); ++i)
  {
    StatsGenWidget* sgwidget = qobject_cast<StatsGenWidget*>(phaseTabs->widget(i));
    sgwidget->setTotalPhaseFraction(static_cast<float>(phaseFractionTotal));
    if(sgwidget->getPhaseType() == PhaseType::Type::Primary)
    {
      PrimaryStatsData::Pointer data = PrimaryStatsData::New();
      data->setName("PrimaryStatsData");
      statsDataArray->setStatsData(i + 1, data);
    }
    if(sgwidget->getPhaseType() == PhaseType::Type::Precipitate)
    {
      PrecipitateStatsData::Pointer data = PrecipitateStatsData::New();
      data->setName("PrecipitateStatsData");
      statsDataArray->setStatsData(i + 1, data);
    }
    if(sgwidget->getPhaseType() == PhaseType::Type::Transformation)
    {
      TransformationStatsData::Pointer data = TransformationStatsData::New();
      data->setName("TransformationStatsData");
      statsDataArray->setStatsData(i + 1, data);
    }
    if(sgwidget->getPhaseType() == PhaseType::Type::Matrix)
    {
      MatrixStatsData::Pointer data = MatrixStatsData::New();
      data->setName("MatrixStatsData");
      statsDataArray->setStatsData(i + 1, data);
    }
    if(sgwidget->getPhaseType() == PhaseType::Type::Boundary)
    {
      BoundaryStatsData::Pointer data = BoundaryStatsData::New();
      data->setName("BoundaryStatsData");
      statsDataArray->setStatsData(i + 1, data);
    }
    err = sgwidget->gatherStatsData(cellEnsembleAttrMat);
    if(err < 0)
    {
      QString msg("Internal error gathering statistics from Statistics Widgets.\nError code ");
      msg.append(QString::number(err));
      QMessageBox::critical(this, QString("Gather StatsData Error"), msg, QMessageBox::Ok);
      return DataContainerArray::NullPointer();
    }
  }
  return dca;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void StatsGeneratorWidget::on_saveJsonBtn_clicked()
{

  QString proposedFile = m_OpenDialogLastFilePath + QDir::separator() + "Untitled.json";
  QString outFile = QFileDialog::getSaveFileName(this, tr("Save JSON File"), proposedFile, tr("JSON Files (*.json)"));

  if(outFile.isEmpty())
  {
    return;
  }
  QFileInfo fi(outFile);
  QString ext = fi.suffix();
  m_OpenDialogLastFilePath = outFile;

  DataContainerArray::Pointer dca = generateDataContainerArray();
  AttributeMatrix::Pointer am = dca->getAttributeMatrix(DataArrayPath(SIMPL::Defaults::StatsGenerator, SIMPL::Defaults::CellEnsembleAttributeMatrixName, ""));

  IDataArray::Pointer ida = am->getAttributeArray(SIMPL::EnsembleData::Statistics);
  StatsDataArray::Pointer sda = std::dynamic_pointer_cast<StatsDataArray>(ida);
  if(nullptr == sda.get())
  {
    QMessageBox::critical(this, QString("JSON File Save Error"), QString("StatsDataArray was nullptr or Invalid"), QMessageBox::Ok);
    return;
  }

  IDataArray::Pointer iDataArray = am->getAttributeArray(SIMPL::EnsembleData::CrystalStructures);
  // unsigned int* crystalStructures = std::dynamic_pointer_cast< UInt32ArrayType >(iDataArray)->getPointer(0);

  QJsonObject jsonRoot;

  sda->writeToJson(jsonRoot, std::dynamic_pointer_cast<UInt32ArrayType>(iDataArray));

  QJsonDocument doc(jsonRoot);

  QFile file(outFile);
  if(!file.open(QIODevice::WriteOnly | QIODevice::Text))
  {
    return;
  }

  QByteArray binary = doc.toJson();
  file.write(binary.data(), binary.length());
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void StatsGeneratorWidget::on_saveH5Btn_clicked()
{
  QString proposedFile = m_OpenDialogLastFilePath;

  QString h5file = QFileDialog::getSaveFileName(this, tr("Save DREAM.3D File"), proposedFile, tr("DREAM.3D Files (*.dream3d)"));

  if(h5file.isEmpty())
  {
    return;
  }

  QFileInfo fi(h5file);
  QString ext = fi.suffix();
  m_OpenDialogLastFilePath = fi.filePath();

  DataContainerArray::Pointer dca = generateDataContainerArray();

  DataContainerWriter::Pointer writer = DataContainerWriter::New();
  writer->setDataContainerArray(dca);
  writer->setOutputFile(h5file);
  writer->setWriteXdmfFile(false);
  writer->setWritePipeline(false);
  writer->execute();
  // Force the clean up of the writer by assigning a nullptr pointer which will
  // have the effect of executing the destructor of the H5StatsWriter Class
  writer = DataContainerWriter::NullPointer();

  setWindowTitle(h5file + " - StatsGenerator");
  setWindowModified(false);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void StatsGeneratorWidget::on_openStatsFile_clicked()
{
  QString proposedFile = m_OpenDialogLastFilePath + QDir::separator() + "Untitled.dream3d";
  QString h5file = QFileDialog::getOpenFileName(this, tr("Open Statistics File"), proposedFile, tr("DREAM3D Files (*.dream3d);;H5Stats Files(*.h5stats);;HDF5 Files(*.h5 *.hdf5);;All Files(*.*)"));
  if(h5file.isEmpty())
  {
    return;
  }

  // Make sure the file path is not empty and does exist on the system
  if(h5file.isEmpty())
  {
    QString ss = QObject::tr("Input file was empty").arg(h5file);
    QMessageBox::critical(this, QString("File Open Error"), ss, QMessageBox::Ok);
    return;
  }

  QFileInfo fi(h5file);
  if(!fi.exists())
  {
    QString ss = QObject::tr("Input file does not exist").arg(fi.absoluteFilePath());
    QMessageBox::critical(this, QString("File Open Error"), ss, QMessageBox::Ok);
    return;
  }

  // Set the last directory that contains our file
  m_OpenDialogLastFilePath = fi.path();

  // Delete any existing phases from the GUI
  phaseTabs->clear();

  DataContainerArray::Pointer dca = DataContainerArray::New();
  DataContainer::Pointer m = DataContainer::New(SIMPL::Defaults::StatsGenerator);
  dca->addOrReplaceDataContainer(m);
  std::vector<size_t> tDims(1, 0);
  AttributeMatrix::Pointer cellEnsembleAttrMat = AttributeMatrix::New(tDims, SIMPL::Defaults::CellEnsembleAttributeMatrixName, AttributeMatrix::Type::CellEnsemble);
  m->addOrReplaceAttributeMatrix(cellEnsembleAttrMat);

  hid_t fileId = QH5Utilities::openFile(h5file, true); // Open the file Read Only
  if(fileId < 0)
  {
    QString ss = QObject::tr(": Error opening input file '%1'").arg(h5file);
    QMessageBox::critical(this, QString("File Open Error"), ss, QMessageBox::Ok);
    return;
  }
  // This will make sure if we return early from this method that the HDF5 File is properly closed.
  H5ScopedFileSentinel scopedFileSentinel(fileId, true);
  hid_t dcaGid = H5Gopen(fileId, SIMPL::StringConstants::DataContainerGroupName.toLatin1().constData(), 0);
  scopedFileSentinel.addGroupId(dcaGid);

  hid_t dcGid = H5Gopen(dcaGid, SIMPL::Defaults::StatsGenerator.toLatin1().constData(), 0);
  if(dcGid < 0)
  {
    QString title = QObject::tr("Error Opening Data Container");
    QString msg = QObject::tr("Error opening default Data Container with name '%1'").arg(SIMPL::Defaults::StatsGenerator);
    QMessageBox::critical(this, title, msg, QMessageBox::Ok, QMessageBox::Ok);
    return;
  }
  scopedFileSentinel.addGroupId(dcGid);
  hid_t amGid = H5Gopen(dcGid, SIMPL::Defaults::CellEnsembleAttributeMatrixName.toLatin1().constData(), 0);
  scopedFileSentinel.addGroupId(amGid);

  // We need to read one of the arrays to get the number of phases so that we can resize our attributeMatrix
  UInt32ArrayType::Pointer phases = UInt32ArrayType::CreateArray(1, SIMPL::EnsembleData::PhaseTypes, true);
  int err = phases->readH5Data(amGid);
  if(err < 0)
  {

    QString title = QObject::tr("Error Opening DataArray");
    QString msg = QObject::tr("Error opening 'PhaseTypes' data array at location '%1/%2/%3/%4'")
                      .arg(SIMPL::StringConstants::DataContainerGroupName)
                      .arg(SIMPL::Defaults::StatsGenerator)
                      .arg(SIMPL::Defaults::CellEnsembleAttributeMatrixName)
                      .arg(SIMPL::EnsembleData::PhaseTypes);
    QMessageBox::critical(this, title, msg, QMessageBox::Ok, QMessageBox::Ok);
    return;
  }
  tDims[0] = phases->getNumberOfTuples();
  cellEnsembleAttrMat->resizeAttributeArrays(tDims);

  cellEnsembleAttrMat->addAttributeArrayFromHDF5Path(amGid, "Statistics", false);
  cellEnsembleAttrMat->addAttributeArrayFromHDF5Path(amGid, "CrystalStructures", false);
  cellEnsembleAttrMat->addAttributeArrayFromHDF5Path(amGid, "PhaseTypes", false);

  // Get the number of Phases
  size_t ensembles = cellEnsembleAttrMat->getNumberOfTuples();

  QProgressDialog progress("Opening Stats File....", "Cancel", 0, ensembles, this);
  progress.setWindowModality(Qt::WindowModal);

  IDataArray::Pointer iDataArray = cellEnsembleAttrMat->getAttributeArray(SIMPL::EnsembleData::PhaseTypes);
  PhaseType::EnumType* phaseTypes = std::dynamic_pointer_cast<UInt32ArrayType>(iDataArray)->getPointer(0);

  // We should iterate on all the phases here to start setting data and creating
  // all of the StatsGenPhase Objects
  for(size_t phase = 1; phase < ensembles; ++phase)
  {
    progress.setValue(phase);

    if(progress.wasCanceled())
    {
      return;
    }

    if(phaseTypes[phase] == static_cast<PhaseType::EnumType>(PhaseType::Type::Boundary))
    {
      progress.setLabelText("Opening Boundaray Phase...");
      BoundaryPhaseWidget* w = new BoundaryPhaseWidget(this);
      phaseTabs->addTab(w, w->getTabTitle());
      w->extractStatsData(cellEnsembleAttrMat, static_cast<int>(phase));
    }
    else if(phaseTypes[phase] == static_cast<PhaseType::EnumType>(PhaseType::Type::Matrix))
    {
      progress.setLabelText("Opening Matrix Phase...");
      MatrixPhaseWidget* w = new MatrixPhaseWidget(this);
      phaseTabs->addTab(w, w->getTabTitle());
      w->extractStatsData(cellEnsembleAttrMat, static_cast<int>(phase));
    }
    if(phaseTypes[phase] == static_cast<PhaseType::EnumType>(PhaseType::Type::Precipitate))
    {
      progress.setLabelText("Opening Precipitate Phase...");
      PrecipitatePhaseWidget* w = new PrecipitatePhaseWidget(this);
      phaseTabs->addTab(w, w->getTabTitle());
      w->extractStatsData(cellEnsembleAttrMat, static_cast<int>(phase));
    }
    if(phaseTypes[phase] == static_cast<PhaseType::EnumType>(PhaseType::Type::Primary))
    {
      progress.setLabelText("Opening Primary Phase...");
      PrimaryPhaseWidget* w = new PrimaryPhaseWidget(this);
      phaseTabs->addTab(w, w->getTabTitle());
      w->extractStatsData(cellEnsembleAttrMat, static_cast<int>(phase));
    }
    if(phaseTypes[phase] == static_cast<PhaseType::EnumType>(PhaseType::Type::Transformation))
    {
      progress.setLabelText("Opening Transformation Phase...");
      TransformationPhaseWidget* w = new TransformationPhaseWidget(this);
      phaseTabs->addTab(w, w->getTabTitle());
      w->extractStatsData(cellEnsembleAttrMat, static_cast<int>(phase));
    }
    else
    {
    }
  }
  progress.setValue(ensembles);

  // Now delete the first Phase from the Combo which was left over from something else
  phaseTabs->setCurrentIndex(0);

  // Set the window title correctly
  setWindowModified(false);
  QString windowTitle = QString("");
  setWindowTitle(windowTitle);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void StatsGeneratorWidget::displayDialogBox(const QString& title, const QString& text, QMessageBox::Icon icon)
{

  QMessageBox msgBox;
  msgBox.setText(title);
  msgBox.setInformativeText(text);
  msgBox.setStandardButtons(QMessageBox::Ok);
  msgBox.setDefaultButton(QMessageBox::Ok);
  msgBox.setIcon(icon);
  msgBox.exec();
}
