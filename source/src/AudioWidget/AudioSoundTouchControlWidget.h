/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/

/** @file */

#ifndef __HAVE_AUDIOSOUNDTOUCHCONTROLWIDGET__
#define __HAVE_AUDIOSOUNDTOUCHCONTROLWIDGET__

namespace tag {

/**
 * @class	AudioSoundTouchControlWidget
 * @ingroup	AudioWidget
 * 
 * This class implements a graphical scale, dedicated to pitch / speed / tempo control.
 */
class AudioSoundTouchControlWidget : public Gtk::Frame
{
	public:
		/**
		 * Factor accessor
		 * @return The effect factor
		 */
		float getFactor() { return a_factor; }

		/**
		 * Factor accessor
		 * @param p_factor New effect factor
		 */
		void setFactor(float p_factor);

		/**
		 * GTK signal : valueChanged accessor
		 * @return GTK valueChanged signal
		 */
		sigc::signal<void, float> signalValueChanged() { return a_signalValueChanged; }

		/**
		 * GTK signal : focusIn accessor
		 * @return GTK focusIn signal
		 */
		sigc::signal<void> signalFocusIn() { return a_signalFocusIn; }

		/**
		 * Sets tooltip texts
		 * @param p_text1	Scale text
		 * @param p_text2	Reset button text
		 */
		void setTooltip(const char* p_text1, const char* p_text2)
		{
			a_tooltips.set_tip(*a_scale, p_text1);
			a_tooltips.set_tip(a_buttonReset, p_text2);
		}

	protected:
		/**
		 * Constructor
		 * @param p_title 			The title of the frame
		 * @param p_maxFactor 		The maximal modification factor
		 * @param p_vertical
		 * @param p_displayValues
		 * @param p_displayValue
		 * @param p_displayReset
		 * @note 					AudioSoundTouchControlWidget can only be instancied by subclasses.
		 */
		AudioSoundTouchControlWidget(char* p_title, float p_maxFactor, bool p_vertical, bool p_displayValues, bool p_displayValue, bool p_displayReset);
		~AudioSoundTouchControlWidget();

		/**
		 * GTK Event : Scrollbar FocusIn
		 * @param event : GDK Focus Event
		 */
		bool onScrollBarFocusIn(GdkEventFocus* event);

		/**
		 * GTK callback - Scale value changed
		 * @param p_scrollType	GTK scroll type (check GTK docs)
		 * @param p_data		New scale value
		 * @return false
		 */
		bool onScaleValueChanged(Gtk::ScrollType p_scrollType, double p_data);

		/**
		 * GTK callback - Reset button clicked
		 */
		void onResetButtonClicked();

		/**
		 * Resets values
		 */
		void reset();

		/**
		 * Updates GUI with current values
		 */
		void updateGUI();

		/**
		 * Converts a GTK scale value into effect factor
		 * @param p_scaleValue	GTK scale value
		 * @return Equivalent effect factor
		 */
		float scaleValueToFactor(float p_scaleValue);

		/**
		 * Converts an effect factor into GTK scale value
		 * @param p_factor Effect factor
		 * @return Equivalent scale value
		 */
		float factorToScaleValue(float p_factor);

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
		float a_factor;									/**< Effect factor, from 0.5 (half) to 2.0 (double) */
		float a_maxFactor;								/**< Max effect factor */
		bool a_vertical;								/**< Vertical orientation */
		Gtk::Scale* a_scale;							/**< GTK scale */
		Gtk::Label a_labelValue;						/**< GTK scale label */
		sigc::signal<void, float> a_signalValueChanged;	/**< GTK valueChanged signal */
		Gtk::Button a_buttonReset;						/**< GTK reset button */
		Gtk::Tooltips a_tooltips;						/**< GTK tooltips */

		Gtk::Label a_labelValueUnit;					/**< GTK label - Unit */
		Gtk::Label a_labelValueData;					/**< GTK label - Data */
		Gtk::Alignment a_labelValueAlign;				/**< GTK label - Align */

		sigc::signal<void> a_signalFocusIn;				/**< GTK Signal - FocusIn */
};

} // namespace

#endif // __HAVE_AUDIOSOUNDTOUCHCONTROLWIDGET__

