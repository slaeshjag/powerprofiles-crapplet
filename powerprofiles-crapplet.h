/*
Copyright (c) 2024 Steven Arnow
'powerprofiles-crapplet.c' - This file is part of powerprofiles-crapplet

This software is provided 'as-is', without any express or implied
warranty. In no event will the authors be held liable for any damages
arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it
freely, subject to the following restrictions:

	1. The origin of this software must not be misrepresented; you must not
	claim that you wrote the original software. If you use this software
	in a product, an acknowledgment in the product documentation would be
	appreciated but is not required.

	2. Altered source versions must be plainly marked as such, and must not be
	misrepresented as being the original software.

	3. This notice may not be removed or altered from any source
	distribution.
*/

#ifndef __POWERPROFILES_CRAPPLET_H__
#define	__POWERPROFILES_CRAPPLET_H__


#include <gtk/gtk.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>
#include <dlfcn.h>
#include <signal.h>

#include <sys/types.h>


struct PowerProfileEntry {
	char				*name;
	char				*label;
	char				*profile_name;
	GtkWidget			*item;
	int				selected;
};


struct PowerProfileStruct {
	GtkStatusIcon			*icon;
	GtkWidget			*menu;
	
	struct PowerProfileEntry	*ppe;
	int				ppes;
};



#endif
