/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
/*
 *  $Id$
 */

#ifndef _HAVE_UNDOABLE_DATA_MODEL_H
#define _HAVE_UNDOABLE_DATA_MODEL_H 1


#include <fstream>
#include <sstream>
#include <set>

#include "DataModel/DataModel.h"

using namespace std;

namespace tag {
/**
 *  @class UndoableDataModel
 *  @ingroup DataModel
 *  implements undo/redo mechanism for DataModel
 */

class UndoableDataModel : public DataModel
{
	public:
		/*! default constructor */
		UndoableDataModel() { setInhibUndoRedo(false) ; }
		/**
		 * constructor
		 * @param corpus_name corpus name
		 */
		UndoableDataModel(const string& corpus_name)
		: DataModel(corpus_name) {}
		/*! destructor */
		~UndoableDataModel() {}

		/**
		 * inhibate undo / redo mechanism
		 * @param b true to inhibate undo/redo, false to restore undo/redo
		 */
		void inhibateUndoRedo(bool b) { setInhibUndoRedo(b) ; }

		/**
		 * undo/redo signal handler - should be connected by callee to undo/redo events
		 * @param eventData event data
		 * @param undo undo if true, redo if false
		 */
		void onUndoRedo(const std::string& eventData, bool undo);

		/* undo/redo signals */
		/**
		 * Signal emitted when an undoable action has occured.\n
		 * <b>string parameter:</b>			action description\n
		 * <b>int parameter:</b>			related track no\n
		 */
		sigc::signal<void, const std::string&, int>& signalUndoableAction() { return m_signalUndoableAction ; }
		/**
		 * Signal emitted when undo/redo stack is corrupted.\n
		 * <b>string parameter:</b>			action description\n
		 * <b>bool parameter:</b>			true if was undoing, false if was redoing\n
		 */
		sigc::signal<void, bool>& signalUndoRedoStackCorruption() { return m_signalUndoRedoStackCorruption ; }
		/**
		 * Signal emitted when an action has been undone or redone.\n
		 * <b>string parameter:</b>			Element type\n
		 * <b>string parameter:</b>			Element id\n
		 * <b>int	  parameter:</b>		Update action type\n
		 */
		sigc::signal<void, const std::string&, const std::string&, UpdateType>& signalUndoRedoModification() { return m_signalUndoRedoModification ; }

		/** herited methods **/
		void tagRemoveFromAnchorLinks(const string& anchorId, const string& toBeRemoved) ;
		void tagInsertIntoAnchorLinks(const string& anchorId, const string& toBeInserted) ;

	protected:

		/**
		 * @name AGAPI interfaces redefined for undo/redo support
		 */
		//@{
		virtual AnnotationId agCreateAnnotation(const Id& id, const string& type, const AnchorId& start, const AnchorId& end);
		virtual void agDeleteAnnotation(const AnnotationId& id) ;

		virtual AnnotationId agCopyAnnotation(const AnnotationId& id, const AnnotationId& toBeCreated="") ;
		virtual list<AnnotationId> agSplitAnnotation(const AnnotationId& id, const AnnotationId& toBeCreated, AnchorId& anchorTobeCreated);
		void undoSplitAnnotation(const AnnotationId& base, const AnnotationId& neww);

		virtual void agSetFeature(const AnnotationId& id, const string& key, const string& value);
		virtual void agDeleteFeature(const AnnotationId& id, const string& key);
		//virtual void agUnsetFeature(const AnnotationId& id, const string& key) ;
		virtual AnchorId agCreateAnchor(const Id& id) ;
		virtual AnchorId agCreateAnchor(const Id& id, float offset, string unit, set<SignalId>& sigIds) ;

		virtual AnchorId agSplitAnchor(const AnchorId& id, const AnchorId& toBeCreated);
		void undoSplitAnchor(const AnchorId& base, const AnchorId& neww);

		virtual void agDeleteAnchor(const Id& id) ;
		virtual void agSetStartAnchor(const AnnotationId& id, const AnchorId& a) ;
		virtual void agSetEndAnchor(const AnnotationId& id, const AnchorId& a) ;
		virtual void agSetAnchorOffset(const AnchorId& id, float f);
		virtual void agUnsetAnchorOffset(const AnchorId& id) ;
		virtual void agSetStartOffset(const AnnotationId& id, float offset) ;

		//@}

	private:

		sigc::signal<void, const std::string&, int> m_signalUndoableAction ;
		sigc::signal<void, bool> m_signalUndoRedoStackCorruption ;
		sigc::signal<void, const std::string&, const std::string&, UpdateType> m_signalUndoRedoModification ;
		static set<string> not_a_feature;

		int getAViewTrack(string id, string mode) ;

		bool isUndoableKey(const string& key) ;
		bool isFeatureItem(const string& key) ;
		void undoAction(const std::string& eventData) ;
		void redoAction(const std::string& eventData) ;
		void emitUndoRedoSignal(const string& type, const string& id, UpdateType upd) ;
};

} // end namespace

#endif /* _HAVE_UNDOABLE_DATA_MODEL_H */
