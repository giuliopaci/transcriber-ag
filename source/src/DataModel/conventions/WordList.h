/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/

#ifndef _HAVE_WORD_LIST
#define _HAVE_WORD_LIST

#include <list>
#include <set>
#include <ostream>
#include <glibmm.h>
#include <glibmm/objectbase.h>

#include "Common/CommonXMLReader.h"

using namespace std;

namespace tag {

/**
 *  @class WordList
 *  @ingroup DataModel
 *  Predefined word lists loaded from conventions definition.
 *  <br>Those lists are used as a support for transcription input, when the scripting of some terms like onomatopoeia,
 *  proper nouns, brand marks, ..., may be fixed in annotations conventions.
 */

class WordList {
public:
	/**
	 * constructor
	 * @param label list label
	 */
	WordList(std::string label) :
		m_label(label) {
	}
	;
	/*! copy constructor */
	WordList(const WordList& copy) :
		m_label(copy.getLabel()), m_words(copy.getWords()) {
	}

	typedef std::set<std::string>::iterator iterator;
	typedef std::set<std::string>::const_iterator const_iterator;

	WordList& operator=(const WordList& copy) {
		m_words.clear();
		m_words = copy.getWords();
		return *this;
	}

	/* dictionary size */
	int size() { return m_words.size(); } /**< nb of entries in word list */
	bool empty() {	return (m_words.size() == 0); } /**< true if empty word list */

	/*! browse through dictionary - begin iterator */
	const_iterator begin() const { 	return m_words.begin();	}
	/*! browse through dictionary - non-const begin iterator */
	iterator begin() {	return m_words.begin(); }
	/*! browse through dictionary - end iterator */
	const_iterator end() const { return m_words.end(); }
	/*! browse through dictionary - non-const end iterator */
	iterator end() { return m_words.end(); }

	const std::set<std::string>& getWords() const { return m_words;	} /**< get words in list */
	std::set<std::string>& getWords() {	return m_words; }  /**< get words in list - non const */
	void addWord(const char* word) { m_words.insert(word); }  /**< add word to list */
	void addWord(const string& word) {	m_words.insert(word);} /**< add word to list */

	const std::string& getLabel() const { return m_label; }  /**< get list label */

private:
	std::string m_label; /**< word list label */
	std::set<std::string> m_words; /**< all words */

public:
	/** load words lists from file in list
	 * @param in word lists file path
	 * @param lists (returned) list in which words lists will be loaded
	 */
	static void loadLists(string in, list<WordList>& lists) throw(const char *);
};

/**
 *  @class WordList_XMLHandler
 *  @ingroup DataModel
 *  SAX-based XML parsing handler for word lists
 */
class WordList_XMLHandler: public CommonXMLHandler {
public:

	/**
	 * constructor
	 * @param l list of wordlists where to store loaded data
	 */
	WordList_XMLHandler(list<WordList>& l);
	~WordList_XMLHandler();

	/**
	 * handler for XML element start
	 * @param uri
	 * @param localname
	 * @param qname
	 * @param attrs
	 */
	void startElement(const XMLCh* const uri, const XMLCh* const localname,
			const XMLCh* const qname, const Attributes& attrs);
	/**
	 * handler for XML element end
	 * @param uri
	 * @param localname
	 * @param qname
	 */
	void endElement(const XMLCh* const uri, const XMLCh* const localname,
			const XMLCh* const qname);
	/**
	 * handler for XML element character data
	 * @param chars
	 * @param length
	 */
	void characters(const XMLCh* const chars, const unsigned int length);

private:
	list<WordList>& m_wordLists;
	bool m_inword;
	XMLCh* m_wordTag;
	XMLCh* m_wordlistTag;
	string m_curWord;
};

} /* namespace tag */

#endif // _HAVE_WORD_LIST
