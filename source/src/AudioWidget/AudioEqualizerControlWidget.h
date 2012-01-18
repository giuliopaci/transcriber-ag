/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/

/** @file */

#ifndef __HAVE_AUDIOEQUALIZERCONTROLWIDGET__
#define __HAVE_AUDIOEQUALIZERCONTROLWIDGET__

#include <math.h>

namespace tag {

/**
 * @class	AudioEqualizerControlWidget
 * @ingroup	AudioWidget
 *
 * Widget implementing an multi-ranges audio equalizer
 */
class AudioEqualizerControlWidget : public Gtk::Frame
{
public:
	/**
	 * Default Constructor
	 * @param p_rangesCount		Equalizer - Ranges count
	 * @param p_rangesFreqs		Range frequencies (in Hz)
	 * @param p_displayLimits	True to display limit ranges (ie. min/max frequencies), false to hide
	 */
	AudioEqualizerControlWidget(int p_rangesCount, float* p_rangesFreqs, bool p_displayLimits);

	/**
	 * Ranges count accessor
	 * @return Equalizer - Ranges count
	 */
	int getRangesCount() { return a_rangesCount; }

	/**
	 * Ranges frequencies accessor
	 * @return Range frequencies (in Hz)
	 */
	float* getRangesFreqs() { return a_rangesFreqs; }

	/**
	 * Ranges factors accessor
	 * @return Range factors, from 0.5 (half) to 2.0 (double)
	 */
	float* getRangesFactors() { return a_rangesFactors; }

	/**
	 * Ranges factors setter
	 * @param p_rangesFactors	Range factors (array)
	 */
	void setRangesFactors(float* p_rangesFactors);

	/**
	 * Display limits accessor
	 * @return True if limits are displayed, false otherwise.
	 */
	bool getDisplayLimits() { return a_displayLimits; }

	/**
	 * GTK Signal : value changed accessor
	 * @return GTK valueChanged signal
	 */
	sigc::signal<void, float*> signalValueChanged() { return a_signalValueChanged; }

protected:
	int		a_rangesCount;								/**< Equalizer - Ranges count */
	float*	a_rangesFreqs;								/**< Range frequencies (in Hz) */
	float*	a_rangesFactors;							/**< Range factors from 0.5 (half) to 2.0 (double) */
	bool	a_displayLimits;							/**< True if limits are being displayed, false otherwise. */
	Gtk::VScale** a_scales;								/**< GTK graphic scales */
	Gtk::Label** a_labelsValues;						/**< GTK graphic labels containing scales values */
	sigc::signal<void, float*> a_signalValueChanged;	/**< GTK valueChanged signal */

	/**
	 * GTK callback method - Scale value changed
	 * @param p_scrollType	GTK scroll type (check GTK docs)
	 * @param p_data		New scale value
	 * @param p_index		Scale index
	 * @return false
	 */
	bool onScaleValueChanged(Gtk::ScrollType p_scrollType, double p_data, int p_index);

	/**
	 * GTK callback method - Reset button is clicked
	 */
	void onResetButtonClicked();

	/**
	 * Refreshes GUI widgets with current factors
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
	 * Converts decibels into range factor
	 * @param p_DB	Decibels value
	 * @return Equivalent range factor
	 */
	static float DBtoFactor(float p_DB);

	/**
	 * Converts range factor into decibels
	 * @param p_factor Range factor
	 * @return Decibels conversion
	 */
	static float factorToDB(float p_factor);
};

} // namespace

#endif

