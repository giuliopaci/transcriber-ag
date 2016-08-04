/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
/* $Id */

/** @file */

#ifndef TOOLLAUNCHER_H_
#define TOOLLAUNCHER_H_

#include <gtkmm.h>
#include "Common/Parameters.h"
#include "Common/CommonXMLReader.h"
#include "Common/icons/Icons.h"
#include "Common/icons/IcoPackImage.h"
#include "Common/widgets/GeoWindow.h"

namespace tag {

/**
 * @class 	ToolLauncher
 * @ingroup GUI
 *	A class that launched external tool such as script or executables.
 */
class ToolLauncher
{
	public:

		/**
		 * Class representing tool information
		 */
		class Tool
		{
			public :
				/**
				 * Constructor
				 * @param identifiant	Unique tool identifiant
				 * @param display		String menu
				 * @param scope			"file" for file related tool or "global" for global tool
				 * @param type			"bin" for binaries, "script" for scripts. For scripts, a dialog will be prompted
				 */
				Tool(Glib::ustring identifiant, Glib::ustring display, Glib::ustring scope, Glib::ustring type) ;
				~Tool() {}

				/**
				 * Set tool path
				 * @param value		Script/binary path
				 */
				void setObject(Glib::ustring value) ;
				/**
				 * Set tool options
				 * @param value		Tool options
				 */
				void setOptions(Glib::ustring value) ;
				/**
				 * Set personalized options
				 * @param value		Tool options with replaced values
				 */
				void setPersonalizedOptions(Glib::ustring value) ;
				/**< Accessor */
				bool isFileScope() ;
				/**< Accessor */
				Glib::ustring getObject() ;
				/**< Accessor */
				Glib::ustring getIdentifiant() ;
				/**< Accessor */
				Glib::ustring getDisplay() ;
				/**< Accessor */
				Glib::ustring getCommand() ;
				/**< Accessor */
				Glib::ustring getOptions() ;
				/**< Accessor */
				Glib::ustring getType() ;
				/**< Accessor */
				std::vector<std::string> getOptionsArgv() ;
				/**
				 * Replaces variable in options by their real values
				 * @param file_path		File path
				 * @note  current version: only file path can be a variable
				 */
				void configureOptions(Glib::ustring file_path) ;

			private :
				Glib::ustring identifiant ; 			/**< unique tool identifiant */
				Glib::ustring display ; 				/**< display */
				Glib::ustring scope ; 					/**< scope (global/file) */
				Glib::ustring object ;					/**< command path */
				Glib::ustring options ;					/**< command options */
				Glib::ustring personalizedOptions ;		/**< command options with replaced values */
				Glib::ustring type ;					/**< bin or script */
		} ;

	public :

		/**
		 * Returns the single instance of the settings object, or creates it
		 * if it hasn't been created yet.
		 * @return		Pointer on the Setting singleton
		 */
		static ToolLauncher* getInstance() ;

		/**
		 * Creates and configure the ToolLauncher singleton.
		 * @param p_parameters		Application Parameters configuration
		 * @param parent			Graphic parent
		 */
		static void configure(Parameters* p_parameters, Gtk::Window* parent) ;

		/**
		 * Kills the ToolLauncher instance
		 */
		static void kill() ;

		/**
		 * Launch the associated tool
		 * @param tool	Tool to be launched
		 */
		void launch(ToolLauncher::Tool* tool) ;

		/**
		 * Adds a new tool
		 * @param identifiant	Tool identifiant
		 * @param display		Tool menu display
		 * @param scope			"file" for file related tool or "global" for global tool
		 * @param type			"bin" for ninaries, "script" for scripts. For scripts, a dialog will be prompted
		 * @return				Poiter on the newly created Tool
		 * @remarks				Mainly used in tool file parsing, otherwise useless
		 */
		ToolLauncher::Tool* addTool(Glib::ustring identifiant, Glib::ustring display, Glib::ustring scope, Glib::ustring type) ;

		/**
		 * Accessor to all loaded tools.
		 * @return		Vector of Tools pointers
		 */
		const std::vector<ToolLauncher::Tool*>& getTools() { return tools ; }

		/**
		 * Checks if the launcher has tools with global scope
		 * @return 		True if the launcher has tools with global scope
		 */
		bool hasGlobalScopeTools() { return (nbGlobal > 0) ;}

