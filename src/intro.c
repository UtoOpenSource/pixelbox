#include "raygui.h"
#include "pixel.h"
#include <raylib.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include <time.h>

extern char* world_db_path;

int CreationDialog() {
	bool stat = false;
	char seed     [64] = {0};
	char filename [64] = {0};
	int  mode          = 0;
	int  input = 0, input2 = 0;

	snprintf(seed, 64, "%li", time(NULL)*clock());

	while (!(stat = WindowShouldClose())) {
		BeginDrawing();
		ClearBackground(BLACK);

		Rectangle rec = {
			GetScreenWidth()/4, GetScreenHeight()/4,
			GetScreenWidth()/2, GetScreenHeight()/2
		};
		GuiPanel(rec, "Create new world");

		rec.x += 5;
		rec.width -= 10;
		rec.y += 40;
		rec.height = 25;

		rec.width /= 2;

		GuiLine(rec, "World Name");

		rec.y += 30;
		GuiLine(rec, "Seed ");

		rec.y += 30;
		GuiLine(rec, "Terrain");
	
		rec.y -= 60;
		rec.x += rec.width;
		if (GuiTextBox(rec, filename, 63, input)) input = !input;

		rec.y += 30;
		if (GuiTextBox(rec, seed, 63, input2)) input2 = !input2;

		rec.y += 30;
		mode = GuiToggle(rec, mode ? "Flat" : "Normal", mode);

		rec.y += 80;
		rec.x -= rec.width - 2;
		if (GuiButton(rec, "Abort")) {
			abort(); // that's was not joke :D
		};

		rec.x += rec.width + 2;
		if (GuiButton(rec, "Done")) {
			const char* src = TextFormat("./saves/%s.db", filename);
			uint64_t len = strlen(src)+1;
			world_db_path = malloc(len);
			assert(world_db_path);
			memcpy(world_db_path, src, len);
			openWorld(world_db_path);

			uint64_t seedr = 0;
			sscanf(seed, "%li", &seedr);
			setWorldSeed(seedr);

			World.mode = mode;
			perror("OK");

			EndDrawing();
			break;
		}

		EndDrawing();
	}
	return stat;
}

// world selection dialog
int IntroDialog() {
	bool stat = false;
	
	FilePathList files = LoadDirectoryFilesEx("./saves", ".db", false);

	const char* MEMORY_STR = ":memory:";
	const char* EMPITY_STR = "-- EMPITY --";

	const char** list = (const char**)calloc(sizeof(char*), files.count + 3);
	assert(list);
	memcpy(list+2, files.paths, files.count*sizeof(char*));
	list[0] = EMPITY_STR;
	list[1] = MEMORY_STR;
	int list_len = files.count+2, scroll = 0, select = 0;

	while (!(stat = WindowShouldClose())) {
		BeginDrawing();
		ClearBackground(BLACK);
		GuiEnable();

		Rectangle rec = {
			GetScreenWidth()/4, GetScreenHeight()/4,
			GetScreenWidth()/2, GetScreenHeight()/2
		};
		GuiPanel(rec, "Select world file");
		rec.x += 5;
		rec.width -= 10;
		rec.y += 30;
		rec.height -= 35*3;

		select = GuiListViewEx(rec, list, list_len, NULL, &scroll, select);

		rec.y += rec.height + 5;
		rec.height = 25;
		rec.width  = rec.width / 2 - 2;

		int active = select >= 0 ? select : 0;

		if (select < 0 || list[active] == MEMORY_STR
				|| list[active] == EMPITY_STR) GuiDisable();

		if (GuiButton(rec, "Delete")) {
			remove(list[active]);
			list[active] = EMPITY_STR;
		}
		
		if (select >= 0) GuiEnable();

		rec.x += rec.width + 4;
		if (GuiButton(rec, (list[active] == EMPITY_STR) ? "Create" : "Open")) {
			perror("OK");
			if (list[active] == EMPITY_STR) { // add file at first!
				EndDrawing();
			} else {
				uint64_t len = strlen(list[active]);
				if (world_db_path) free(world_db_path);
				world_db_path = malloc(len+1);
				assert(world_db_path);
				memcpy(world_db_path, list[active], len+1);
				EndDrawing();
			}
			break; // exit
		}

		rec.y += 25;
		GuiLine(rec, list[active]);

		EndDrawing();
	}
	free(list);
	UnloadDirectoryFiles(files);
	return stat;
}

