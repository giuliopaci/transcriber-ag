/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/

/** @file */

#ifndef __HAVE_AUDIOVOLUMECONTROLWIDGET__
#define __HAVE_AUDIOVOLUMECONTROLWIDGET__

namespace tag {

#include <math.h>


/**
 * @class	AudioVolumeControlWidget
 * @ingroup	AudioWidget
 *
 * Scale widget allowing playback volume control
 */
class AudioVolumeControlWidget : public Gtk::Frame
{
public:
	/**
	 * Default constructor
	 * @param p_vertical		Vertical orientation
	 * @param p_displayValues	Values display
	 * @param p_displayValue	Actual value display
	 * @param p_displayReset	Reset button display
	 */
	AudioVolumeControlWidget(bool p_vertical, bool p_displayValues, bool p_displayValue, bool p_displayReset);

	/**
	 * Default destructor
	 */
	~AudioVolumeControlWidget() {}

	/**
	 * Factor accessor
	 * @return Factor value
	 */
	float getFactor() { return a_factor; }

	/**
	 * Factor accessor
	 * @param p_factor New factor value
	 */
	void setFactor(float p_factor);

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

	/**
	 * Scale widget reset
	 */
	void reset();

protected:
	// -----------------
	// --- Variables ---
	// -----------------
	float			a_factor;						/**< Volume factor, from 0.5 (half) to 2.0 (double */
	bool			a_vertical;						/**< Vertical orientation */
	Gtk::Scale*		a_scale;						/**< GTK scale */
	Gtk::Tooltips	a_tooltips;						/**< GTK tooltips */
	Gtk::VBox		label_scale;					/**< GTK scale box */
	Gtk::Label		a_labelValue;					/**< GTK scale label */
	Gtk::Label		a_labelValueUnit;				/**< GTK label - Unit */
	Gtk::Label		a_labelValueData;				/**< GTK label - Data */
	Gtk::Alignment	a_labelValueAlign;				/**< GTK label - Align */

	/**
	 * GTK callback - Scale value changed
	 * @param p_scrollType	GTK scrolltype (check GTK docs)
	 * @param p_data		New scale value
	 * @return false
	 */
	bool onScaleValueChanged(Gtk::ScrollType p_scrollType, double p_data);

	/**
	 * GTK callback - Button press
	 * @param event	Gdk button event
	 */
	bool buttonPressEventCallback(GdkEventButton* event);

	/**
	 * GTK callback - FocusIn
	 * @param event Gdk focus event
	 */
	bool onScrollBarFocusIn(GdkEventFocus* event);

	/**
	 * GTK callback - Reset button clicked
	 */
	void onResetButtonClicked();

	/**
	 * Updates GUI with new factor value
	 */
	void updateGUI();

	/**
	 * Converts a GTK scale value into decibels
	 * @param p_scaleValue	GTK scale value
	 * @return Decibels conversion
	 */
	static float scaleValueToDB(float p_scaleValue);

	/**
	 * Converts decibels into GTK scale values
	 * @param p_DB	Decibels value
	 * @return GTK scale value
	 */
	static float DBToScaleValue(float p_DB);

	/**
	 * Converts decibels into volume factor 
	 * @param p_DB	Decibels value
	 * @return Equivalent volume factor
	 */
	static float DBtoFactor(float p_DB);

	/**
	 * Converts volume factor into decibels 
	 * @param p_factor Volume factor
	 * @return Decibels conversion
	 */
	static float factorToDB(float p_factor);

	sigc::signal<void>			a_signalFocusIn;		/**< GTK signal - FocusIn */
	sigc::signal<void, float>	a_signalValueChanged;	/**< GTK signal - ValueChanged */
};

} // namespace

#endif // __HAVE_AUDIOVOLUMECONTROLWIDGET__

