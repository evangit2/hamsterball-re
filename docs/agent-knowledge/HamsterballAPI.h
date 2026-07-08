#pragma once
#include <windows.h>
#include <math.h>
#include <cstdint>
#include <cstddef>
#include <dinput.h>
#include <cstdio>
#define DIRECTINPUT_VERSION 0x0800

#define HAMSTERBALL_API_VERSION 1

struct Collision;
class HamsterballAPI;

#pragma pack(push, 1)
struct Color {
	float r, g, b, a;
	Color() : r(1.0f), g(1.0f), b(1.0f), a(1.0f) {}
	Color(float _r, float _g, float _b, float _a = 1.0f) : r(_r), g(_g), b(_b), a(_a) {}
};

struct CustomButton {
	const char* id;
	const char* displayText;
	bool defaultState = false;
	const char* trueText = "YES";
	const char* falseText = "NO";
	Color color = Color();
	CustomButton() = default;
	CustomButton(const char* id, const char* displayText) : id(id), displayText(displayText) {}
};

struct CustomSlider {
	const char* id;
	const char* displayText;
	float startingValue;
	float stepSize = .1;
	int decimalPlaces = 2;
	float lowerBound = -INFINITY;
	float upperBound = INFINITY;
	const char* unitName = "";
	Color color;
	CustomSlider() = default;
	CustomSlider(const char* id, const char* displayText, float startingValue) :
		id(id), displayText(displayText), startingValue(startingValue) {}
};

struct CustomText {
	void* font;
	int x = 0;
	int y = 0;
	bool enable_shadow = true;
	int shadow_x = 5;
	int shadow_y = 5;
	Color text_color = Color(1, 1, 1, 1);
	Color shadow_color = Color(0, 0, 0, 1);
	CustomText() = default;
	CustomText(void* font, int x, int y, Color text_color, bool enable_shadow) :
		font(font), x(x), y(y), text_color(text_color), enable_shadow(enable_shadow) {}
};

struct CustomControl {
	int dikCode;
	bool requiresCtrl = false;
	CustomControl() = default;
	CustomControl(int dikCode) : dikCode(dikCode) {}
	CustomControl(int dikCode, bool requiresCtrl) : dikCode(dikCode), requiresCtrl(requiresCtrl) {}
};
#pragma pack(pop)

struct PhysicsObject;
struct App;
struct Ball;
struct Scene;
struct Vec3;
struct PhysicsConstants;
struct Sounds;
struct Fonts;

