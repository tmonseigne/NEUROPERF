#include "ovpCBoxAlgorithmNeuroperfDisplay.h"

#include <string>
#include <sstream>
#include <iomanip>

#include <openvibe/ovITimeArithmetics.h>
#include <system/ovCMemory.h>
#include <visualization-toolkit/ovvizColorGradient.h>

using namespace std;

using namespace OpenViBE;
using namespace Kernel;
using namespace Plugins;

using namespace OpenViBEPlugins;
using namespace NEUROPERF;

//---------------------------------------------------------------------------------------------------
bool CBoxAlgorithmNeuroperfDisplay::initialize()
{
	//***** Decoder *****
	m_iMatrixCodec.initialize(*this, 0);
	m_i1StimCodec.initialize(*this, 1);
	m_i2StimCodec.initialize(*this, 2);
	m_iMatrix = m_iMatrixCodec.getOutputMatrix();
	m_iStimSet = m_i1StimCodec.getOutputStimulationSet();

	//***** Settings *****
	const CString colorsSetting = FSettingValueAutoCast(*this->getBoxAlgorithmContext(), 0);
	m_gradientSteps = FSettingValueAutoCast(*this->getBoxAlgorithmContext(), 1);
	m_min = FSettingValueAutoCast(*this->getBoxAlgorithmContext(), 2);
	m_max = FSettingValueAutoCast(*this->getBoxAlgorithmContext(), 3);
	m_time = ITimeArithmetics::secondsToTime(double(FSettingValueAutoCast(*this->getBoxAlgorithmContext(), 4)));

	//***** Assert *****
	OV_ERROR_UNLESS_KRF(m_gradientSteps >= 2,
		"Pas de gradient invalide [" << m_gradientSteps << "] (valeur attendue >= 2)\n",
		//"Invalid Gradient Step [" << m_gradientSteps << "] (expected value >= 2)\n",
		OpenViBE::Kernel::ErrorType::BadSetting
	);
	OV_ERROR_UNLESS_KRF(m_min != m_max,
		"Minimum et Maximum doivent etre differents [Min : " << m_min << ", Max : " << m_max << "]\n",
		//"Invalid Min/Max [Min : " << m_min << ", Max : "<< m_max << "]\n",
		OpenViBE::Kernel::ErrorType::BadSetting
	);

	if (m_min > m_max)
	{
		swap(m_min, m_max);
		this->getLogManager() << LogLevel_Warning << "Min et Max intervertis\n";
	}


	//***** Palette *****
	CMatrix colorsInterpolated, colors;
	OpenViBEVisualizationToolkit::Tools::ColorGradient::parse(colors, colorsSetting);
	OpenViBEVisualizationToolkit::Tools::ColorGradient::interpolate(colorsInterpolated, colors, uint32_t(m_gradientSteps));
	m_cBar.resize(m_gradientSteps);
	for (uint32_t i = 0; i < m_gradientSteps; ++i)
	{
		const uint32_t id = i * 4;										// Identifiant du debut de la couleur
		m_cBar[i].red = guint16(colorsInterpolated[id + 1] * 655.35);	// Interpolation de 0 a 100 ramene de 0 a 65 535
		m_cBar[i].green = guint16(colorsInterpolated[id + 2] * 655.35);
		m_cBar[i].blue = guint16(colorsInterpolated[id + 3] * 655.35);
	}

	//***** Widget *****
	m_widget = GTK_WIDGET(gtk_drawing_area_new());
	gtk_widget_set_double_buffered(m_widget, TRUE);
	gtk_widget_set_size_request(m_widget, 1280, 720);	// Taille de fenetre si non plein ecran
	gtk_widget_show(m_widget);

	OV_ERROR_UNLESS_KRF(this->canCreatePluginObject(OVP_ClassId_Plugin_VisualizationContext),
		"Le framework de visualisation n\'est pas charge\n",
		//"Visualization framework is not loaded\n,
		OpenViBE::Kernel::ErrorType::BadProcessing
	);

	m_vizuContext = dynamic_cast<OpenViBEVisualizationToolkit::IVisualizationContext*>(this->createPluginObject(OVP_ClassId_Plugin_VisualizationContext));
	m_vizuContext->setWidget(*this, m_widget);
	return true;
}
//---------------------------------------------------------------------------------------------------

//---------------------------------------------------------------------------------------------------
bool CBoxAlgorithmNeuroperfDisplay::uninitialize()
{
	//***** Decodeurs *****
	m_iMatrixCodec.uninitialize();
	m_i1StimCodec.uninitialize();
	m_i2StimCodec.uninitialize();

	//***** Variables *****
	m_cBar.clear();
	m_rewards.clear();

	//***** Widgets *****
	this->releasePluginObject(m_vizuContext);
	return true;
}
//---------------------------------------------------------------------------------------------------

