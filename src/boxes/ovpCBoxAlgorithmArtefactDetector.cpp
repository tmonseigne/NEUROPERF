#include "ovpCBoxAlgorithmArtefactDetector.h"

using namespace OpenViBE;
using namespace Kernel;
using namespace Plugins;

using namespace OpenViBEPlugins;
using namespace NEUROPERF;

//---------------------------------------------------------------------------------------------------
bool CBoxAlgorithmArtefactDetector::initialize()
{
	//***** Codecs *****
	m_iSignalCodec.initialize(*this, 0);
	m_oSignalCodec.initialize(*this, 0);
	m_iMatrix = m_iSignalCodec.getOutputMatrix();
	m_oMatrix = m_oSignalCodec.getInputMatrix();

	//***** Mise a Jour de la matrice reference *****
	m_oMatrix->setDimensionCount(2);									// Mise a jour du nombre de dimensions
	m_oMatrix->setDimensionSize(0, 1);									// Mise a jour du nombre de canaux (un seul)
	m_oMatrix->setDimensionSize(1, 1);									// Taille du morceau de sortie vaut 1 (c'est un booleen)
	m_oMatrix->setDimensionLabel(0, 0, "Artefact");						// Label

	//***** Settings *****
	m_max = FSettingValueAutoCast(*this->getBoxAlgorithmContext(), 0);

	//***** Assert *****
	OV_ERROR_UNLESS_KRF( m_max > 0,
		"Maximum invalide [" << m_max << "] (valeur attendue > 0)\n",
		//"Invalid Maximum [" << m_max << "] (expected value > 0)\n",
		OpenViBE::Kernel::ErrorType::BadSetting);

	return true;
}
//---------------------------------------------------------------------------------------------------

//---------------------------------------------------------------------------------------------------
bool CBoxAlgorithmArtefactDetector::uninitialize()
{
	m_iSignalCodec.uninitialize();
	m_oSignalCodec.uninitialize();
	return true;
}
//---------------------------------------------------------------------------------------------------


//---------------------------------------------------------------------------------------------------
bool CBoxAlgorithmArtefactDetector::processInput(uint32_t inputIndex)
{
	(void)inputIndex;
	getBoxAlgorithmContext()->markAlgorithmAsReadyToProcess();
	return true;
}
//---------------------------------------------------------------------------------------------------

//---------------------------------------------------------------------------------------------------
bool CBoxAlgorithmArtefactDetector::process()
{
	IBoxIO& boxContext = this->getDynamicBoxContext();
	for (uint32_t i = 0; i < boxContext.getInputChunkCount(0); ++i)
	{
		m_iSignalCodec.decode(i);												// Decode le morceau
		OV_ERROR_UNLESS_KRF(m_iMatrix->getDimensionCount() == 2, "Invalid Input Signal", ErrorType::BadInput);
		const uint32_t nChannels = m_iMatrix->getDimensionSize(0),				// Recuperation du nombre de canaux
					   nSamples = m_iMatrix->getDimensionSize(1);				// Recuperation de l'echantillonnage

		if (m_iSignalCodec.isHeaderReceived())									// Entete recu
		{
			const uint64_t iSamplingRate = m_iSignalCodec.getOutputSamplingRate(),	// Recuperation du Taux d'echantillonnage
						   oSamplingRate = uint64_t(ceil(double(iSamplingRate) / double(nSamples)));

			m_oSignalCodec.getInputSamplingRate() = oSamplingRate;
			m_oSignalCodec.encodeHeader();										// Encodage de l'entete
		}
		if (m_iSignalCodec.isBufferReceived())									// Buffer recu
		{
			const double* iBuffer = m_iMatrix->getBuffer();						// Buffer d'entree
			double* oBuffer = m_oMatrix->getBuffer();							// Buffer de Sortie
			oBuffer[0] = 0;														// Initialisation a 0
			for (uint32_t c = 0; c < nChannels; ++c)							// Parcours des valeurs
			{
				for (uint32_t s = 0; s < nSamples; ++s)
				{
					if (abs(iBuffer[c * nSamples + s]) > m_max)					// Detection d'artefact
					{
						oBuffer[0] = 1;
						c = nChannels;
						s = nSamples;											// Sortie de boucle
					}
				}
			}
			m_oSignalCodec.encodeBuffer();										// Encodage du Buffer
		}
		else if (m_iSignalCodec.isEndReceived()) { m_oSignalCodec.encodeEnd(); }	// Fin recu

		const uint64_t tStart = boxContext.getInputChunkStartTime(0, i),		// Time Code Debut de morceau
					   tEnd = boxContext.getInputChunkEndTime(0, i);			// Time Code Fin de morceau
		boxContext.markOutputAsReadyToSend(0, tStart, tEnd);					// Rend la sortie disponible
	}
	return true;
}
//---------------------------------------------------------------------------------------------------
