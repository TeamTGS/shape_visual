/*=========================================================================
  Program: MILX MixView
  Module: itkIncrementalPCAModelEstimator.h
  Author: Jurgen Fripp
  Modified by: King Fai Ho
  Language: C++
  Created: Fri 09 March 2007 16:21:00 EST

  Copyright: (c) 2009 CSIRO, Australia.

  This software is protected by international copyright laws.
  Any unauthorised copying, distribution or reverse engineering is prohibited.

  Licence:
  All rights in this Software are reserved to CSIRO. You are only permitted
  to have this Software in your possession and to make use of it if you have
  agreed to a Software License with CSIRO.

  BioMedIA Lab: http://www.ict.csiro.au/BioMedIA/
=========================================================================*/
#ifndef __itkIncrementalPCAModelEstimator_h
#define __itkIncrementalPCAModelEstimator_h

#include "itkPCAModelEstimator.h"

#include <fstream>
#include <iostream>
#include <numeric>
#include <vector>
#include <iterator>
#include <functional>

namespace itk
{

/** \class PCAModelEstimator
 * \brief Base class for PCAModelEstimator object
 *
 * Perform user-defined batch size PCA and pass on the eigenmodel to perform Incremental PCA
 *
 * The Update() function enables the calculation of the various models, creates
 * the membership function objects and populates them.
 *
 * \ingroup Algorithms 
 
 This work is implemented from:
 Skocaj, D.; Leonardis, A. Incremental and robust learning of subspace representations., 2008, 26, 27-38
 
 */



template <class TPrecisionType=double >
class ITK_EXPORT IncrementalPCAModelEstimator : public PCAModelEstimator<TPrecisionType>
{
public:
  /** Standard class typedefs. */
  typedef IncrementalPCAModelEstimator			Self;
  typedef PCAModelEstimator<TPrecisionType>		Superclass;
  typedef SmartPointer<Self>					Pointer;
  typedef SmartPointer<const Self>				ConstPointer;


  /** Method for creation through the object factory. */
  itkNewMacro(Self);

  /** Run-time type information (and related methods). */
  itkTypeMacro(IncrementalPCAModelEstimator, Object);

  /** Type definition for matrix. */
  typedef vnl_matrix<TPrecisionType> MatrixType;

  /** Type definition for vector. */
  typedef vnl_vector<TPrecisionType> VectorType;

  /** Set/Get the number of required largest principal components. The filter produces
   the required number of principal components plus one outputs. Output index 0 represents
   the mean image and the remaining outputs the requested principal components.*/
  //virtual void SetNumberOfPrincipalComponentsRequired( unsigned int n );
  itkGetMacro( NumberOfPrincipalComponentsRequired, unsigned int );

  /** Add a training set. */
  virtual void AddTrainingSet(VectorType trainingSet);
  /** Get a training set. */
  virtual void GetTrainingSet(int index, vnl_vector<TPrecisionType> &vector);
  /** Clear a training set. */
  virtual void ClearTrainingSets();

  /** Set batch size. */
  virtual void setPCABatchSize(int batchSize);

  /** Set precision. */
  virtual void setPrecision(double precision);

  /** Set eigenvalue control size. */
  virtual void setEigenvalueSizeControl(int eigenvalueSizeControl);

  /** Set/Get the number of measures in the input. */
  itkSetMacro(NumberOfMeasures, unsigned int);
  itkGetMacro(NumberOfMeasures, unsigned int);

  /** Get the number of training set in the input. */
  itkGetMacro(NumberOfTrainingSets, unsigned int);

  /** Get the eigen values */
  itkGetMacro(EigenValues, VectorType);

  /** Get the eigen vectors */
  itkGetMacro(EigenVectors, MatrixType);

  /** Get the mean values */
  itkGetMacro(Means, VectorType);

  void Update();

