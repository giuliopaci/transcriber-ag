/* sendpraat.c */
/* by Paul Boersma */
/* December 24, 2007 */

/*
 * The sendpraat subroutine (Unix with X Window; Windows; Macintosh) sends a message
 * to a running program that uses the Praat shell.
 * The sendpraat program does the same from a Unix command shell,
 * from a Windows or DOS console, or from a MacOS X terminal window.
 *
 * New versions: http://www.praat.org or http://www.fon.hum.uva.nl/praat/sendpraat.html
 *
 * On Windows NT, 2000, and XP, this version works only with Praat version 4.3.28 and higher.
 * On Macintosh, this version works only with Praat version 3.8.75 and higher.
 */

/*******************************************************************

   THIS CODE CAN BE COPIED, USED, AND DISTRIBUTED FREELY.
   IF YOU MODIFY IT, PLEASE KEEP MY NAME AND MARK THE CHANGES.
   IF YOU IMPROVE IT, PLEASE NOTIFY ME AT paul.boersma@uva.nl.

*******************************************************************/

/*
 * The way to call the sendpraat subroutine from another program.
 */
char *sendpraat (void *display, const char *programName, long timeOut, const char *text);
wchar_t *sendpraatW (void *display, const wchar_t *programName, long timeOut, const wchar_t *text);
/*
 * Parameters:
 * 'display' is the Display pointer, which will be available if you call sendpraat from an X Window program.
 *    If 'display' is NULL, sendpraat will open the display by itself, and close it after use.
 *    On Windows and Macintosh, sendpraat ignores the 'display' parameter.
 * 'programName' is the name of the program that receives the message.
 *    This program must have been built with the Praat shell (the most common such programs are Praat and ALS).
 *    On Unix, the program name is usually all lower case, e.g. "praat" or "als", or the name of any other program.
 *    On Windows, you can use either "Praat", "praat", or the name of any other program.
 *    On Macintosh, 'programName' must be "Praat", "praat", "ALS", or the Macintosh signature of any other program.
 * 'timeOut' is the time (in seconds) after which sendpraat will return with a time-out error message
 *    if the receiving program sends no notification of completion.
 *    On Unix and Macintosh, the message is sent asynchronously if 'timeOut' is 0;
 *    this means that sendpraat will return OK (NULL) without waiting for the receiving program
 *    to handle the message.
 *    On Windows, the time out is ignored.
 * 'text' contains the contents of the Praat script to be sent to the receiving program.
 */

static char errorMessage [1000];
static wchar_t errorMessageW [1000];
#if xwin
	static long theTimeOut;
	static void handleCompletion (int message) { (void) message; }
	static void handleTimeOut (int message) { (void) message; sprintf (errorMessage, "Timed out after %ld seconds.", theTimeOut); }
#endif

