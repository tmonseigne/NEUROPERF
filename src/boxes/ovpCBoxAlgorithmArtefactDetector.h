#pragma once

#include "ovp_defines.h"
#include <openvibe/ov_all.h>
#include <toolkit/ovtk_all.h>

#define OVP_ClassId_BoxAlgorithm_ArtefactDetector (0x4aefaf18, 0xb68095e4)
#define OVP_ClassId_BoxAlgorithm_ArtefactDetectorDesc (0xa65f2a61, 0x83596875)
#define OV_AttributeId_Box_FlagIsUnstable OpenViBE::CIdentifier(0x666FFFFF, 0x666FFFFF)


namespace OpenViBEPlugins
{
	namespace NEUROPERF
	{
		/// <summary>	The class CBoxAlgorithmArtefactDetector describes the box Artefact Detector. </summary>
		class CBoxAlgorithmArtefactDetector : virtual public OpenViBEToolkit::TBoxAlgorithm<OpenViBE::Plugins::IBoxAlgorithm>
		{
		public:
			void release() override { delete this; }

			bool initialize() override;
			bool uninitialize() override;

			bool processInput(uint32_t inputIndex) override;
			bool process() override;

			_IsDerivedFromClass_Final_(OpenViBEToolkit::TBoxAlgorithm < OpenViBE::Plugins::IBoxAlgorithm >, OVP_ClassId_BoxAlgorithm_ArtefactDetector);

		protected:

			OpenViBEToolkit::TSignalDecoder<CBoxAlgorithmArtefactDetector> m_iSignalCodec;			// Input decoder
			OpenViBEToolkit::TSignalEncoder<CBoxAlgorithmArtefactDetector> m_oSignalCodec;			// Output decoder
			OpenViBE::IMatrix* m_iMatrix = nullptr;		// Input Matrix pointer
			OpenViBE::IMatrix* m_oMatrix = nullptr;		// Output Matrix pointer

			double m_max = 0;
		};

		/// <summary>	Descriptor of the box Artefact Detector. </summary>
		class CBoxAlgorithmArtefactDetectorDesc : virtual public OpenViBE::Plugins::IBoxAlgorithmDesc
		{
		public:

			void release() override { }

			OpenViBE::CString getName() const override { return OpenViBE::CString("Artefact Detector"); }
			OpenViBE::CString getAuthorName() const override { return OpenViBE::CString("Thibaut Monseigne"); }
			OpenViBE::CString getAuthorCompanyName() const override { return OpenViBE::CString("SANPSY / Inria Bordeaux"); }
			OpenViBE::CString getShortDescription() const override { return OpenViBE::CString("Simple Artefact Detection"); }
			OpenViBE::CString getDetailedDescription() const override { return OpenViBE::CString("Check if one element is higher than Max setting.\nGive 0 if no artefact, 1 if artefact."); }
			OpenViBE::CString getCategory() const override { return OpenViBE::CString("NEUROPERF"); }
			OpenViBE::CString getVersion() const override { return OpenViBE::CString("1.0"); }
			OpenViBE::CString getStockItemName() const override { return OpenViBE::CString("gtk-no"); }

			OpenViBE::CIdentifier getCreatedClass() const override { return OVP_ClassId_BoxAlgorithm_ArtefactDetector; }
			OpenViBE::Plugins::IPluginObject* create() override { return new CBoxAlgorithmArtefactDetector; }

			bool getBoxPrototype(OpenViBE::Kernel::IBoxProto& rBoxAlgorithmPrototype) const override
			{
				rBoxAlgorithmPrototype.addInput("Signal",OV_TypeId_Signal);
				rBoxAlgorithmPrototype.addOutput("Is Artefact",OV_TypeId_Signal);
				rBoxAlgorithmPrototype.addSetting("Max (mV)",OV_TypeId_Float, "100");
				rBoxAlgorithmPrototype.addFlag(OV_AttributeId_Box_FlagIsUnstable);
				return true;
			}

			_IsDerivedFromClass_Final_(OpenViBE::Plugins::IBoxAlgorithmDesc, OVP_ClassId_BoxAlgorithm_ArtefactDetectorDesc);
		};
	}  // namespace NEUROPERF
}  // namespace OpenViBEPlugins
