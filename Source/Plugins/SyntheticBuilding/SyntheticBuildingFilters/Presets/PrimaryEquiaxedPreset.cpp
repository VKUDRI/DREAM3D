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

#include "PrimaryEquiaxedPreset.h"

#include "SIMPLib/Common/Constants.h"
#include "SIMPLib/Math/SIMPLibMath.h"
#include "SIMPLib/Math/SIMPLibRandom.h"

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
PrimaryEquiaxedPreset::PrimaryEquiaxedPreset() = default;

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
PrimaryEquiaxedPreset::~PrimaryEquiaxedPreset() = default;

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString PrimaryEquiaxedPreset::getName()
{
  return QString::fromLatin1("Primary Equiaxed");
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PrimaryEquiaxedPreset::initializeOmega3TableModel(QMap<QString, QVector<float>>& data)
{
  QVector<float>& binNumbers = data[kBinNumbers];
  qint32 count = binNumbers.count();

  float alpha, beta;
  SIMPL_RANDOMNG_NEW()

  QVector<float> alphas;
  QVector<float> betas;

  for(qint32 i = 0; i < count; ++i)
  {
    alpha = (0 * i) + 10.0 + rg.genrand_res53();
    beta = (0 * i) + 1.5 + (0.5 * rg.genrand_res53());
    alphas.push_back(alpha);
    betas.push_back(beta);
  }
  data[kAlpha] = alphas;
  data[kBeta] = betas;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PrimaryEquiaxedPreset::initializeBOverATableModel(QMap<QString, QVector<float>>& data)
{
  QVector<float>& binNumbers = data[kBinNumbers];
  qint32 count = binNumbers.count();

  float alpha, beta;
  SIMPL_RANDOMNG_NEW()

  QVector<float> alphas;
  QVector<float> betas;

  for(qint32 i = 0; i < count; ++i)
  {
    alpha = (0 * i) + 15.0 + rg.genrand_res53();
    beta = (0 * i) + 1.25 + (0.5 * rg.genrand_res53());
    alphas.push_back(alpha);
    betas.push_back(beta);
  }

  data[kAlpha] = alphas;
  data[kBeta] = betas;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PrimaryEquiaxedPreset::initializeCOverATableModel(QMap<QString, QVector<float>>& data)
{
  QVector<float>& binNumbers = data[kBinNumbers];
  qint32 count = binNumbers.count();

  float alpha, beta;
  SIMPL_RANDOMNG_NEW()

  QVector<float> alphas;
  QVector<float> betas;

  for(qint32 i = 0; i < count; ++i)
  {
    alpha = (0 * i) + 15.0 + rg.genrand_res53();
    beta = (0 * i) + 1.25 + (0.5 * rg.genrand_res53());
    alphas.push_back(alpha);
    betas.push_back(beta);
  }

  data[kAlpha] = alphas;
  data[kBeta] = betas;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PrimaryEquiaxedPreset::initializeNeighborTableModel(QMap<QString, QVector<float>>& data)
{
  QVector<float>& binNumbers = data[kBinNumbers];
  qint32 count = binNumbers.count();

  float mu, sigma;
  SIMPL_RANDOMNG_NEW()

  QVector<float> mus;
  QVector<float> sigmas;

  int middlebin = count / 2;
  for(qint32 i = 0; i < count; ++i)
  {
    mu = log(14.0 + (2.0 * float(i - middlebin)));
    sigma = 0.3 + (float(middlebin - i) / float(middlebin * 10));
    mus.push_back(mu);
    sigmas.push_back(sigma);
  }

  data[kMu] = mus;
  data[kSigma] = sigmas;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PrimaryEquiaxedPreset::initializeClusteringTableModel(QMap<QString, QVector<float>>& data)
{
  Q_ASSERT(false);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
unsigned int PrimaryEquiaxedPreset::getDistributionType(const QString& distType)
{
  if(distType == AbstractMicrostructurePreset::kOmega3Distribution)
  {
    return SIMPL::DistributionType::Beta;
  }
  if(distType == AbstractMicrostructurePreset::kBOverADistribution)
  {
    return SIMPL::DistributionType::Beta;
  }
  if(distType == AbstractMicrostructurePreset::kCOverADistribution)
  {
    return SIMPL::DistributionType::Beta;
  }
  if(distType == AbstractMicrostructurePreset::kClusterDistribution)
  {
    return SIMPL::DistributionType::UnknownDistributionType;
  }
  else if(distType == AbstractMicrostructurePreset::kNeighborDistribution)
  {
    return SIMPL::DistributionType::LogNormal;
  }
  return SIMPL::DistributionType::UnknownDistributionType;
}

// -----------------------------------------------------------------------------
PrimaryEquiaxedPreset::Pointer PrimaryEquiaxedPreset::NullPointer()
{
  return Pointer(static_cast<Self*>(nullptr));
}

// -----------------------------------------------------------------------------
PrimaryEquiaxedPreset::Pointer PrimaryEquiaxedPreset::New()
{
  Pointer sharedPtr(new(PrimaryEquiaxedPreset));
  return sharedPtr;
}
