#pragma once

#include "ovp_defines.h"
#include <openvibe/ov_all.h>
#include <toolkit/ovtk_all.h>
#include <visualization-toolkit/ovviz_all.h>
#include <gtk/gtk.h>
#include <map>
#include <string>

#define OVP_ClassId_BoxAlgorithm_TrafficLight (0x6ff1f864, 0xcc5f2fd8)
#define OVP_ClassId_BoxAlgorithm_TrafficLightDesc (0x8d259567, 0x25f832b1)
#define OV_AttributeId_Box_FlagIsUnstable OpenViBE::CIdentifier(0x666FFFFF, 0x666FFFFF)


namespace OpenViBEPlugins
{
	namespace NEUROPERF
	{
		/// <summary>	 The class CBoxAlgorithmTrafficLight describes the box Traffic Light. </summary>
		class CBoxAlgorithmTrafficLight : virtual public OpenViBEToolkit::TBoxAlgorithm<OpenViBE::Plugins::IBoxAlgorithm>
		{
		public:
			void release() override { delete this; }

			bool initialize() override;
			bool uninitialize() override;
			bool processInput(uint32_t inputIndex) override;
			bool processClock(OpenViBE::CMessageClock& rMessageClock) override;
			uint64_t getClockFrequency() override;
			bool process() override;

			_IsDerivedFromClass_Final_(OpenViBEToolkit::TBoxAlgorithm < OpenViBE::Plugins::IBoxAlgorithm >, OVP_ClassId_BoxAlgorithm_TrafficLight);

		protected:
			// Codec
			OpenViBEToolkit::TStimulationDecoder<CBoxAlgorithmTrafficLight> m_iStimCodec;
			OpenViBE::IStimulationSet* m_iStimSet = nullptr;	// Input Stimulation Set pointer

			// Outputs: visualization in a gtk window
			GtkWidget* m_widget = nullptr;
			OpenViBEVisualizationToolkit::IVisualizationContext* m_vizuContext = nullptr;
			bool m_needRedraw = true;													// Force redraw
			// Variables
			size_t m_steps = 3, m_lastStep = 0;

			// Fonctions
			virtual bool computeSize();													// Calcul des diff�rentes tailles et trac� du fond
			virtual bool draw(size_t step);												// Mise a jour des lumieres
			virtual bool drawBackground(GdkGC* gc);										// Trace du fond
			virtual bool drawLight(GdkGC* gc, const SCircle& light, bool fill = false);	// Trace d'une lumiere (allume ou eteinte)
			virtual bool drawLights(GdkGC* gc, size_t step);							// Trace des Lumieres eteintes (sauf step)

			// Variables
			gint m_windowW = 0, m_windowH = 0;
			std::vector<SCircle> m_lights;												// Lumieres
			SRect m_back;																// Fond
		};

		/// <summary>	 Descriptor of the box Traffic Light. </summary>
		class CBoxAlgorithmTrafficLightDesc : virtual public OpenViBE::Plugins::IBoxAlgorithmDesc
		{
		public:

			void release() override { }

			OpenViBE::CString getName() const override { return OpenViBE::CString("Traffic Light"); }
			OpenViBE::CString getAuthorName() const override { return OpenViBE::CString("Thibaut Monseigne"); }
			OpenViBE::CString getAuthorCompanyName() const override { return OpenViBE::CString("SANPSY / Inria Bordeaux"); }
			OpenViBE::CString getShortDescription() const override { return OpenViBE::CString("Traffic Light Display"); }
			OpenViBE::CString getDetailedDescription() const override { return OpenViBE::CString("Display of a traffic light with N (Step parameter) lights on a color scale (Color parameter)"); }
			OpenViBE::CString getCategory() const override { return OpenViBE::CString("NEUROPERF"); }
			OpenViBE::CString getVersion() const override { return OpenViBE::CString("1.0"); }
			OpenViBE::CString getStockItemName() const override { return OpenViBE::CString("gtk-find"); }

			OpenViBE::CIdentifier getCreatedClass() const override { return OVP_ClassId_BoxAlgorithm_TrafficLight; }
			OpenViBE::Plugins::IPluginObject* create() override { return new CBoxAlgorithmTrafficLight; }

			OpenViBE::boolean hasFunctionality(OpenViBE::CIdentifier func) const override
			{
				return func == OVD_Functionality_Visualization;
			}

			bool getBoxPrototype(OpenViBE::Kernel::IBoxProto& rBoxPrototype) const override
			{
				rBoxPrototype.addInput("Step",OV_TypeId_Stimulations);

				rBoxPrototype.addSetting("Colors", OV_TypeId_ColorGradient, "0:0,0,50; 25:0,100,100; 50:0,50,0; 75:100,100,0; 100:75,0,0");
				rBoxPrototype.addSetting("Steps",OV_TypeId_Integer, "3");

				rBoxPrototype.addFlag(OV_AttributeId_Box_FlagIsUnstable);

				return true;
			}

			_IsDerivedFromClass_Final_(OpenViBE::Plugins::IBoxAlgorithmDesc, OVP_ClassId_BoxAlgorithm_TrafficLightDesc);
		};
	} // namespace NEUROPERF
}  // namespace OpenViBEPlugins