//---------------------------------------------------------------------------------------------------
bool CBoxAlgorithmNeuroperfDisplay::processInput(uint32_t /*inputIndex*/)
{
	getBoxAlgorithmContext()->markAlgorithmAsReadyToProcess();
	return true;
}
//---------------------------------------------------------------------------------------------------

//---------------------------------------------------------------------------------------------------
bool CBoxAlgorithmNeuroperfDisplay::process()
{
	IBoxIO& boxContext = this->getDynamicBoxContext();

	//***** Lecture des Stimulations *****
	// Stimulations d'evenement
	for (uint32_t i = 0; i < boxContext.getInputChunkCount(1); ++i)
	{
		m_i1StimCodec.decode(i);								// Decodage du morceau
		if (m_i1StimCodec.isBufferReceived())					// Buffer recu
		{
			const uint64_t T = boxContext.getInputChunkStartTime(1, i) - m_startTime;	// Recuperation du temps
			for (uint32_t j = 0; j < m_iStimSet->getStimulationCount(); ++j)
			{
				//***** Ajout de l'item Recompense/Artefact *****
				const uint64_t S = m_iStimSet->getStimulationIdentifier(j);
				if (S == OVTK_StimulationId_Label_01) { m_rewards.emplace_back(true, GETVALID(T, uint64_t(0), m_time)); }		// Recompense
				else if (S == OVTK_StimulationId_Label_02) { m_rewards.emplace_back(false, GETVALID(T, uint64_t(0), m_time)); }	// Artefact
			}
		}
	}

	// Stimulations de depart
	for (uint32_t i = 0; i < boxContext.getInputChunkCount(2); ++i)
	{
		m_i2StimCodec.decode(i);								// Decodage du morceau
		if (m_i2StimCodec.isBufferReceived())					// Buffer recu
		{
			IStimulationSet* stimSet = m_i2StimCodec.getOutputStimulationSet();
			for (uint32_t j = 0; j < stimSet->getStimulationCount(); ++j)
			{
				if (stimSet->getStimulationIdentifier(j) == OVTK_StimulationId_BaselineStart)
				{
					m_startTime = boxContext.getInputChunkStartTime(2, i);
				}
			}
		}
	}


	//***** Lecture des morceaux *****
	for (uint32_t i = 0; i < boxContext.getInputChunkCount(0); ++i)
	{
		m_iMatrixCodec.decode(i);								// Decodage du morceau
		OV_ERROR_UNLESS_KRF(m_iMatrix->getDimensionCount() == 2 && m_iMatrix->getDimensionSize(0) == 1 && m_iMatrix->getDimensionSize(1) == 1,
			"Invalid Input Signal", ErrorType::BadInput);

		if (m_iMatrixCodec.isBufferReceived())					// Buffer recu
		{		
			const uint64_t T = boxContext.getInputChunkStartTime(0, i) - m_startTime;	// Recuperation du temps
			const double value = m_iMatrix->getBuffer()[i];		// Recuperation de la valeur
			const size_t step = value == 0.0					// Positionnement sur l'echelle
									? m_lastBarStep
									: size_t(((value - m_min) / (m_max - m_min)) * (m_gradientSteps - 1));
			draw(GETVALID(step, size_t(0), m_gradientSteps - 1), GETVALID(T, uint64_t(0), m_time));	// Dessin sur des valeurs valides
		}
	}
	return true;
}
//---------------------------------------------------------------------------------------------------

