/* uKernel — modul-registry és runtime kontroll.
 *
 * A driver .so betöltésekor a module_init/module_exit makrók (ELF konstruktorok)
 * ide regisztrálják az init/exit függvényt. A uServer ezután lefuttatja őket. */
#include "ukernel/runtime.h"
#include <linux/kernel.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define MAX_MODULES 64

struct mod_slot {
	char  name[48];
	int  (*init)(void);
	void (*exit)(void);
	int   inited;
};

static struct mod_slot g_mods[MAX_MODULES];
static size_t g_nmods;

static struct mod_slot *find_or_add(const char *mod)
{
	for (size_t i = 0; i < g_nmods; i++)
		if (strcmp(g_mods[i].name, mod) == 0) return &g_mods[i];
	if (g_nmods >= MAX_MODULES) return &g_mods[MAX_MODULES - 1];
	struct mod_slot *s = &g_mods[g_nmods++];
	snprintf(s->name, sizeof(s->name), "%s", mod);
	return s;
}

void ukernel_register_init(const char *mod, int (*fn)(void))
{ find_or_add(mod)->init = fn; }

void ukernel_register_exit(const char *mod, void (*fn)(void))
{ find_or_add(mod)->exit = fn; }

size_t ukernel_module_count(void) { return g_nmods; }

const struct ukernel_module *ukernel_module_get(size_t i)
{
	static struct ukernel_module view;   /* nem reentráns, de a loader szekvenciális */
	if (i >= g_nmods) return NULL;
	view.name = g_mods[i].name;
	view.init = g_mods[i].init;
	view.exit = g_mods[i].exit;
	return &view;
}

/* a betöltött modul nevét a uksh `lsmod`-jának (UK_MODLIST, alapért. /tmp/uk_modules —
 * minden uk-processz: ukd a soros-drivereket, userver a wifit, FS a fájlrendszert).
 * Dedup: csak ha még nincs a fájlban (a perzisztens lista ne nőjön korlátlanul). */
static void modlist_add(const char *name)
{
	const char *p = getenv("UK_MODLIST"); if (!p || !*p) p = "/tmp/uk_modules";
	if (!strcmp(p, "off")) return;
	FILE *r = fopen(p, "r");
	if (r) { char ln[128]; size_t nl = strlen(name);
		while (fgets(ln, sizeof ln, r)) { ln[strcspn(ln, "\n|")] = 0; if (!strncmp(ln, name, nl) && !ln[nl]) { fclose(r); return; } }
		fclose(r); }
	FILE *f = fopen(p, "a"); if (!f) return;
	fprintf(f, "%s\n", name); fclose(f);
}

int ukernel_run_module_inits(void)
{
	for (size_t i = 0; i < g_nmods; i++) {
		if (g_mods[i].inited || !g_mods[i].init) continue;
		printk(KERN_INFO "uKernel: module_init(%s)\n", g_mods[i].name);
		int r = g_mods[i].init();
		if (r) { printk(KERN_ERR "uKernel: %s init -> %d\n", g_mods[i].name, r); return r; }
		g_mods[i].inited = 1;
		modlist_add(g_mods[i].name);
	}
	return 0;
}

void ukernel_run_module_exits(void)
{
	for (size_t i = g_nmods; i-- > 0; ) {
		if (!g_mods[i].inited || !g_mods[i].exit) continue;
		printk(KERN_INFO "uKernel: module_exit(%s)\n", g_mods[i].name);
		g_mods[i].exit();
		g_mods[i].inited = 0;
	}
}
