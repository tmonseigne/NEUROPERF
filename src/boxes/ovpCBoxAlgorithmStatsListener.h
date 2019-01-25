#pragma once

#include "ovp_defines.h"
#include <openvibe/ov_all.h>
#include <toolkit/ovtk_all.h>

#define OVP_ClassId_BoxAlgorithm_StatsListener (0x463b82a8, 0x5c8fa780)
#define OVP_ClassId_BoxAlgorithm_StatsListenerDesc (0xd34e947d, 0xbf40f345)
#define OV_AttributeId_Box_FlagIsUnstable OpenViBE::CIdentifier(0x666FFFFF, 0x666FFFFF)


namespace OpenViBEPlugins
{
	namespace NEUROPERF
	{
		/// <summary>	 The class CBoxAlgorithmStatsListener describes the box Stats Listener. </summary>
		class CBoxAlgorithmStatsListener : virtual public OpenViBEToolkit::TBoxAlgorithm<OpenViBE::Plugins::IBoxAlgorithm>
		{
		public:
			void release() override { delete this; }

			bool initialize() override;
			bool uninitialize() override;

			bool processInput(uint32_t inputIndex) override;
			bool process() override;

			_IsDerivedFromClass_Final_(OpenViBEToolkit::TBoxAlgorithm < OpenViBE::Plugins::IBoxAlgorithm >, OVP_ClassId_BoxAlgorithm_StatsListener);

		protected:
			// Codec
			OpenViBEToolkit::TSignalDecoder<CBoxAlgorithmStatsListener> m_iSignalCodec;		// Signal
			OpenViBEToolkit::TStimulationDecoder<CBoxAlgorithmStatsListener> m_iStimCodec;	// Start/Stop Stimulation
			OpenViBEToolkit::TStreamedMatrixEncoder<CBoxAlgorithmStatsListener> m_oMatrixCodec;		// Stat Matrix Encodeur
			OpenViBE::IMatrix* m_iMatrix = nullptr;				// Input Matrix pointer
			OpenViBE::IMatrix* m_oMatrix = nullptr;				// Output Matrix pointer
			OpenViBE::IStimulationSet* m_iStimSet = nullptr;	// Input Stimulation Set pointer

			// Variables
			bool m_isEnable = false;								// Activation du calcul
			size_t m_chunkNumber = 0;						// Nombre de morceaux recuperes
			double m_sum = 0, m_squaredSum = 0;			// Somme et somme des carres
		};

		/// <summary>	 Descriptor of the box Stats Listener. </summary>
		class CBoxAlgorithmStatsListenerDesc : virtual public OpenViBE::Plugins::IBoxAlgorithmDesc
		{
		public:

			void release() override { }

			OpenViBE::CString getName() const override { return OpenViBE::CString("Stats Listener"); }
			OpenViBE::CString getAuthorName() const override { return OpenViBE::CString("Thibaut Monseigne"); }
			OpenViBE::CString getAuthorCompanyName() const override { return OpenViBE::CString("SANPSY / Inria Bordeaux"); }
			OpenViBE::CString getShortDescription() const override { return OpenViBE::CString("Stats Listener"); }
			OpenViBE::CString getDetailedDescription() const override { return OpenViBE::CString("Stats Listener \nCompute and send min, max, mean and standard deviation\nStart with Baseline Start Stimulation\nEnd with Baseline Stop Stimulation"); }
			OpenViBE::CString getCategory() const override { return OpenViBE::CString("NEUROPERF"); }
			OpenViBE::CString getVersion() const override { return OpenViBE::CString("1.0"); }
			OpenViBE::CString getStockItemName() const override { return OpenViBE::CString("gtk-sort-ascending"); }

			OpenViBE::CIdentifier getCreatedClass() const override { return OVP_ClassId_BoxAlgorithm_StatsListener; }
			OpenViBE::Plugins::IPluginObject* create() override { return new CBoxAlgorithmStatsListener; }

			bool getBoxPrototype(OpenViBE::Kernel::IBoxProto& rBoxAlgorithmPrototype) const override
			{
				rBoxAlgorithmPrototype.addInput("Signal",OV_TypeId_Signal);
				rBoxAlgorithmPrototype.addInput("Stimulations",OV_TypeId_Stimulations);

				rBoxAlgorithmPrototype.addOutput("Stats",OV_TypeId_StreamedMatrix);

				rBoxAlgorithmPrototype.addFlag(OV_AttributeId_Box_FlagIsUnstable);

				return true;
			}

			_IsDerivedFromClass_Final_(OpenViBE::Plugins::IBoxAlgorithmDesc, OVP_ClassId_BoxAlgorithm_StatsListenerDesc);
		};
	}  // namespace NEUROPERF
}  // namespace OpenViBEPlugins