//---------------------------------------------------------------------------------------------------
bool CBoxAlgorithmNeuroperfDisplay::computeSize()
{
	if (m_widget != nullptr && GTK_WIDGET_VISIBLE(m_widget))	// Environnement existant
	{
		//***** Si aucun changement, on ne va pas plus loin *****
		if (m_widget->allocation.width == m_windowW && m_widget->allocation.height == m_windowH) { return true; }

		//***** Recuperation de la Largeur et de la Hauteur *****
		m_windowW = m_widget->allocation.width;
		m_windowH = m_widget->allocation.height;
		OV_ERROR_UNLESS_KRF(m_windowW > 0 && m_windowH > 0, "Taille de Fenêtre invalide", ErrorType::BadProcessing);
		//***** Precalculs *****
		const gint margin = gint(0.01 * MIN(m_windowW, m_windowH)),						// Marge
				   W = gint(m_windowW) - 2 * margin, H = gint(m_windowH) - 2 * margin,	// Dimension de dessin max
				   w = gint(0.015 * m_windowW), h = gint(0.02 * m_windowH);				// Dimension de dessin min

		//***** Palette en haut a droite *****
		m_rBar.m_X = m_windowW - w - margin;
		m_rBar.m_Y = margin;
		m_rBar.m_W = w;
		m_rBar.m_H = H - h - margin;

		//***** Temps en bas *****
		m_rTime.m_X = margin;
		m_rTime.m_Y = m_windowH - h - margin;
		m_rTime.m_W = W;
		m_rTime.m_H = h;

		//***** Couleur en haut a gauche *****
		m_rColor.m_X = margin;
		m_rColor.m_Y = margin;
		m_rColor.m_W = W - w - margin;
		m_rColor.m_H = H - h - margin;

		GdkGC* gc = m_widget->style->fg_gc[GTK_WIDGET_STATE(m_widget)];		// Recuperation de l'environnement

		//***** Reinitialisation de la base (Remise a 0, Palette, Temps) *****
		gdk_gc_set_rgb_fg_color(gc, &COLOR_WHITE);
		gdk_draw_rectangle(m_widget->window, gc, TRUE, 0, 0, m_windowW, m_windowH);

		drawPalette(gc);
		drawTimer(gc);
	}
	return true;
}
//---------------------------------------------------------------------------------------------------

//---------------------------------------------------------------------------------------------------
bool CBoxAlgorithmNeuroperfDisplay::drawRewards(GdkGC* gc, const size_t number)
{
	const size_t begin = (number == 0) ? 0 : MAX(0, m_rewards.size() - number);
	for (size_t i = begin; i < m_rewards.size(); ++i)
	{
		m_rewards[i].first ? gdk_gc_set_rgb_fg_color(gc, &COLOR_GREEN) : gdk_gc_set_rgb_fg_color(gc, &COLOR_RED);	// Couleur Recompense / Artefact
		gdk_draw_rectangle(m_widget->window, gc, TRUE,
						   gint(m_rTime.m_X + m_rewards[i].second * m_fTimeStep), m_rTime.m_Y,
						   m_rewardTime, m_rTime.m_H);
	}
	return true;
}
//---------------------------------------------------------------------------------------------------

//---------------------------------------------------------------------------------------------------
bool CBoxAlgorithmNeuroperfDisplay::drawTimer(GdkGC* gc)
{
	m_fTimeStep = float(1.0 * m_rTime.m_W / m_time);			// Decoupe sur la largeur
	m_iTimeStep = MAX(1, gint(m_fTimeStep));					// Max pour eviter les epaisseurs de 0 du a la troncature 
	m_rewardTime = MAX(1, gint(ITimeArithmetics::secondsToTime(0.5) * m_fTimeStep));	// Max pour eviter les epaisseurs de 0 du a la troncature 

	const auto pos = gint(m_lastTimeStep * m_fTimeStep);		// Position actuelle
	gdk_gc_set_rgb_fg_color(gc, &COLOR_WHITE);					// Partie Blanche
	gdk_draw_rectangle(m_widget->window, gc, TRUE, m_rTime.m_X + pos, m_rTime.m_Y, m_rTime.m_W - pos, m_rTime.m_H);
	gdk_gc_set_rgb_fg_color(gc, &COLOR_BLUE);					// Partie Bleue
	gdk_draw_rectangle(m_widget->window, gc, TRUE, m_rTime.m_X, m_rTime.m_Y, pos + 1, m_rTime.m_H);
	drawRewards(gc);											// Trace des recompenses

	gdk_gc_set_rgb_fg_color(gc, &COLOR_BLACK);					// Bordure
	gdk_draw_rectangle(m_widget->window, gc, FALSE, m_rTime.m_X - 1, m_rTime.m_Y - 1, m_rTime.m_W + 2, m_rTime.m_H + 2);
	return true;
}
//---------------------------------------------------------------------------------------------------

//---------------------------------------------------------------------------------------------------
bool CBoxAlgorithmNeuroperfDisplay::drawPalette(GdkGC* gc)
{
	m_fBarStep = float(1.0 * m_rBar.m_H / m_gradientSteps);		// Decoupe sur la hauteur
	m_iBarStep = gint(m_fBarStep) + 1;							// +1 pour eviter les blancs du a la troncatures 

	auto Y = float(m_rBar.m_Y);									// Position sur Y du rectangle a dessiner
	for (int i = int(m_gradientSteps) - 1; i >= 0; --i)			// Parcours du haut vers le bas (clair au fonce)
	{	
		gdk_gc_set_rgb_fg_color(gc, &m_cBar[i]);				// Affectation de la couleur
		gdk_draw_rectangle(m_widget->window, gc, TRUE, m_rBar.m_X, gint(Y), m_rBar.m_W, m_iBarStep);
		Y += m_fBarStep;										// Incrementation de la position sur Y
	}
	return true;
}
//---------------------------------------------------------------------------------------------------

