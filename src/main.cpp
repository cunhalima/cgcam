// http://www.flipcode.com/archives/Light_Mapping_Theory_and_Implementation.shtml
// http://http.developer.nvidia.com/GPUGems2/gpugems2_chapter39.html

#include <cstring>
#include "defs.h"

Options_t opt;
bool  g_QuitFlag;

static cvar_t *var_minmilli;

// ===== console functions ======
static void quit_f() {
	g_QuitFlag = true;
}

// =====

char *my_strdup(const char *original) {
	if (original == NULL) {
		return NULL;
	}
	char *copy;
	size_t len = strlen(original) + 1;
	copy = new char[len];
	memcpy(copy, original, len);
	return copy;
}

void readoptions(int argc, char **argv) {
	opt.argc = argc;
	opt.argv = argv;
	opt.remoteHost = "127.0.0.1";
    for (int i = 1; i < argc; i++) {
		// dedicated
		if (argv[i][0] == '-') {
			switch(argv[i][1]) {
			case '0':
			case '1':
			case '2':
			case '3':
				opt.winpos = argv[i][1] - '0';
				break;
			case 'd':
				opt.server = true;
				opt.client = false;
				break;
			case 'c':
				opt.server = false;
				opt.client = true;
				if (i + 1 < argc && argv[i + 1][0] != '-') {
					opt.remoteHost = argv[i + 1];
					i++;
				}
				break;
			case 'l':
				opt.nolog = true;
				break;
			}
		}
    }
}

int main(int argc, char **argv) {
	memset(&opt, 0, sizeof(opt));
	g_QuitFlag = false;
	SYS_Init();
	SYS_ReadOptions(argc, argv);
	// force dedicated
	//opt.client = false;
	con_init();
    readoptions(argc, argv);
	// tirar ISSO!!
	//if (opt.client) {
	//	opt.server = false;
	//}
	if (opt.client) {
		if (!SYS_InitVideo()) {
			return -1;
		}
		RE_2DInit();
		RE_VidconInit();
	} else {
		Syscon_Init();
	}
	// Console is OK from now on
	cfun_register(quit_f, "quit");
	var_minmilli = cvar_register("minmilli", "10");
	net_init();
	if (opt.server) {
		SV_Init();
	}
	if (opt.client) {
		CL_Init();
		if (!SND_Init()) {
			SYS_Cleanup();
			return -1;
		}
		//MD2_Init();
		MAP_Init();
		MDL_Init();
		RE_ParticlesInit();
		T01_Init();
		//T03_Init();
		CL_GameInit();
	}
	GL_DEBUG();

	// tirar isso!!
	if (opt.server) {
		SV_Start();
	}
	if (opt.client) {
		CL_Connect(opt.remoteHost);
	}

	do {
		GL_DEBUG();
		SYS_UpdateTime(var_minmilli->integer);
		if (opt.server) {
			SV_Frame();
		}
		if (opt.client) {
			SYS_PollEvents();
		} else {
			Syscon_ProcessEvents();
		}
		con_run();
		if (opt.client) {
			CL_Frame();
		}
		//if (opt.client) {
		//	//T01_Frame();
		//}
		//if (opt.client) {
			//T03_Frame();
		//}
	} while(SYS_IsRunning());
	if (opt.client) {
		//T03_Cleanup();
		CL_GameCleanup();
		T01_Cleanup();
		RE_ParticlesCleanup();
		//MD2_Cleanup();
		MDL_Cleanup();
		MAP_Cleanup();
		SND_Cleanup();
		CL_Cleanup();
		RE_VidconCleanup();
		RE_2DCleanup();
	} else {
		Syscon_Cleanup();
	}
	if (opt.server) {
		SV_Cleanup();
	}
	net_cleanup();
	SYS_Cleanup();	// shuts down video if necessary
	con_cleanup();
	return 0;
}
