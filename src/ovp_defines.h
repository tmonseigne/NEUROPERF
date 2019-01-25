#pragma once

#include <gtk/gtk.h>

//********** GTK Structures **********
struct SCircle final
{
	gint m_X = 0, m_Y = 0, m_D = 0, m_A1 = 0, m_A2 = 0;
	GdkColor m_Color{};
};

struct SRect final
{
	gint m_X = 0, m_Y = 0, m_W = 0, m_H = 0;
	GdkColor m_Color{};
};


//********** Functions **********
#define GETVALID(x, min, max) MIN(MAX(x, min), max)

//********** Variables **********
const GdkColor COLOR_BLACK = { 0, 0, 0, 0 },
			   COLOR_WHITE = { 0, 65535, 65535, 65535 },
			   COLOR_LIGHTGREY = { 0, 60000, 60000, 60000 },
			   COLOR_RED = { 0, 60909, 0, 0 },					// Ecarlate : (237, 0, 0)
			   COLOR_GREEN = { 0, 7967, 41120, 21845 },		// Malachite : (31, 160, 85)
			   COLOR_BLUE = { 0, 8738, 16962, 31868 };		// Cobalt : (34, 66, 124)
