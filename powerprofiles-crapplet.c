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


#include "powerprofiles-crapplet.h"

#define	PPROFILES 3

struct PowerProfileLookup {
	char 		*name;
	char		*label;
	char		*profile_name;
};

static const struct PowerProfileLookup _pprofile_lut[PPROFILES] = {
	{ "low-power", "Battery saver", "power-saver" },
	{ "balanced", "Balanced", "balanced" },
	{ "performance", "Performance", "performance" }
};

static struct PowerProfileStruct *ppa_state;

void ppapplet_make_is_alone() {
	FILE *pidfile, *exec;
	int pid_kill;
	pid_t pid;
	char path[128];

	pid_kill = 0;

	if ((pidfile = fopen("/var/tmp/powerfiles-crapplet.pid", "r")) != NULL) {
		fscanf(pidfile, "%i", &pid_kill);
		if (pid_kill < 1);
		else {
			sprintf(path, "/proc/%i/cmdline", pid_kill);
			fprintf(stderr, "Opening %s...\n", path);
			if ((exec = fopen(path, "r")) != NULL) {
				fgets(path, 128, exec);
				if (strstr(path, "powerprofile-crapplet") != NULL)
					kill(pid_kill, 15);
				else fprintf(stderr, "PID %i doesn't seem to be a powerprofiles-crapplet: %s\n", pid_kill, path);
				fclose(exec);
			}
		}
		fclose(pidfile);
	}

	pid = getpid();

	if ((pidfile = fopen("/var/tmp/powerprofiles-crapplet.pid", "w")) == NULL) {
		fprintf(stderr, "ERROR: Unable to create /var/tmp/powerprofiles-crapplet.pid\n");
		exit(1);
	}

	fprintf(pidfile, "%i", pid);
	fclose(pidfile);

	return;
}


void ppapplet_activate(GtkWidget *menu_item, gpointer data) {
	struct PowerProfileEntry *ppe = data;
	char *cmdline;

	asprintf(&cmdline, "powerprofilesctl set %s", ppe->profile_name);
	system(cmdline);

	return;
}


void ppapplet_profile_active(struct PowerProfileStruct *ppa) {
	FILE *fp;
	int i;
	char buff[64];

	for (i = 0; i < ppa->ppes; i++)
		ppa->ppe[i].selected = 0;

	if (!(fp = fopen("/sys/firmware/acpi/platform_profile", "r")))
		return;
	*buff = 0, fscanf(fp, "%63s", buff);
	fclose(fp);

	for (i = 0; i < ppa->ppes; i++) {
		if (!strcmp(ppa->ppe[i].name, buff)) {
			ppa->ppe[i].selected = 1;
			break;
		}
	}
}


void ppapplet_menu_create(struct PowerProfileStruct *ppa) {
	int i;
	
	ppa->menu = gtk_menu_new();
	ppapplet_profile_active(ppa);
	
	for (i = 0; i < ppa->ppes; i++) {
		ppa->ppe[i].item = gtk_radio_menu_item_new_with_label(NULL, ppa->ppe[i].label);
		if (i > 0)
			gtk_radio_menu_item_join_group(GTK_RADIO_MENU_ITEM(ppa->ppe[i].item), GTK_RADIO_MENU_ITEM(ppa->ppe[i-1].item));
		if (ppa->ppe[i].selected)
			gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(ppa->ppe[i].item), 1);
		
		g_signal_connect(G_OBJECT(ppa->ppe[i].item), "activate", G_CALLBACK(ppapplet_activate), &ppa->ppe[i]);
		gtk_menu_shell_append(GTK_MENU_SHELL(ppa->menu), ppa->ppe[i].item);
	}

	gtk_widget_show_all(ppa->menu);
	
	return;
}


void ppapplet_menu_destroy(struct PowerProfileStruct *ppa) {
	gtk_widget_destroy(ppa->menu);

	return;
}


void ppapplet_menu_spawn(GtkWidget *icon, gpointer data) {
	struct PowerProfileStruct *ppa = data;

	if (ppa->menu != NULL)
		ppapplet_menu_destroy(ppa);

	ppapplet_menu_create(ppa);
	gtk_menu_popup_at_pointer(GTK_MENU(ppa->menu), NULL);

	return;
}


int ppapplet_profiles_detect(struct PowerProfileStruct *ppa) {
	FILE *fp;
	int i, j;
	char buff[64];

	if (!(fp = fopen("/sys/firmware/acpi/platform_profile_choices", "r")))
		return -1;
	
	for (i = 0; fscanf(fp, "%63s", buff) == 1; i++) {
		ppa->ppe = realloc(ppa->ppe, sizeof(*ppa->ppe) * (++ppa->ppes));
		ppa->ppe[i].item = NULL;
		ppa->ppe[i].name = strdup(buff);
		ppa->ppe[i].selected = 0;
		
		for (j = 0; j < PPROFILES; j++) {
			if (!strcmp(_pprofile_lut[j].name, ppa->ppe[i].name)) {
				ppa->ppe[i].profile_name = strdup(_pprofile_lut[j].profile_name);
				ppa->ppe[i].label = strdup(_pprofile_lut[j].label);
				break;
			}
		}

		if (j == PPROFILES) {
			ppa->ppe[i].profile_name = strdup(buff);
			ppa->ppe[i].label = strdup(buff);
		}
	}

	return 0;
}


int ppapplet_init(struct PowerProfileStruct *ppa) {
	ppa->icon = gtk_status_icon_new_from_stock(GTK_STOCK_PREFERENCES);

	if (ppapplet_profiles_detect(ppa) < 0)
		fprintf(stderr, "No power profiles detected\n"), exit(1);
	g_signal_connect(G_OBJECT(ppa->icon), "activate", G_CALLBACK(ppapplet_menu_spawn), ppa);

	return 0;
}


int main(int argc, char **argv) {

	if ((ppa_state = malloc(sizeof(*ppa_state))) == NULL) {
		fprintf(stderr, "Unable to malloc(%i), probably out of RAM\n", (int) sizeof(ppa_state));
		return 1;
	}


	memset(ppa_state, 0, sizeof(*ppa_state));

	gtk_init(&argc, &argv);
	if (ppapplet_init(ppa_state) < 0)
		return -1;
	
	if (argc >= 2 && strcmp(argv[1], "nokill"))
		ppapplet_make_is_alone();
	gtk_main();

	return 0;
}
