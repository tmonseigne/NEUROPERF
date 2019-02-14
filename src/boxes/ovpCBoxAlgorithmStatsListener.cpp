#include "ovpCBoxAlgorithmStatsListener.h"

using namespace std;

using namespace OpenViBE;
using namespace Kernel;
using namespace Plugins;

using namespace OpenViBEPlugins;
using namespace NEUROPERF;

//---------------------------------------------------------------------------------------------------
bool CBoxAlgorithmStatsListener::initialize()
{
	//***** Codecs *****
	m_iSignalCodec.initialize(*this, 0);
	m_iStimCodec.initialize(*this, 1);
	m_oMatrixCodec.initialize(*this, 0);
	m_iMatrix = m_iSignalCodec.getOutputMatrix();
	m_oMatrix = m_oMatrixCodec.getInputMatrix();
	m_iStimSet = m_iStimCodec.getOutputStimulationSet();	// Récupération des stimulations

	//***** Mise à Jour de la matrice référence *****
	m_oMatrix->setDimensionCount(2);									// Mise à jour du nombre de dimensions
	m_oMatrix->setDimensionSize(0, 1);									// Mise à jour du nombre de canaux (un seul)
	m_oMatrix->setDimensionSize(1, 4);									// Taille du morceau de sortie
	m_oMatrix->setDimensionLabel(0, 0, "Stats");						// Labels
	double* buffer = m_oMatrix->getBuffer();							// Buffer
	const char* LABELS[4] = { "min", "max", "mean", "std" };
	for (uint32_t i = 0; i < 4; ++i)
	{
		m_oMatrix->setDimensionLabel(1, i, LABELS[i]);					// Labels
		buffer[i] = 0;													// Initialisation à 0
	}
	buffer[0] = 9999;													// Initialisation du min

	return true;
}
//---------------------------------------------------------------------------------------------------

//---------------------------------------------------------------------------------------------------
bool CBoxAlgorithmStatsListener::uninitialize()
{
	//***** Codecs *****
	m_iSignalCodec.uninitialize();
	m_iStimCodec.uninitialize();
	m_oMatrixCodec.uninitialize();

	return true;
}
//---------------------------------------------------------------------------------------------------

//---------------------------------------------------------------------------------------------------
bool CBoxAlgorithmStatsListener::processInput(uint32_t /*inputIndex*/)
{
	getBoxAlgorithmContext()->markAlgorithmAsReadyToProcess();
	return true;
}
//---------------------------------------------------------------------------------------------------

//---------------------------------------------------------------------------------------------------
bool CBoxAlgorithmStatsListener::process()
{
	IBoxIO& boxContext = this->getDynamicBoxContext();

	//***** Stimulations *****
	for (uint32_t i = 0; i < boxContext.getInputChunkCount(1); ++i)
	{
		m_iStimCodec.decode(i);													// Décode la stimulation
		if (m_iStimCodec.isBufferReceived())									// Buffer reçu
		{
			for (uint32_t j = 0; j < m_iStimSet->getStimulationCount(); ++j)
			{
				const uint64_t S = m_iStimSet->getStimulationIdentifier(j);
				if (S == OVTK_StimulationId_BaselineStart) { m_isEnable = true; }
				else if (S == OVTK_StimulationId_BaselineStop) { m_isEnable = false; }
			}
		}
	}

	//***** Signal *****
	for (uint32_t i = 0; i < boxContext.getInputChunkCount(0); ++i)
	{
		m_iSignalCodec.decode(i);												// Décode le morceau
		OV_ERROR_UNLESS_KRF(m_iMatrix->getDimensionCount() == 2 && m_iMatrix->getDimensionSize(0) == 1 && m_iMatrix->getDimensionSize(1) == 1,
			"Invalid Input Signal", ErrorType::BadInput);
		if (m_iSignalCodec.isHeaderReceived()) { m_oMatrixCodec.encodeHeader(); }
		else if (m_iSignalCodec.isBufferReceived())								// Buffer reçu
		{		
			const double value = m_iMatrix->getBuffer()[0];
			if (m_isEnable && value > 0.0)
			{
				double* oBuffer = m_oMatrix->getBuffer();						// Buffer de Sortie
				if (oBuffer[0] > value) { oBuffer[0] = value; }					// Min
				if (oBuffer[1] < value) { oBuffer[1] = value; }					// Max
				m_sum += value;													// Somme
				m_squaredSum += value * value;									// Somme des carrés
				m_chunkNumber += 1;												// Nombre d'echantillons
				oBuffer[2] = m_sum / double(m_chunkNumber);						// Moyenne
				oBuffer[3] = sqrt(m_squaredSum / double(m_chunkNumber) - oBuffer[2] * oBuffer[2]);	// Ecart-Type
			}
			m_oMatrixCodec.encodeBuffer();										// Encodage du Buffer
		}
		else if (m_iSignalCodec.isEndReceived()) { m_oMatrixCodec.encodeEnd(); }	// Fin reçu

		const uint64_t tStart = boxContext.getInputChunkStartTime(0, i),		// Time Code Début de morceau
					   tEnd = boxContext.getInputChunkEndTime(0, i);			// Time Code Fin de morceau
		boxContext.markOutputAsReadyToSend(0, tStart, tEnd);					// Rend la sortie disponible
	}

	return true;
}
//---------------------------------------------------------------------------------------------------
