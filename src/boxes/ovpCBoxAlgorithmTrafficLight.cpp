#include "ovpCBoxAlgorithmTrafficLight.h"

#include <sstream>
#include <iomanip>

#include <system/ovCMemory.h>
#include <visualization-toolkit/ovvizColorGradient.h>

using namespace std;

using namespace OpenViBE;
using namespace Kernel;
using namespace Plugins;

using namespace OpenViBEPlugins;
using namespace NEUROPERF;

/*
static gboolean DrawCallback(GtkWidget* widget, GdkEventConfigure* event, gpointer data)
{
	auto* l_pView = reinterpret_cast <CBoxAlgorithmTrafficLight*>(data);
	l_pView->getLogManager() << LogLevel_Info << "Callback is comming\n";
	if (l_pView->m_widget != nullptr && gtk_widget_get_visible(l_pView->m_widget))
	{
		l_pView->m_needRedraw = true;
		l_pView->draw(l_pView->m_lastStep);
	}
	return true;
}
*/
//---------------------------------------------------------------------------------------------------
bool CBoxAlgorithmTrafficLight::initialize()
{
	//***** Decoder *****
	m_iStimCodec.initialize(*this, 0);
	m_iStimSet = m_iStimCodec.getOutputStimulationSet();	// Recuperation des stimulations

	//***** Settings *****
	const CString colorsSetting = FSettingValueAutoCast(*this->getBoxAlgorithmContext(), 0);
	m_steps = FSettingValueAutoCast(*this->getBoxAlgorithmContext(), 1);

	//***** Assert *****
	OV_ERROR_UNLESS_KRF(m_steps >= 2 && m_steps < 255,
		"Pas de gradient invalide [" << m_steps << "] (valeur attendue >= 2)\n",
		//"Invalid Gradient Step [" << m_steps << "] (expected value >= 2)\n",
		OpenViBE::Kernel::ErrorType::BadSetting
	);

	//***** Palette *****
	CMatrix colorInterpolated, colors;
	OpenViBEVisualizationToolkit::Tools::ColorGradient::parse(colors, colorsSetting);
	OpenViBEVisualizationToolkit::Tools::ColorGradient::interpolate(colorInterpolated, colors, uint32_t(m_steps));
	m_lights.resize(m_steps);
	for (size_t i = 0; i < m_steps; ++i)
	{
		const uint32_t id = uint32_t(i) * 4;													// Identifiant du debut de la couleur
		m_lights[i].m_Color.red = guint16(colorInterpolated[id + 1] * 655.35);		// Interpolation de 0 a 100 ramene de 0 a 65 535
		m_lights[i].m_Color.green = guint16(colorInterpolated[id + 2] * 655.35);
		m_lights[i].m_Color.blue = guint16(colorInterpolated[id + 3] * 655.35);
		m_lights[i].m_A1 = 0;														// Angle de depart
		m_lights[i].m_A2 = 23040;													// Angle total (360 * 64 = 23040)
	}
	m_back.m_Color = COLOR_LIGHTGREY;

	//***** Widget *****
	m_widget = GTK_WIDGET(gtk_drawing_area_new());
	gtk_widget_set_double_buffered(m_widget, TRUE);
	gtk_widget_show(m_widget);

	// Force le redraw en cas de Scale...... pas comme ca a l'evidence....
	//g_signal_connect(G_OBJECT(m_widget), "configure-event", G_CALLBACK(DrawCallback), this);

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
bool CBoxAlgorithmTrafficLight::uninitialize()
{
	m_iStimCodec.uninitialize();				// Decodeurs
	m_lights.clear();							// Variables
	this->releasePluginObject(m_vizuContext);	// Widgets
	return true;
}
//---------------------------------------------------------------------------------------------------

//---------------------------------------------------------------------------------------------------
bool CBoxAlgorithmTrafficLight::processInput(uint32_t inputIndex)
{
	(void)inputIndex;
	getBoxAlgorithmContext()->markAlgorithmAsReadyToProcess();
	return true;
}

//---------------------------------------------------------------------------------------------------
bool CBoxAlgorithmTrafficLight::processClock(CMessageClock& rMessageClock)
{
	// Lance le process une fois par seconde
	(void)rMessageClock;
	m_needRedraw = true;
	getBoxAlgorithmContext()->markAlgorithmAsReadyToProcess();
	return true;
}
//---------------------------------------------------------------------------------------------------

//---------------------------------------------------------------------------------------------------
uint64_t CBoxAlgorithmTrafficLight::getClockFrequency()
{
	// Note that the time is coded on a 64 bits unsigned integer, fixed decimal point (32:32)
	return 1LL << 32; // the box clock frequency	
}
//---------------------------------------------------------------------------------------------------

//---------------------------------------------------------------------------------------------------
bool CBoxAlgorithmTrafficLight::process()
{
	if (m_widget != nullptr && (gtk_widget_get_visible(m_widget) != 0))
	{
		size_t step = m_lastStep;
		IBoxIO& boxContext = this->getDynamicBoxContext();
		for (size_t i = 0; i < boxContext.getInputChunkCount(0); ++i)
		{
			m_iStimCodec.decode(uint32_t(i));										// Decodage du morceau
			if (m_iStimCodec.isBufferReceived())									// Buffer recu
			{
				IStimulationSet* stims = m_iStimCodec.getOutputStimulationSet();	// Recuperation des stimulations
				for (size_t j = 0; j < stims->getStimulationCount(); ++j)			// Parcours des Stimulations
				{
					const uint64_t S = GETVALID(stims->getStimulationIdentifier(j), OVTK_StimulationId_Label_01, OVTK_StimulationId_Label_FF);
					step = GETVALID(size_t(S - OVTK_StimulationId_Label_01), 0, m_steps);
				}
			}
		}
		draw(step);										// Trace des Lumieres ou redraw si necessaire
	}
	else { m_needRedraw = true; }					// Si on est pas passé ici, on force le draw pour la prochaine fois
	return true;
}
//---------------------------------------------------------------------------------------------------

//---------------------------------------------------------------------------------------------------
bool CBoxAlgorithmTrafficLight::computeSize()
{
	//***** Si aucun changement, on ne va pas plus loin *****
	if (m_widget->allocation.width == m_windowW && m_widget->allocation.height == m_windowH) { return true; }

	//***** Recuperation de la Largeur et de la Hauteur *****
	m_windowW = m_widget->allocation.width;
	m_windowH = m_widget->allocation.height;
	OV_ERROR_UNLESS_KRF(m_windowW > 0 && m_windowH > 0, "Taille de Fenêtre invalide", ErrorType::BadProcessing);

	//***** Precalculs *****
	const gint margin = gint(0.025 * MIN(m_windowW, m_windowH)),	// Marge
			   margin2 = 2 * margin,								// Precalcul d'une double marge
			   W = m_windowW - margin2, H = m_windowH - margin2,	// Dimension de dessin max (exterieur du feu)
			   w = W - margin2, h = H - margin2;					// Dimension de dessin max (interieur du feu)
	gint D = MIN(gint((h - (m_steps - 1)*margin) / m_steps), w);	// Diametre des lumieres
	if (D % 2 == 1) { D -= 1; }										// Diametre Pair (pour eviter les problemes d'arrondi

	//***** Dimension et position du rectangle de fond des lumieres (centre) *****
	m_back.m_W = D + margin2;
	m_back.m_H = gint(m_steps) * (D + margin) + margin;
	m_back.m_X = (m_windowW - m_back.m_W) / 2;
	m_back.m_Y = (m_windowH - m_back.m_H) / 2;

	//***** Dimension et position des Lumieres (parcours negatifs car repere Image) *****
	const gint stepY = margin + D;
	gint lY = m_back.m_Y + margin;
	for (int i = int(m_lights.size() - 1); i >= 0; --i)
	{
		m_lights[i].m_X = m_back.m_X + margin;
		m_lights[i].m_Y = lY;
		m_lights[i].m_D = D;
		lY += stepY;
	}
	m_needRedraw = true;
	return true;
}
//---------------------------------------------------------------------------------------------------

//---------------------------------------------------------------------------------------------------
bool CBoxAlgorithmTrafficLight::draw(const size_t step)
{
	OV_ERROR_UNLESS_KRF(computeSize(), "Calcul des dimensions invalide", ErrorType::BadProcessing);
	GdkGC* gc = m_widget->style->fg_gc[gtk_widget_get_state(m_widget)];			// Recuperation de l'environnement
	if (m_needRedraw)
	{
		gdk_gc_set_rgb_fg_color(gc, &COLOR_BLACK);								// (normalement la couleur est deja noire mais parfois non)
		gdk_draw_rectangle(m_widget->window, gc, TRUE, 0, 0, m_windowW, m_windowH);	// Reinitialisation
		drawBackground(gc);														// Trace du fond
		drawLights(gc, step);													// Trace des Lumieres
		gdk_gc_set_rgb_fg_color(gc, &COLOR_BLACK);								// Remise a noir de la couleur
		m_needRedraw = false;
	}
	else if (m_lastStep != step) { drawLights(gc, step); }						// Modification des lumieres uniquement si necessaire
	m_lastStep = step;															// Mise a jour de la reference
	return true;
}
//---------------------------------------------------------------------------------------------------

//---------------------------------------------------------------------------------------------------
bool CBoxAlgorithmTrafficLight::drawBackground(GdkGC* gc)
{
	const gint radius = m_back.m_W / 2,
			   Y1 = m_back.m_Y + radius,
			   Y2 = m_back.m_Y + m_back.m_H - radius;

	gdk_gc_set_rgb_fg_color(gc, &m_back.m_Color);
	gdk_draw_rectangle(m_widget->window, gc, 1, m_back.m_X, Y1 - 1, m_back.m_W, m_back.m_H - 2 * radius + 2);
	gdk_draw_arc(m_widget->window, gc, 1, m_back.m_X, m_back.m_Y, m_back.m_W, m_back.m_W, 0, 11520);	// 180 * 64 = 11 520
	gdk_draw_arc(m_widget->window, gc, 1, m_back.m_X, Y2 - radius, m_back.m_W, m_back.m_W, 11520, 11520);
	return true;
}
//---------------------------------------------------------------------------------------------------

//---------------------------------------------------------------------------------------------------
bool CBoxAlgorithmTrafficLight::drawLights(GdkGC* gc, const size_t step)
{
	for (size_t i = 0; i < m_lights.size(); ++i) { drawLight(gc, m_lights[i], i == step); }
	gdk_gc_set_rgb_fg_color(gc, &COLOR_BLACK);				// Remise a noir de la couleur
	return true;
}
//---------------------------------------------------------------------------------------------------

//---------------------------------------------------------------------------------------------------
bool CBoxAlgorithmTrafficLight::drawLight(GdkGC* gc, const SCircle& light, const bool fill)
{
	gdk_gc_set_rgb_fg_color(gc, fill ? &light.m_Color : &COLOR_WHITE);			// Affectation de la couleur correspondante
	gdk_draw_arc(m_widget->window, gc, 1, light.m_X, light.m_Y, light.m_D, light.m_D, light.m_A1, light.m_A2);
	gdk_gc_set_rgb_fg_color(gc, &COLOR_BLACK);									// Contour
	gdk_draw_arc(m_widget->window, gc, 0, light.m_X, light.m_Y, light.m_D, light.m_D, light.m_A1, light.m_A2);
	return true;
}
//---------------------------------------------------------------------------------------------------
