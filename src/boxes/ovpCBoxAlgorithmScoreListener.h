#pragma once

#include "ovp_defines.h"
#include <openvibe/ov_all.h>
#include <toolkit/ovtk_all.h>

#define OVP_ClassId_BoxAlgorithm_ScoreListener (0xa562449e, 0x2505b9fc)
#define OVP_ClassId_BoxAlgorithm_ScoreListenerDesc (0xe10a6905, 0x4d3c7e2a)
#define OV_AttributeId_Box_FlagIsUnstable OpenViBE::CIdentifier(0x666FFFFF, 0x666FFFFF)

namespace OpenViBEPlugins
{
	namespace NEUROPERF
	{
		/// <summary>	The class CBoxAlgorithmScoreListener describes the box Score Listener. </summary>
		class CBoxAlgorithmScoreListener : virtual public OpenViBEToolkit::TBoxAlgorithm<OpenViBE::Plugins::IBoxAlgorithm>
		{
		public:
			void release() override { delete this; }

			bool initialize() override;
			bool uninitialize() override;

			bool processInput(uint32_t ui32InputIndex) override;
			bool process() override;

			_IsDerivedFromClass_Final_(OpenViBEToolkit::TBoxAlgorithm < OpenViBE::Plugins::IBoxAlgorithm >, OVP_ClassId_BoxAlgorithm_ScoreListener);

		protected:
			// Input decoder:
			OpenViBEToolkit::TSignalDecoder<CBoxAlgorithmScoreListener> m_iSignalCodec;	// Signal
			OpenViBEToolkit::TStimulationDecoder<CBoxAlgorithmScoreListener> m_iStimCodec;	// Start/Stop Stimulation
			// Output decoder:
			OpenViBEToolkit::TStreamedMatrixEncoder<CBoxAlgorithmScoreListener> m_oMatrixCodec;	// Score Matrix Encoder
			OpenViBEToolkit::TStimulationEncoder<CBoxAlgorithmScoreListener> m_oStimulationCodec[2];	// Trafic Light & Reward Stimulation
			OpenViBE::IMatrix* m_iMatrix = nullptr;		// Input Matrix pointer
			OpenViBE::IMatrix* m_oMatrix = nullptr;		// Output Matrix pointer
			OpenViBE::IStimulationSet* m_iStimSet = nullptr;	// Input Stimulation Set pointer

			// Variables
			bool m_isEnable = false;						// Activation du calcul
			uint32_t m_chainNeed = 0,						// Nombre de recompense pour valider une Super recompense
					 m_rewardChain = 0,						// Nombre de recompense consecutives
					 m_prevState = 0;						// Zone Precedente
			uint64_t m_rewardTime = 0, m_prevTime = 0,		// Temps pour recompense et Temps lors du passage au dessus du seuil
					 m_prevStimTime[2];						// Temps pour la continuite des stimulations
			double m_coefScore = 0;							// Precalcul pour le score
			std::vector<double> m_thresholds;				// Liste des seuils

			// Fonctions
			bool computeRewards(double* buffer, double value, int& light, int& reward);
			bool updateRewards(double* buffer, uint64_t time, int& reward);
			bool stimSender(OpenViBE::Kernel::IBoxIO& boxContext, int index, uint64_t time, uint64_t stimCode);
		};

		/// <summary>	Descriptor of the box Score Listener. </summary>
		class CBoxAlgorithmScoreListenerDesc : virtual public OpenViBE::Plugins::IBoxAlgorithmDesc
		{
		public:

			void release() override { }

			OpenViBE::CString getName() const override { return OpenViBE::CString("Score Listener"); }
			OpenViBE::CString getAuthorName() const override { return OpenViBE::CString("Thibaut Monseigne"); }
			OpenViBE::CString getAuthorCompanyName() const override { return OpenViBE::CString("SANPSY / Inria Bordeaux"); }
			OpenViBE::CString getShortDescription() const override { return OpenViBE::CString("Score Listener"); }
			OpenViBE::CString getDetailedDescription() const override { return OpenViBE::CString("Score Listener \nCompute and send the number of Rewards and the score\nStart with Experiment Start Stimulation\nEnd with Experiment Stop Stimulation"); }
			OpenViBE::CString getCategory() const override { return OpenViBE::CString("NEUROPERF"); }
			OpenViBE::CString getVersion() const override { return OpenViBE::CString("1.0"); }
			OpenViBE::CString getStockItemName() const override { return OpenViBE::CString("gtk-sort-ascending"); }

			OpenViBE::CIdentifier getCreatedClass() const override { return OVP_ClassId_BoxAlgorithm_ScoreListener; }
			OpenViBE::Plugins::IPluginObject* create() override { return new CBoxAlgorithmScoreListener; }

			bool getBoxPrototype(OpenViBE::Kernel::IBoxProto& rBoxAlgorithmPrototype) const override
			{
				rBoxAlgorithmPrototype.addInput("Signal",OV_TypeId_Signal);
				rBoxAlgorithmPrototype.addInput("Stimulations",OV_TypeId_Stimulations);

				rBoxAlgorithmPrototype.addOutput("Score",OV_TypeId_StreamedMatrix);
				rBoxAlgorithmPrototype.addOutput("Traffic Light",OV_TypeId_Stimulations);
				rBoxAlgorithmPrototype.addOutput("Rewards",OV_TypeId_Stimulations);

				rBoxAlgorithmPrototype.addSetting("Rewards Time", OV_TypeId_Float, "0.25");
				rBoxAlgorithmPrototype.addSetting("Super Reward Level",OV_TypeId_Integer, "4");
				rBoxAlgorithmPrototype.addSetting("Min", OV_TypeId_Float, "0");
				rBoxAlgorithmPrototype.addSetting("Critical", OV_TypeId_Float, "1");
				rBoxAlgorithmPrototype.addSetting("Reward", OV_TypeId_Float, "2");
				rBoxAlgorithmPrototype.addSetting("Max", OV_TypeId_Float, "3");

				rBoxAlgorithmPrototype.addFlag(OV_AttributeId_Box_FlagIsUnstable);

				return true;
			}

			_IsDerivedFromClass_Final_(OpenViBE::Plugins::IBoxAlgorithmDesc, OVP_ClassId_BoxAlgorithm_ScoreListenerDesc);
		};
	}  // namespace NEUROPERF
}  // namespace OpenViBEPlugins