class IModAPI {
public:
	virtual ~IModAPI() {}
	virtual void RegisterCustomHook(DWORD targetAddress, void* hookFunction, void** original) = 0;
	virtual void RegisterCustomControl(const char* controlID, CustomControl defaultControl) = 0;
	virtual CustomControl GetCustomControlKey(const char* controlID) = 0;
	virtual bool IsKeyDown(int dik) = 0;
	virtual bool WasKeyPressed(int dik) = 0;
	virtual bool WasKeyReleased(int dik) = 0;
	virtual bool IsControlDown(const char* controlID) = 0;
	virtual bool WasControlPressed(const char* controlID) = 0;
	virtual bool WasControlReleased(const char* controlID) = 0;
	virtual void CreateToggleButton(const CustomButton& button, HamsterballAPI* modInstance) = 0;
	virtual void CreateSlider(const CustomSlider& slider, HamsterballAPI* modInstance) = 0;
	virtual void PatchMemory(DWORD address, const char* bytes, size_t size) = 0;
	virtual void UnlockAll() = 0;
	virtual void LockAll() = 0;
	virtual bool QuitGame() = 0;
	virtual bool SaveConfig() = 0;
	virtual void ApplyForce(Ball* ball, float x, float y, float z, float magnitude) = 0;
	virtual void SetSpeed(Ball* ball, float mult) = 0;
	virtual bool GetButtonState(const char* id) = 0;
	virtual float GetSliderState(const char* id) = 0;
	virtual Ball* GetPlayer() = 0;
	virtual Ball* GetPlayer2() = 0;
	virtual Ball* GetPlayer3() = 0;
	virtual Ball* GetPlayer4() = 0;
	virtual Ball** GetEnemies(size_t* enemyCount) = 0;
	virtual PhysicsObject* GetPhysicsObj() = 0;
	virtual Scene* GetScene() = 0;
	virtual DWORD GetGameBaseAddress() = 0;
	virtual App* GetApp() = 0;
	virtual void* AllocateMem(unsigned int size) = 0;
	virtual void CreateBadBall(Vec3 spawn_pos, Vec3 home_pos, float home_distance = 200, float chase_distance = 300, float radius = 35, float spin_distance = 45) = 0;
	virtual void ReloadIniFile() = 0;
	virtual int GetTimerTime() = 0;
	virtual void SetTimerTime(int time) = 0;
	virtual Vec3 LevelRaycastVec(Vec3 position, Vec3 direction, float radius) = 0;
	virtual bool LevelRaycastHit(Vec3 position, Vec3 direction, float radius, float max_dist = -1) = 0;
	virtual PhysicsConstants* GetPhysicsConstants() = 0;
	virtual void PlaySoundEffect(void* soundEffect, float volume) = 0;
	virtual void Play3dSoundEffect(void* soundEffect, Vec3 position, float volume) = 0;
	virtual void ShowBallMessage(Ball* ball, char* message) = 0;
	virtual void RespawnPlayer(Ball* player) = 0;
	virtual void DrawCustomText(const char* text, const CustomText& params) = 0;
	virtual void DrawTimedMessage(const char* text, const CustomText& params, float messageDuration) = 0;
	virtual float GetBallSpeed(Ball* ball) = 0;
	virtual void ShatterBall(Ball* ball) = 0;
};

class HamsterballAPI {
public:
	virtual ~HamsterballAPI() {}
	virtual const char* GetModName() = 0;
	virtual const char* GetAuthorName() = 0;
	virtual int GetApiVersion() = 0;
	virtual const char* GetContributors() { return ""; }
	virtual void Initialize(IModAPI* loader) {}
	virtual void onBallUpdate(Ball* ball) {}
	virtual void onRenderApply(void* this_ptr, float* viewMatrix) {}
	virtual void onButtonToggle(const char* buttonId, bool newState) {}
	virtual void onSliderChange(const char* sliderId, float newValue) {}
	virtual void onGameUpdate() {}
	virtual void onEventPlaneCollide(Ball* colliding_ball, char* eventPlaneID) {}
	virtual void onTextRenderLoop() {}
	virtual void onBallBump(Ball* ball1, Ball* ball2) {}
	virtual void onSceneEnd() {}
	virtual void onLevelStart() {}
};

typedef HamsterballAPI* (*CreateModFunct)();

struct Vec3 {
	float x, y, z;
	Vec3(float _x, float _y, float _z) : x(_x), y(_y), z(_z) {}
};

inline Vec3 Subtract(Vec3 a, Vec3 b) { return Vec3(a.x - b.x, a.y - b.y, a.z - b.z); }
inline Vec3 Cross(Vec3 a, Vec3 b) {
	return Vec3(a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x);
}
inline float Dot(Vec3 a, Vec3 b) { return a.x * b.x + a.y * b.y + a.z * b.z; }
inline Vec3 Normalize(Vec3 v) {
	float length = sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);
	if (length == 0.0f) return Vec3(0, 0, 0);
	return Vec3(v.x / length, v.y / length, v.z / length);
}
inline void BuildCustomViewMatrix(float* outMatrix, Vec3 eye, Vec3 target, Vec3 up) {
	Vec3 zaxis = Normalize(Subtract(target, eye));
	Vec3 xaxis = Normalize(Cross(up, zaxis));
	Vec3 yaxis = Cross(zaxis, xaxis);
	outMatrix[0] = xaxis.x;           outMatrix[1] = yaxis.x;           outMatrix[2] = zaxis.x;           outMatrix[3] = 0.0f;
	outMatrix[4] = xaxis.y;           outMatrix[5] = yaxis.y;           outMatrix[6] = zaxis.y;           outMatrix[7] = 0.0f;
	outMatrix[8] = xaxis.z;           outMatrix[9] = yaxis.z;           outMatrix[10] = zaxis.z;          outMatrix[11] = 0.0f;
	outMatrix[12] = -Dot(xaxis, eye); outMatrix[13] = -Dot(yaxis, eye); outMatrix[14] = -Dot(zaxis, eye); outMatrix[15] = 1.0f;
}