		/**
		 * Checks if the launcher has tools with file scope
		 * @return 		True if the launcher has tools with file scope
		 */
		bool hasFileScopeTools() { return (nbFile > 0) ;}

	private:
		/**
		 * Class representing tool information
		 */
		class ToolDialog : public Gtk::Dialog, public GeoWindow
		{
			public :
				ToolDialog(Glib::ustring command) ;
				~ToolDialog() {} ;

				/*** Geometry interface ***/
				virtual void saveGeoAndHide() ;
				virtual int loadGeoAndDisplay(bool rundlg=false) ;

				void setStartStatus() ;
				void setEndStatus(std::string out, std::string err, int error) ;

				sigc::signal<void>& signalClosed() {return m_signalClosed ;}

			private :
				bool in_progress ;
				Gtk::ScrolledWindow scroll ;
				Gtk::VBox v_box ;

				Gtk::Button* close_button ;
				Gtk::HBox title_box ;
				Gtk::HBox command_hbox ;
				Gtk::Frame command_frame, stout_frame, sterr_frame ;
				Gtk::Label title_label, command_text ;
				Gtk::TextView* stout_view ;
				Gtk::TextView* sterr_view ;
				IcoPackImage title_image ;
				IcoPackImage title_waiting_image ;

				void onClose() ;
				bool on_delete_event(GdkEventAny* event) ;
				sigc::signal<void> m_signalClosed ;


				/*** Geometry interface ***/
				virtual void getGeo(int& size_xx, int& size_yy, int& pos_x, int& pos_y, int& panel) ;
				virtual void setGeo(int size_xx, int size_yy, int pos_x, int pos_y, int panel) ;
				virtual void getDefaultGeo(int& size_xx, int& size_yy, int& pos_x, int& pos_y, int& panel) ;
				virtual Glib::ustring getWindowTagType()  ;
		};


	private:

		static ToolLauncher* m_instance ;

		/**
		 * Constructor
		 * @param p_parameters		Application Parameters configuration
		 * @param parent			Graphic parent
		 */
		ToolLauncher(Parameters* p_parameters, Gtk::Window* parent) ;

		/**
		 * Destructor
		 */
		virtual ~ToolLauncher() ;

		/** Options */
		Parameters* parameters ;

		/** Graphic parent */
		Gtk::Window* parent ;

		/** Tools loaded */
		std::vector<ToolLauncher::Tool*> tools ;

		ToolDialog* dialog ;

		/** Count for quick test **/
		int nbGlobal ;
		int nbFile ;

		/** Loading method **/
		void loadTools() ;
		void emptyslot() {} ;
		void process(ToolLauncher::ToolDialog* dialog, Glib::ustring childDirectory, std::vector<std::string> arg) ;
		void onDialogClose() ;
};


/*** XML Management ***/
/**
 *  @class ToolLauncher_XMLHandler
 *  @ingroup DataModel
 *  SAX-based XML parsing handler for external tools
 */
class ToolLauncher_XMLHandler : public CommonXMLHandler
{
	public:
		/**
		 * constructor
		 * @param launcher	Pointer on the ToolLauncher
		 */
		ToolLauncher_XMLHandler(ToolLauncher* launcher) ;
		/*! desctructor */
		~ToolLauncher_XMLHandler();

		/**
		 * handler for XML element start
		 * @param uri
		 * @param localname
		 * @param qname
		 * @param attrs
		 */
		void startElement( const   XMLCh* const    uri,
			     const   XMLCh* const    localname,
			     const   XMLCh* const    qname,
			     const   Attributes&     attrs);
		/**
		 * handler for XML element end
		 * @param uri
		 * @param localname
		 * @param qname
		 */
		void endElement( const XMLCh* const uri,
			   const XMLCh* const localname,
			   const XMLCh* const qname);

		/**
		 * handler for XML element character data
		 * @param chars
		 * @param length
		 */
		void characters(const XMLCh* const chars, const XMLSize_t length);

	private:
		ToolLauncher* tLauncher ;
		ToolLauncher::Tool* m_current_tool ;
		XMLCh* m_tools ;
		XMLCh* m_tool ;
		XMLCh* m_object ;
		XMLCh* m_options ;

		Glib::ustring current_object ;
		Glib::ustring current_options ;
		bool in_object ;
		bool in_options ;
} ;

} // namespace

#endif /* TOOLLAUNCHER_H_ */
