#include "ovp_defines.h"
#include <openvibe/ov_all.h>

// Boxes Includes
#include "boxes/ovpCBoxAlgorithmArtefactDetector.h"
#include "boxes/ovpCBoxAlgorithmNeuroperfDisplay.h"
#include "boxes/ovpCBoxAlgorithmScoreListener.h"
#include "boxes/ovpCBoxAlgorithmStatsListener.h"
#include "boxes/ovpCBoxAlgorithmTrafficLight.h"

OVP_Declare_Begin();
	// Register boxes
	OVP_Declare_New(OpenViBEPlugins::NEUROPERF::CBoxAlgorithmArtefactDetectorDesc);
	OVP_Declare_New(OpenViBEPlugins::NEUROPERF::CBoxAlgorithmNeuroperfDisplayDesc);
	OVP_Declare_New(OpenViBEPlugins::NEUROPERF::CBoxAlgorithmScoreListenerDesc);
	OVP_Declare_New(OpenViBEPlugins::NEUROPERF::CBoxAlgorithmStatsListenerDesc);
	OVP_Declare_New(OpenViBEPlugins::NEUROPERF::CBoxAlgorithmTrafficLightDesc);

OVP_Declare_End();