template <typename ReturnType = void, typename... Args>
ReturnType Call(DWORD offset, Args... args) {
	DWORD realAddress = (DWORD)GetModuleHandle(NULL) + offset;
	typedef ReturnType(__cdecl* GameFunc)(Args...);
	GameFunc func = (GameFunc)realAddress;
	return func(args...);
}
template <typename ReturnType = void, class ObjectType, typename... Args>
ReturnType CallMethod(DWORD offset, ObjectType* objPointer, Args... args) {
	DWORD realAddress = (DWORD)GetModuleHandle(NULL) + offset;
	typedef ReturnType(__thiscall* GameFunc)(ObjectType*, Args...);
	GameFunc func = (GameFunc)realAddress;
	return func(objPointer, args...);
}
template <typename ReturnType = void, typename... Args>
ReturnType CallFast(DWORD offset, Args... args) {
	DWORD realAddress = (DWORD)GetModuleHandle(NULL) + offset;
	typedef ReturnType(__fastcall* GameFunc)(Args...);
	GameFunc func = (GameFunc)realAddress;
	return func(args...);
}

#pragma pack(push, 1)
struct PhysicsConstants {
	float unknown;
	float dizzyForceMult;
	std::uint8_t pad_08[0x04];
	float glassForceMult;
	float unknown2;
	std::uint8_t pad_14[0x04];
	float unknown3;
	std::uint8_t pad_1C[0x18];
	float hamsterSize;
	std::uint8_t pad_38[0x48];
	float unknown4;
	std::uint8_t pad_84[0x04];
	float cameraDamping;
	std::uint8_t pad_8C[0x4CF484 - 0x4CF3F4];
	float unknown5;
	std::uint8_t pad_120[4];
	float unknown6;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct Ball {
	void** vtable;
	std::uint8_t pad_004[0x014 - 0x004];
	Scene* scene;
	int playerID;
	std::uint8_t pad_01C[0x158 - 0x01C];
	float prev_pos_x;
	float prev_pos_y;
	float prev_pos_z;
	float pos_x;
	float pos_y;
	float pos_z;
	std::uint8_t pad_170[0x17C - 0x170];
	float accel_x;
	float accel_y;
	float accel_z;
	float max_speed;
	std::uint8_t pad_18C[0x190 - 0x18C];
	float facing_angle;
	std::uint8_t pad_194[0x1A0 - 0x194];
	float speed_mult;
	PhysicsObject* physics_object;
	float gravity_vec[3];
	std::uint8_t pad_1B4[0x1C8 - 0x1B4];
	float ball_outline_opacity;
	std::uint8_t pad_1CC[0x260 - 0x1CC];
	bool boost_hit_flag;
	std::uint8_t pad_261[0x264 - 0x261];
	std::uint8_t rumble_timer1[0x14];
	float bounciness;
	std::uint8_t pad_27C[0x284 - 0x27C];
	float radius;
	std::uint8_t pad_288[0x290 - 0x288];
	std::uint8_t rumble_timer2[0x14];
	float spin_rate;
	std::uint8_t pad_2A8[0x2BC - 0x2A8];
	float force_x;
	float force_y;
	float force_z;
	std::uint8_t pad_2C8[0x2CC - 0x2C8];
	bool disable_ball;
	std::uint8_t pad_2CD[0x2D4 - 0x2CD];
	bool render_jitter; // +0x2D4 — set by vacuum (CollisionFace_Update) to enable CPUID-based random jitter in render function (FUN_00403DB8)
	std::uint8_t pad_2D5[0x2DC - 0x2D5];
	float checkpoint_x;
	float checkpoint_y;
	float checkpoint_z;
	bool event_checkpoint_flag;
	bool unknown;
	std::uint8_t pad_2EA[0x310 - 0x2EA];
	bool state_active;
	std::uint8_t pad_311[0x700 - 0x311];
	int sound_3d_handle;
	std::uint8_t pad_704[0x748 - 0x704];
	int gravity_type;
	std::uint8_t pad_74C[0x768 - 0x74C];
	bool cam_active;
	std::uint8_t pad_769[0xC4C - 0x769];
	bool low_gravity_mode;
	std::uint8_t pad_C4D[0xC50 - 0xC4D];
	float burn_amount;
	std::uint8_t pad_C54[0xC60 - 0xC54];
	float home_position_x;
	float home_position_y;
	float home_position_z;
	float home_distance;
	float chase_distance;
	bool is_badball_on_screen;
	std::uint8_t pad_C75[0xC78 - 0xC75];
	float spin_counter;
	float spin_distance;
	std::uint8_t pad_C80[0xC88 - 0xC80];
	float world_matrix[16];
};
#pragma pack(pop)

#pragma pack(push, 1)
struct Scene {
	void** vtable;
	std::uint8_t pad_004[0x014 - 0x004];
	App* owner_app;
	std::uint8_t pad_018[0x868 - 0x018];
	char* name;
	std::uint8_t pad_86C[0x8AC - 0x86C];
	void* level_ptr;
	Collision* collision_mesh;
	std::uint8_t pad_8B4[0x29BC - 0x8B4];
	float camera_angle;
	float camera_distance;
	std::uint8_t pad_29C4[0x29D0 - 0x29C4];
	Ball* current_ball_ptr;
	void* ball_list;
	int ball_list_count;
	std::uint8_t pad_29DC[0x2DE0 - 0x29DC];
	Ball** ball_array;
	std::uint8_t pad_2DE4[0x3620 - 0x2DE4];
	int frame_counter;
	std::uint8_t pad_3624[0x362C - 0x3624];
	void* player_list;
	int player_count;
	std::uint8_t pad_3634[0x3F1C - 0x3634];
	int path_follow_mode;
	void* cam_path_object;
	float cam_path_position;
	std::uint8_t pad_3F28[0x3F2C - 0x3F28];
	float cam_time_to_zoom;
	std::uint8_t pad_3F30[0x434C - 0x3F30];
	float cam_offset_x;
	float cam_offset_y;
	float cam_offset_z;
	std::uint8_t pad_4358[0x47AC - 0x4358];
	int arena_timer;
	bool timer_started;
	std::uint8_t pad_47B1[0x47B4 - 0x47B1];
	int p1Score;
	int p2Score;
	int p3Score;
	int p4Score;
	bool weird_camera;
	bool is_tiebreaker;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct PhysicsObject {
	void** vtable;
	std::uint8_t pad_004[0x010 - 0x004];
	Ball* owner_ball;
	std::uint8_t pad_014[0x01C - 0x014];
	int collision_count;
	std::uint8_t pad020[0x424 - 0x020];
	void* collision_arr;
	std::uint8_t pad428[0xC60 - 0x428];
	int unknown;
	float speed_scalar;
	float friction;
	std::uint8_t pad_0C6C[0x0C7C - 0x0C6C];
	bool noclip;
	std::uint8_t pad_0C7D[0x0C8C - 0x0C7D];
	float gravity_x;
	float gravity_y;
	float gravity_z;
	std::uint8_t pad_0C98[0x0CA4 - 0x0C98];
	float velocity_x;
	float velocity_y;
	float velocity_z;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct Sounds {
	void* collide;
	void* roll;
	void* whistle;
	void* bumper;
	void* ballbreak;
	void* ballbreaksmall;
	void* thwomp;
	void* snap;
	void* popup;
	void* dropin;
	void* dropinshort;
	void* popout;
	void* pipebump1;
	void* pipebump2;
	void* pipebump3;
	void* gearclank;
	void* bridgeslam;
	void* platformtick;
	void* gluestuck;
	void* bubble1;
	void* bubble2;
	void* wheelcreak;
	void* catapult;
	void* trapdoor;
	void* fwing;
	void* clink;
	void* whoosh;
	void* chomp;
	void* fan_start;
	void* fan_blow;
	void* crack;
	void* crumble;
	void* sawstartup;
	void* sawcut;
	void* minipop;
	void* bell;
	void* zip;
	void* ting;
	void* shrink;
	void* grow;
	void* tweet;
	void* creakyplatform;
	void* wubba;
	void* saw;
	void* sawspeedy;
	void* dawgstep1;
	void* dawgstep2;
	void* dawgsmash;
	void* sizzle;
	void* explode;
	void* vac_o_sux;
	void* speedcylinder;
	void* bonuspop;
	void* buzzbonus;
	void* breakbridge;
	void* unlock;
	void* NeonRide;
	void* NeonFlicker;
	void* ZoopDown;
	void* LightsOff;
	void* GlassBonus;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct Fonts {
	void* showcardGothic28;
	void* showcardGothic14;
	void* showcardGothic16;
	void* arialNarrow12bold;
	void* showcardGothic72;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct App {
	void** vtable;
	std::uint8_t pad_004[0x158 - 0x004];
	bool isFullscreen;
	bool quitFlag;
	bool isGameFocused;
	std::uint8_t pad_15B[0x1];
	int gameWidth;
	int gameHeight;
	std::uint8_t pad_164[0x10];
	void* graphics;
	std::uint8_t pad_178[0x17C - 0x178];
	void* audioSystem;
	void* inputHandler;
	void* gameUpdateObj;
	std::uint8_t pad_188[0x238 - 0x188];
	bool rightButtonPauseEnabled;
	std::uint8_t pad_239[0x318 - 0x239];
	Fonts fonts;
	std::uint8_t pad_32C[0x43C - 0x32C];
	Sounds sounds;
	std::uint8_t pad_530[0x534 - 0x530];
	void* musicHandle;
	void* musicChannel1;
	void* musicChannel2;
	std::uint8_t pad_540[0x10];
	void* gameMode1;
	void* gameMode2;
	void* gameMode3;
	void* gameMode4;
	std::uint8_t pad_560[0x84C - 0x560];
	float sensitivity;
	bool unlockMirrorTournament;
	bool unlockDizzyRace;
	bool unlockTowerRace;
	bool unlockUpRace;
	bool unlockExpertRace;
	bool unlockOddRace;
	bool unlockToobRace;
	bool unlockWobblyRace;
	bool unlockSkyRace;
	bool unlockMasterRace;
	bool unlockDizzyArena;
	bool unlockTowerArena;
	bool unlockUpArena;
	bool unlockExpertArena;
	bool unlockOddArena;
	bool unlockToobArena;
	bool unlockWobblyArena;
	bool unlockSkyArena;
	bool unlockMasterArena;
	bool unlockNeonRace;
	bool unlockGlassRace;
	bool unlockImpossibleRace;
	bool unlockNeonArena;
	bool unlockGlassArena;
	bool unlockImpossibleArena;
	std::uint8_t pad_869[0x86C - 0x869];
	std::uint8_t bestTimes[0x50];
	std::uint8_t medals[0x50];
	std::uint8_t pad_90C[0xB28 - 0x90C];
	DWORD p2Controller1;
	DWORD p2Controller2;
	DWORD p2Controller3;
	DWORD p2Controller4;
};
#pragma pack(pop)
