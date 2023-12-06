
namespace screen {
	extern Base* Debug;
	extern Base* Menu;
	extern Base* Settings;
	extern Base* ServerList;
	extern Base* WorldList;
	extern Base* Tutorial;

	// should not be accessed directly!
	extern Base* Gameplay;

	class Windowed : public Base {
		public:
		GuiWindowCtx win;	
		public:
		bool collision(Vector2) {
			return GuiWindowCollision(&win);
		}
		void update(float dt) {
			GuiWindowMove(&win);
			if (GuiWindowCollision(&win) || win.moving) {
				manager->refreshGui();
			};
		}
	};
};
