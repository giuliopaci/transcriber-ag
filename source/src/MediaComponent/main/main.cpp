/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/

// -- Simple Stream Player --

// Plays a remote mp3 file

#define ZZZ (1000000 * 4)

#include "player/Player.h"

int main(int argc, char** argv)
{
	// -- Simple Player Test --
	if (argc > 1)
	{
		g_thread_init(NULL);

		Player *player = new Player();

		player->open(argv[1]);
		player->thr_play();

		usleep(ZZZ * 4);
	}
	else
	{
		printf("Please specify a local / remote URL\n");
	}

	return 0;
}

