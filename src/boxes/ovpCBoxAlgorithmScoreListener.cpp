#include "ovpCBoxAlgorithmScoreListener.h"

#include <openvibe/ovITimeArithmetics.h>

using namespace std;

using namespace OpenViBE;
using namespace Kernel;
using namespace Plugins;

using namespace OpenViBEPlugins;
using namespace NEUROPERF;

//---------------------------------------------------------------------------------------------------
bool CBoxAlgorithmScoreListener::initialize()
{
	//***** Codecs *****
	m_iSignalCodec.initialize(*this, 0);
	m_iStimCodec.initialize(*this, 1);
	m_oMatrixCodec.initialize(*this, 0);
	m_oStimulationCodec[0].initialize(*this, 1);	// Traffic Light
	m_oStimulationCodec[1].initialize(*this, 2);	// Reward
	m_iMatrix = m_iSignalCodec.getOutputMatrix();
	m_oMatrix = m_oMatrixCodec.getInputMatrix();
	m_iStimSet = m_iStimCodec.getOutputStimulationSet();	// Récupération des stimulations
	m_prevStimTime[0] = 0;	m_prevStimTime[1] = 0;

	//***** Consts ***** 
	const char* LABELS[5] = { "Super Critiques", "Critiques", "Recompenses", "Super Recompenses", "Score" };

	//***** Mise a Jour de la matrice reference *****
	m_oMatrix->setDimensionCount(2);						// Mise a jour du nombre de dimensions
	m_oMatrix->setDimensionSize(0, 1);						// Mise a jour du nombre de canaux (un seul)
	m_oMatrix->setDimensionSize(1, 5);						// Taille du morceau de sortie
	m_oMatrix->setDimensionLabel(0, 0, "Scores");			// Labels
	double* buffer = m_oMatrix->getBuffer();				// Buffer
	for (uint32_t i = 0; i < 5; ++i)
	{
		m_oMatrix->setDimensionLabel(1, i, LABELS[i]);
		buffer[i] = 0;
	}

	//***** Settings *****
	m_rewardTime = ITimeArithmetics::secondsToTime(double(FSettingValueAutoCast(*this->getBoxAlgorithmContext(), 0)));
	m_chainNeed = FSettingValueAutoCast(*this->getBoxAlgorithmContext(), 1);
	m_thresholds.resize(4);
	m_thresholds[0] = FSettingValueAutoCast(*this->getBoxAlgorithmContext(), 2);
	m_thresholds[1] = FSettingValueAutoCast(*this->getBoxAlgorithmContext(), 3);
	m_thresholds[2] = FSettingValueAutoCast(*this->getBoxAlgorithmContext(), 4);
	m_thresholds[3] = FSettingValueAutoCast(*this->getBoxAlgorithmContext(), 5);
	m_coefScore = m_thresholds[3] - m_thresholds[0];


	return true;
}
//---------------------------------------------------------------------------------------------------

//---------------------------------------------------------------------------------------------------
bool CBoxAlgorithmScoreListener::uninitialize()
{
	//***** Codecs *****
	m_iSignalCodec.uninitialize();
	m_iStimCodec.uninitialize();
	m_oMatrixCodec.uninitialize();
	m_oStimulationCodec[0].uninitialize();
	m_oStimulationCodec[1].uninitialize();

	return true;
}
//---------------------------------------------------------------------------------------------------

//---------------------------------------------------------------------------------------------------
bool CBoxAlgorithmScoreListener::processInput(uint32_t /*inputIndex*/)
{
	getBoxAlgorithmContext()->markAlgorithmAsReadyToProcess();
	return true;
}
//---------------------------------------------------------------------------------------------------