  /** Get training matrix */
  void GetTrainingMatrix(vnl_matrix<TPrecisionType> &matrix);
  /** Save training matrix */
  void SaveTrainingMatrix(const char *);
  /** Get training vector */
  void SaveVector(const char *, vnl_vector<int> vector);

  /**
   * Get parameters as a vector array
   */
  void GetDecomposition(vnl_vector<TPrecisionType> vector, vnl_vector<TPrecisionType> &decomposition);

  /**
   * Return vector from parameters as a vector array
   */
  void GetVectorFromDecomposition(vnl_vector<TPrecisionType> decomposition, vnl_vector<TPrecisionType> &reconstruction, int numberOfModes = -1);
  void GetVectorFromNormalizedDecomposition(vnl_vector<TPrecisionType> decomposition, vnl_vector<TPrecisionType> &reconstruction, int numberOfModes = -1);

  /**
   * Get the number of modes required to explain threshold% of the variance
   *  - 0 <= threshold <= 1
   */
  int GetNumberOfModesRequired(float threshold)
    {
    //this->Update();
    float sumEig = threshold*m_EigenValues.sum();
    float value = 0;
    for(unsigned int i = 0; i < m_EigenValues.size()-1; i++)
      {
      value += m_EigenValues(i);
      if(value >  sumEig)
        {
        return i+1;
        }
      }
    return m_EigenValues.size();
    }

  /**
   * Normalize Modes
   */
  void GetNormalizeModes(vnl_vector<TPrecisionType> &decomposition)
    {
    this->Update();
    for(unsigned int i = 0; i < decomposition.size(); i++)
      {
      decomposition(i) = decomposition(i)/sqrt(m_EigenValues(i));
      }
    }

  template<class TPixel, unsigned Dim>
  itk::SmartPointer< itk::Image<TPixel, Dim> > VectorToImage(VectorType &vec, typename itk::Image<TPixel, Dim>::SizeType size, itk::SmartPointer< itk::Image<TPixel, Dim> > image);

protected:
  IncrementalPCAModelEstimator();
  ~IncrementalPCAModelEstimator();
  virtual void PrintSelf(std::ostream& os, Indent indent) const;

  /** Starts the modelling process */
  virtual void GenerateData();

  /**
   * Set/Get the Valid Data Bit for the class
   *  - Used by Update
   */
  itkSetMacro( Valid, bool );
  itkGetMacro( Valid, bool );

  std::vector<VectorType > m_TrainingSets;

  unsigned int  m_NumberOfMeasures;

  int m_batchSize;
  double m_Precision;
  int m_eigenvalueSizeControl;

  /** Local storage variables */
  VectorType    m_Means;
  MatrixType    m_EigenVectors;
  VectorType    m_EigenValues;
  MatrixType	m_A;

  /** The number of input training sets for PCA */
  unsigned int  m_NumberOfTrainingSets;
  /** The number of output Pricipal Components */
  unsigned int  m_NumberOfPrincipalComponentsRequired;
  bool          m_Valid;

private:
  IncrementalPCAModelEstimator(const Self&); //purposely not implemented
  void operator=(const Self&); //purposely not implemented

  /** Local functions */
  virtual void EstimateModels();
  /** Batch PCA functions */
  void EstimatePCAModelParameters();
  /** IPCA functions */
  void IPCAModelParameters();
  /** PCA functions for Batch */
  void ApplyStandardPCA(const MatrixType &data, MatrixType &eigenVecs, VectorType &eigenVals);
  /** PCA functions for IPCA */
  void ApplyStandardPCA2(const MatrixType &data, MatrixType &eigenVecs, VectorType &eigenVals);

  TPrecisionType Reconstruct(MatrixType &recon, const MatrixType &data, const MatrixType &eigenVecs, const MatrixType &coefficients, const MatrixType &means);
}; // class IncrementalPCAModelEstimator


} // namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkIncrementalPCAModelEstimator.txx"
#endif

#endif
