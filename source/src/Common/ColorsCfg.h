/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/

/** @file */

#ifndef COLORSCFG_H_
#define COLORSCFG_H_

#include <gtkmm.h>

namespace tag {

//******************************************************************************
//********************************* EDITOR *************************************
//******************************************************************************

#define TAG_COLORS_BUFFER_LABEL_SEGMENT 		"segment"
#define TAG_COLORS_BUFFER_ENTITY 				"entity"
#define TAG_COLORS_BUFFER_HIGHLIGHT1_TURN 		"highlight1_turn"
#define TAG_COLORS_BUFFER_HIGHLIGHT1_SEGMENT	"highlight1_segment"
#define TAG_COLORS_BUFFER_HIGHLIGHT1_WORD		"highlight1_words"
#define TAG_COLORS_BUFFER_HIGHLIGHT2_TURN 		"highlight2_turn"
#define TAG_COLORS_BUFFER_HIGHLIGHT2_SEGMENT	"highlight2_segment"
#define TAG_COLORS_BUFFER_HIGHLIGHT2_WORD		"highlight2_words"
#define TAG_COLORS_BUFFER_TIMESTAMP_BACKGROUND	"timestamp_bg"
#define TAG_COLORS_BUFFER_TIMESTAMP_FOREGROUND	"timestamp_fg"


//******************************************************************************
//********************************* AUDIO **************************************
//******************************************************************************

#define TAG_COLORS_AUDIO_WAVE_ACTIVE_BACKGROUND 	"signal_bg_active"
#define TAG_COLORS_AUDIO_WAVE_BACKGROUND 			"signal_bg"
#define TAG_COLORS_AUDIO_WAVE_FOREGROUND			"signal_fg"
#define TAG_COLORS_AUDIO_WAVE_SELECTION				"signal_selection"
#define TAG_COLORS_AUDIO_WAVE_DISABLED				"signal_disabled"
#define TAG_COLORS_AUDIO_TIP_FOREGROUND				"signal_tip_fg"
#define TAG_COLORS_AUDIO_TIP_BACKGROUND				"signal_tip_bg"
#define TAG_COLORS_AUDIO_WAVE_SEGMENTEND			"signal_segment_end"
#define TAG_COLORS_AUDIO_WAVE_CURSOR				"signal_cursor"

/**
 * @class 		ColorsCfg
 * @ingroup		Common
 *
 * Class providing useful methods for color settings
 *
 */

class ColorsCfg
{

	public:
		/**
		 * Constructor
		 */
		ColorsCfg() {} ;
		~ColorsCfg() {} ;

		//******************************************************************************
		//********************************* TOOLS **************************************
		//******************************************************************************

		/**
		 * Convert a string hexadecimal color representation to RGB values
		 * @param 		color			Hexadecimal color value
		 * @param[out] 	color_rgb		Vector with the three corresponding RGB values
		 */
		static void color_from_str_to_rgb(Glib::ustring color, std::vector<double>& color_rgb)
		{
			color_rgb.clear() ;
			Gdk::Color c1(color) ;
			color_rgb.push_back(c1.get_red_p()) ;
			color_rgb.push_back(c1.get_green_p()) ;
			color_rgb.push_back(c1.get_blue_p());
		}

		/**
		 * Convert a RGB color representation to a string hexadecimal representation.
		 * @param color_rgb			Vector containing RGB values
		 * @return					Corresponding hexadecimal value
		 */
		static Glib::ustring color_from_rgb_to_str(const std::vector<double>& color_rgb)
		{
			if ( ! color_rgb.empty() ) {
				Gdk::Color c1("blue") ;
				c1.set_rgb_p(color_rgb[0], color_rgb[1], color_rgb[2]) ;
				std::vector<Gdk::Color> v ;
				v.insert(v.begin(), c1) ;
				Glib::ustring color = Gtk::ColorSelection::palette_to_string(v) ;
				return color ;
			}
			else {
				Log::err() << "colors manager: can't determine hexadecimal" << std::endl ;
			}
		}

		/**
		 * Change a string hexadecimal color representation to a darker or lighter color
		 * @param[out] color			hexadecimal color
		 * @param lightChange			changing light value
		 */
		static void color_from_str_change_light(std::string& color, const int lightChange)
		{
			Gdk::Color newColor(color);
			newColor.set_rgb(val_format( newColor.get_red() + lightChange ),
					 val_format( newColor.get_green() + lightChange ),
					 val_format( newColor.get_blue() + lightChange ));

			std::vector<Gdk::Color> v ;
			v.insert(v.begin(), newColor) ;
			color = Gtk::ColorSelection::palette_to_string(v) ;
		}


	private:

		static int val_format (int value)
		{
			if ( value < 0 ) return 0;
			else if ( value > 65535 ) return 65535;
			else return value;
		}
} ;

} //namespace

#endif /*COLORSCFG_H_*/