//---------------------------------------------------------------------------------------------------
bool CBoxAlgorithmNeuroperfDisplay::drawProgress(GdkGC* gc, const size_t step, const uint64_t time)
{
	//***** Precalculs *****
	const gint baseY = m_rBar.m_Y + m_rBar.m_H - m_iBarStep,	// Position sur Y Initiale de la barre
			   cY = gint(baseY - step * m_fBarStep),			// Position sur Y actuelle de la barre
			   lY = gint(baseY - m_lastBarStep * m_fBarStep),	// Position sur Y precedente de la barre
			   X = m_rBar.m_X - m_iBarStep,						// Position sur X de la barre
			   W = m_rBar.m_W + 2 * m_iBarStep;					// Largeur de la barre

	gdk_gc_set_rgb_fg_color(gc, &COLOR_WHITE);
	gdk_draw_rectangle(m_widget->window, gc, TRUE, X, lY - 1, W, m_iBarStep + 2);	// Effacement de la precedente barre + margin (arrondi)

	const int limitMin = int(MAX(0, m_lastBarStep - 1)),
			  limitMax = int(MIN(m_gradientSteps - 1, m_lastBarStep + 1));
	float Y = baseY - limitMax * m_fBarStep;
	for (int i = limitMax; i >= limitMin; --i)					// Reimpression de la Palette a la position precedente [-1;+1]
	{
		gdk_gc_set_rgb_fg_color(gc, &m_cBar[i]);
		gdk_draw_rectangle(m_widget->window, gc, TRUE, m_rBar.m_X, uint32_t(Y), m_rBar.m_W, m_iBarStep);
		Y += m_fBarStep;
	}
	gdk_gc_set_rgb_fg_color(gc, &COLOR_BLUE);
	gdk_draw_rectangle(m_widget->window, gc, TRUE, X, cY, W, m_iBarStep);	// Dessin de la Barre sur la palette
	gdk_draw_rectangle(m_widget->window, gc, TRUE, uint32_t(m_rTime.m_X + time * m_fTimeStep),
					   m_rTime.m_Y, m_iTimeStep, m_rTime.m_H);	// Dessin de la Barre sur le temps
	gdk_gc_set_rgb_fg_color(gc, &COLOR_BLACK);					// Remise a Noir de la couleur d'ecriture

	m_lastBarStep = step;										// Mise a jour de la valeur precedente
	m_lastTimeStep = time;										// Mise a jour du temps precedent
	return true;
}
//---------------------------------------------------------------------------------------------------

//---------------------------------------------------------------------------------------------------
bool CBoxAlgorithmNeuroperfDisplay::draw(const size_t step, const uint64_t time)
{
	if (m_widget != nullptr && GTK_WIDGET_VISIBLE(m_widget))
	{
		OV_ERROR_UNLESS_KRF(computeSize(), "Calcul des dimensions invalide", ErrorType::BadProcessing);
		GdkGC* gc = m_widget->style->fg_gc[GTK_WIDGET_STATE(m_widget)];		// Recuperation de l'environnement
		if (m_needRedraw)
		{
			gdk_gc_set_rgb_fg_color(gc, &COLOR_WHITE);
			gdk_draw_rectangle(m_widget->window, gc, TRUE, 0, 0, m_windowW, m_windowH);	// Reinitialisation
			drawPalette(gc);
			drawTimer(gc);
			m_needRedraw = false;
		}
		if (step != m_lastBarStep)
		{
			gdk_gc_set_rgb_fg_color(gc, &m_cBar[step]);			// Affectation de la couleur correspondante a la valeur
			gdk_draw_rectangle(m_widget->window, gc, TRUE, m_rColor.m_X, m_rColor.m_Y, m_rColor.m_W, m_rColor.m_H);
		}
		drawProgress(gc, step, time);							// Barre de progression
		drawRewards(gc, 10);									// Afficher juste le dernier ne fonctionne pas parfaitement...

		gdk_gc_set_rgb_fg_color(gc, &COLOR_BLACK);				// Remise a Noir de la couleur d'ecriture
	}
	return true;
}
//---------------------------------------------------------------------------------------------------
