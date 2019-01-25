#pragma once


#include "ovp_defines.h"
#include <openvibe/ov_all.h>
#include <toolkit/ovtk_all.h>
#include <visualization-toolkit/ovviz_all.h>
#include <gtk/gtk.h>
#include <map>
#include <string>

#define OVP_ClassId_BoxAlgorithm_NeuroperfDisplay (0x6776e9b6, 0x8366842c)
#define OVP_ClassId_BoxAlgorithm_NeuroperfDisplayDesc (0x47f17bf4, 0x540d6de3)
#define OV_AttributeId_Box_FlagIsUnstable OpenViBE::CIdentifier(0x666FFFFF, 0x666FFFFF)


namespace OpenViBEPlugins
{
	namespace NEUROPERF
	{
		/// <summary>	 The class CBoxAlgorithmNeuroperfDisplay describes the box Neuroperf Display. </summary>
		class CBoxAlgorithmNeuroperfDisplay : virtual public OpenViBEToolkit::TBoxAlgorithm<OpenViBE::Plugins::IBoxAlgorithm>
		{
		public:
			void release() override { delete this; }

			bool initialize() override;
			bool uninitialize() override;
			bool processInput(uint32_t inputIndex) override;
			bool process() override;

			_IsDerivedFromClass_Final_(OpenViBEToolkit::TBoxAlgorithm < OpenViBE::Plugins::IBoxAlgorithm >, OVP_ClassId_BoxAlgorithm_NeuroperfDisplay);

		protected:

			// Codec
			OpenViBEToolkit::TStreamedMatrixDecoder<CBoxAlgorithmNeuroperfDisplay> m_iMatrixCodec;
			OpenViBEToolkit::TStimulationDecoder<CBoxAlgorithmNeuroperfDisplay> m_iStimCodec;
			OpenViBE::IMatrix* m_iMatrix = nullptr;				// Input Matrix pointer
			OpenViBE::IStimulationSet* m_iStimSet = nullptr;	// Input Stimulation Set pointer

			// Outputs: visualization in a gtk window
			GtkWidget* m_widget = nullptr;
			OpenViBEVisualizationToolkit::IVisualizationContext* m_vizuContext = nullptr;

			// Variables (GTK)
			std::vector<GdkColor> m_cBar;				// Couleurs
			SRect m_rColor, m_rBar, m_rTime;			// Dimensions
			gint m_windowW = 0, m_windowH = 0,			// Dimensions Fenetre
				 m_iBarStep = 0, m_iTimeStep = 0;		// Taille de pas (en pixel)
			bool m_needRedraw = true;					// Force redraw

			// Variables
			double m_min = 0, m_max = 0;						// Bornes
			float m_fBarStep = 0, m_fTimeStep = 0;				// Taille de pas (en flottant)
			size_t m_gradientSteps = 0, m_lastBarStep = 0,		// Nombre de degrade et position precedente
				   m_rewardTime = 0;							// Duree recompense
			uint64_t m_lastTimeStep = 0, m_time = 0;
			std::vector<std::pair<bool, uint64_t>> m_rewards;

			// Fonctions
			virtual bool computeSize();											// Calcul des diff�rentes tailles et trac� de base
			virtual bool draw(size_t step, uint64_t time);						// Mise a jour du trace
			virtual bool drawRewards(GdkGC* gc, size_t number = 0);				// Trace des recompenses et art�facts
			virtual bool drawTimer(GdkGC* gc);									// Trace du Timer
			virtual bool drawPalette(GdkGC* gc);								// Trace de la Palette
			virtual bool drawProgress(GdkGC* gc, size_t step, uint64_t time);	// Mise a jour des barres
		};

		/// <summary>	 Descriptor of the box Neuroperf Display. </summary>
		class CBoxAlgorithmNeuroperfDisplayDesc : virtual public OpenViBE::Plugins::IBoxAlgorithmDesc
		{
		public:

			void release() override { }

			OpenViBE::CString getName() const override { return OpenViBE::CString("Neuroperf Display"); }
			OpenViBE::CString getAuthorName() const override { return OpenViBE::CString("Thibaut Monseigne"); }
			OpenViBE::CString getAuthorCompanyName() const override { return OpenViBE::CString("SANPSY / Inria Bordeaux"); }
			OpenViBE::CString getShortDescription() const override { return OpenViBE::CString("Neuroperf Display"); }
			OpenViBE::CString getDetailedDescription() const override { return OpenViBE::CString("Display for the NEUROPERF protocole\nWith a min and a max, calculates the position on a color scale and gives the right color.\nGives the instantaneous value on a color scale and the elapsed time of the work block.\nStimulations can be added to display rewards and artifacts on the timeline"); }
			OpenViBE::CString getCategory() const override { return OpenViBE::CString("NEUROPERF"); }
			OpenViBE::CString getVersion() const override { return OpenViBE::CString("2.0"); }
			OpenViBE::CString getStockItemName() const override { return OpenViBE::CString("gtk-find"); }

			OpenViBE::CIdentifier getCreatedClass() const override { return OVP_ClassId_BoxAlgorithm_NeuroperfDisplay; }
			OpenViBE::Plugins::IPluginObject* create() override { return new CBoxAlgorithmNeuroperfDisplay; }

			OpenViBE::boolean hasFunctionality(OpenViBE::CIdentifier func) const override
			{
				return func == OVD_Functionality_Visualization;
			}

			bool getBoxPrototype(OpenViBE::Kernel::IBoxProto& rBoxPrototype) const override
			{
				rBoxPrototype.addInput("Value", OV_TypeId_StreamedMatrix);
				rBoxPrototype.addInput("Rewards", OV_TypeId_Stimulations);

				rBoxPrototype.addSetting("Color gradient", OV_TypeId_ColorGradient, "0:0,0,50; 25:0,100,100; 50:0,50,0; 75:100,100,0; 100:75,0,0");
				rBoxPrototype.addSetting("Steps", OV_TypeId_Integer, "255");
				rBoxPrototype.addSetting("Min", OV_TypeId_Float, "0");
				rBoxPrototype.addSetting("Max", OV_TypeId_Float, "100");
				rBoxPrototype.addSetting("Time", OV_TypeId_Float, "300");

				rBoxPrototype.addFlag(OV_AttributeId_Box_FlagIsUnstable);


				return true;
			}

			_IsDerivedFromClass_Final_(OpenViBE::Plugins::IBoxAlgorithmDesc, OVP_ClassId_BoxAlgorithm_NeuroperfDisplayDesc);
		};
	}  // namespace NEUROPERF
} // namespace OpenViBEPlugins
