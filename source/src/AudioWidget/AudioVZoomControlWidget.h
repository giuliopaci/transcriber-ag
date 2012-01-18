/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/

/** @file */

#ifndef __HAVE_AUDIOVZOOMCONTROLWIDGET__
#define __HAVE_AUDIOVZOOMCONTROLWIDGET__

#include <math.h>


namespace tag {

/**
 * @class	AudioVZoomControlWidget
 * @ingroup	AudioWidget
 *
 * Scale widget allowing vertical zoom control
 */
class AudioVZoomControlWidget : public Gtk::Frame
{
public:
	/**
	 * Default constructor
	 * @param p_maxFactor	Maximum zoom factor allowed
	 */
	AudioVZoomControlWidget(float p_maxFactor);

	/**
	 * Default destructor
	 */
	~AudioVZoomControlWidget();

	/**
	 * Converts scale values into zoom factors
	 * @param p_scaleValue	Input scale value
	 */
	float scaleValueToFactor(float p_scaleValue);

	/**
	 * Converts zoom factors into scale values
	 * @param p_factor	Input zoom factor
	 */
	float factorToScaleValue(float p_factor);

	/**
	 * Zoom factor accessor
	 * @return Zoom factor
	 */
	float getFactor() { return a_factor; }
	
	/**
	 * Zoom factor accessor
	 * @param p_factor Zoom factor
	 */
	void setFactor(float p_factor);

	/**
	 * Scale widget reset
	 */
	void reset();

	/**
	 * GTK signal - FocusIn
	 * @return FocusIn signal
	 */
	sigc::signal<void> signalFocusIn() { return a_signalFocusIn; }

	/**
	 * GTK signal - ValueChanged
	 * @return ValueChanged signal
	 */
	sigc::signal<void, float> signalValueChanged() { return a_signalValueChanged; }

protected:
	/**
	 * GTK callback - Scrollbar FocusIn
	 * @param event	Gdk focus event (GdkEventFocus)
	 */
	bool onScrollBarFocusIn(GdkEventFocus* event);

	/**
	 * GTK callback - Scale value changed
	 * @param p_scrollType	GTK scrolltype (check GTK docs)
	 * @param p_data		New scale value
	 * @return false
	 */
	bool onScaleValueChanged(Gtk::ScrollType p_scrollType, double p_data);

	/**
	 * Updates GUI with current zoom value
	 */
	void updateGUI();

	/**
	 * Float formatter (string with 2 max digits after comma)
	 * Examples : 46.00 => 46 ; 46.10 => 46.1 ; 46.12 => 46.12
	 * @param p_f Input float
	 * @return Output formatted string
	 */
	static char* formatStringFloat(float p_f);


	// -----------------
	// --- Variables ---
	// -----------------
	float			a_factor;			/**< Zoom factor, exs: 0.5 (half) / 2.0 (double) */
	float			a_maxFactor;		/**< Maximum zoom factor */
	Gtk::Scale*		a_scale;			/**< GTK scale */
	Gtk::Tooltips	a_tooltips;			/**< GTK tooltips */
	Gtk::Label		a_labelValue;		/**< GTK scale label */
	Gtk::Label		a_labelValueUnit;	/**< GTK scale - Unit */
	Gtk::Label		a_labelValueData;	/**< GTK scale - Data */
	Gtk::Alignment	a_labelValueAlign;	/**< GTK scale - Align */
		
	sigc::signal<void>			a_signalFocusIn;		/**< GTK signal - FocusIn */
	sigc::signal<void, float>	a_signalValueChanged;	/**< GTK signal - ValueChanged */
};

} // namespace

#endif // __HAVE_AUDIOVZOOMCONTROLWIDGET__