//---------------------------------------------------------------------------------------------------
bool CBoxAlgorithmScoreListener::process()
{
	IBoxIO& boxContext = this->getDynamicBoxContext();
	//***** Stimulations *****
	for (uint32_t i = 0; i < boxContext.getInputChunkCount(1); ++i)
	{
		m_iStimCodec.decode(i);													// Decode la stimulation
		if (m_iStimCodec.isBufferReceived())									// Buffer recu
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
		m_iSignalCodec.decode(i);												// Decode le morceau
		const uint64_t tStart = boxContext.getInputChunkStartTime(0, i),		// Time Code Debut de morceau
					   tEnd = boxContext.getInputChunkEndTime(0, i);			// Time Code Fin de morceau

		OV_ERROR_UNLESS_KRF(m_iMatrix->getDimensionCount() == 2 && m_iMatrix->getDimensionSize(0) == 1 && m_iMatrix->getDimensionSize(1) == 1, 
			"Invalid Input Signal", ErrorType::BadInput);
		if (m_iSignalCodec.isHeaderReceived()) { m_oMatrixCodec.encodeHeader(); }	// Header recu
		else if (m_iSignalCodec.isBufferReceived())									// Buffer recu
		{					
			const double value = m_iMatrix->getBuffer()[0];							// Valeur
			if (m_isEnable)
			{
				double* oBuffer = m_oMatrix->getBuffer();							// Buffer de Sortie
				oBuffer[4] += value - m_thresholds[0] / m_coefScore;				// Score
				int light = -1, reward = 0;
				computeRewards(oBuffer, value, light, reward);
				if (light != -1) { stimSender(boxContext, 0, tStart, uint64_t(light)); }				// Stimulation feu
				if (reward != 0) { stimSender(boxContext, 1, tStart, uint64_t(reward == 1 ? 0 : 1)); }	// Stimulation Artefact et recompense
			}
			m_oMatrixCodec.encodeBuffer();											// Encodage du Buffer
		}
		else if (m_iSignalCodec.isEndReceived()) { m_oMatrixCodec.encodeEnd(); }	// Fin recu

		boxContext.markOutputAsReadyToSend(0, tStart, tEnd);						// Rend la sortie disponible
	}

	return true;
}
//---------------------------------------------------------------------------------------------------

//---------------------------------------------------------------------------------------------------
bool CBoxAlgorithmScoreListener::computeRewards(double* buffer, const double value, int& light, int& reward)
{
	uint32_t state = 0;													// Recuperation de l'etat
	if (value == 0.0) { reward = -1;	state = 2; }					// Position neutre & Artefact
	else if (value < m_thresholds[0]) { state = 0; }					// Position inferieur a min
	else if (value < m_thresholds[1]) { state = 1; }					// Position entre min et critique
	else if (value < m_thresholds[2]) { state = 2; }					// Position neutre
	else if (value < m_thresholds[3]) { state = 3; }					// Position entre recompense et max
	else if (value >= m_thresholds[3]) { state = 4; }					// Position superieur a max

	const uint64_t T = this->getPlayerContext().getCurrentTime();		// Recuperation du temps

	if (state != m_prevState)											// Si on a un changement d'etat
	{
		if ((m_prevState < 2 && 2 < state) || (state < 2 && 2 < m_prevState))	// Si changement (Positif <=> Negatif)
		{
			m_prevTime = T;												// MAJ du temps de reference
			m_rewardChain = 0;											// Reinitialisation de la serie
		}
		m_prevState = state;											// MAJ de l'etat de reference
		light = m_prevState;											// Indication pour la stimulation du Feu
	}
	else if (m_prevState != 2)											// Sinon, Si on n'est pas dans la zone neutre
	{					
		if (T - m_prevTime >= m_rewardTime) { updateRewards(buffer, T, reward); }	// Si Temps ecoule superieur au seuil de recompense
	}
	return true;
}
//---------------------------------------------------------------------------------------------------

//---------------------------------------------------------------------------------------------------
bool CBoxAlgorithmScoreListener::updateRewards(double* buffer, const uint64_t time, int& reward)
{
	m_prevTime = time;						// Remise a zero du temps
	m_rewardChain += 1;						// Incrementation de la chaine de recompense
	if (m_prevState < 2)					// Critique
	{			
		buffer[1] += 1;
		if (m_rewardChain >= m_chainNeed)	// Super Critique
		{	
			buffer[0] += 1;
			m_rewardChain = 0;
		}
	}
	else if (m_prevState > 2)				// Recompense
	{		
		buffer[2] += 1;
		reward = 1;							// Stimulation Recompense
		if (m_rewardChain >= m_chainNeed)	// Super Recompense
		{	
			buffer[3] += 1;
			m_rewardChain = 0;
		}
	}
	return true;
}
//---------------------------------------------------------------------------------------------------

//---------------------------------------------------------------------------------------------------
bool CBoxAlgorithmScoreListener::stimSender(IBoxIO& boxContext, const int index, const uint64_t time, const uint64_t stimCode)
{
	IStimulationSet* stimSet = m_oStimulationCodec[index].getInputStimulationSet();
	stimSet->clear();																// L'encodeur peut garder le precedent buffer
	stimSet->appendStimulation(OVTK_StimulationId_Label_01 + stimCode, time, 0);	// OVTK_StimulationId_Label_[1;5]
	m_oStimulationCodec[index].encodeBuffer();
	boxContext.markOutputAsReadyToSend(index + 1, m_prevStimTime[index], time);		// Rend la sortie disponible
	m_prevStimTime[index] = time;
	return true;
}
//---------------------------------------------------------------------------------------------------
