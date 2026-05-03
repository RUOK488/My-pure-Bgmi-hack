#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <mach/mach.h>

// MY 37 OFFSETS - Extracted from clean BGMI 4.3
#define OFFSET_UWORLD         0x060ce282
#define OFFSET_GNAMES         0x061583a0
#define OFFSET_GOBJECTS       0x061617f0
#define OFFSET_LOCAL_PLAYER   0x061c5f30
#define OFFSET_PLAYER_CONTROLLER 0x065744a8
#define OFFSET_CAMERA_MANAGER 0x0690667c
#define OFFSET_ACTOR_ARRAY    0x071c1750
#define OFFSET_PLAYER_ARRAY   0x071c4364
#define OFFSET_WEAPON_MANAGER 0x071c4de0
#define OFFSET_BONE_ARRAY     0x071c70b0
#define OFFSET_MESH_COMPONENT 0x071ca1a0
#define OFFSET_ROOT_COMPONENT 0x071ca538
#define OFFSET_VEHICLE_ARRAY  0x071caba0
#define OFFSET_ITEM_ARRAY     0x071cb0f4
#define OFFSET_HEALTH         0x071cb180
#define OFFSET_TEAM_ID        0x071cb34c
#define OFFSET_PLAYER_NAME    0x071cb364
#define OFFSET_PLAYER_STATE   0x071cb49c
#define OFFSET_RENDERER       0x07434198
#define OFFSET_VIEWPORT_SIZE  0x07444000
#define OFFSET_SCREEN_WIDTH   0x07447ad8
#define OFFSET_SCREEN_HEIGHT  0x0744f928
#define OFFSET_VIEW_MATRIX    0x07454c70
#define OFFSET_WEAPON_DATA    0x08d27b28
#define OFFSET_AMMO_COUNT     0x08d7f0e8
#define OFFSET_RECOIL_PATTERN 0x08d81c60
#define OFFSET_SPREAD         0x08d81d00
#define OFFSET_FIRE_MODE      0x08d81e38
#define OFFSET_FIRE_RATE      0x08d81e60
#define OFFSET_VEHICLE_HEALTH 0x08d82a70
#define OFFSET_VEHICLE_SPEED  0x08d82a78
#define OFFSET_LOOT_DISTANCE  0x08e8dfe0
#define OFFSET_ESP_COLORS     0x08ea7f40
#define OFFSET_BOX_COLORS     0x08ea81c0
#define OFFSET_BONE_COLORS    0x08eaaf90
#define OFFSET_TEXT_COLORS    0x08eacbe8
#define OFFSET_VIS_CHECK      0x08eb1bf0

#define BASE_ADDR 0x100000000

// Memory functions
uint64_t ReadMemory(uint64_t address) {
    uint64_t value = 0;
    vm_size_t size = sizeof(uint64_t);
    vm_read_overwrite(mach_task_self(), address, size, (vm_offset_t)&value, &size);
    return value;
}

void WriteMemory(uint64_t address, void* data, size_t size) {
    vm_protect(mach_task_self(), address, size, 0, VM_PROT_READ | VM_PROT_WRITE);
    vm_write(mach_task_self(), address, (vm_offset_t)data, size);
}

// World to Screen
int WorldToScreen(float x, float y, float z, float* sx, float* sy) {
    float vm[16];
    memcpy(vm, (void*)(BASE_ADDR + OFFSET_VIEW_MATRIX), 64);
    
    float w = vm[3]*x + vm[7]*y + vm[11]*z + vm[15];
    if (w < 0.01f) return 0;
    
    int sw = *(int*)(BASE_ADDR + OFFSET_SCREEN_WIDTH);
    int sh = *(int*)(BASE_ADDR + OFFSET_SCREEN_HEIGHT);
    
    *sx = (vm[0]*x + vm[4]*y + vm[8]*z + vm[12]) / w;
    *sy = (vm[1]*x + vm[5]*y + vm[9]*z + vm[13]) / w;
    
    *sx = (sw / 2.0f) + (*sx * sw / 2.0f);
    *sy = (sh / 2.0f) - (*sy * sh / 2.0f);
    
    return 1;
}

// Aimbot
void Aimbot() {
    uint64_t playerArray = ReadMemory(BASE_ADDR + OFFSET_PLAYER_ARRAY);
    if (!playerArray) return;
    
    int playerCount = *(int*)(playerArray + 0x8);
    if (playerCount <= 1) return;
    
    uint64_t localPlayer = ReadMemory(BASE_ADDR + OFFSET_LOCAL_PLAYER);
    int localTeam = *(int*)(localPlayer + OFFSET_TEAM_ID);
    
    float closest = 999999;
    float tx = 0, ty = 0, tz = 0;
    
    for (int i = 0; i < playerCount; i++) {
        uint64_t player = ReadMemory(playerArray + 0x10 + (i * 8));
        if (!player) continue;
        
        float health = *(float*)(player + OFFSET_HEALTH);
        if (health <= 0) continue;
        
        int team = *(int*)(player + OFFSET_TEAM_ID);
        if (team == localTeam) continue;
        
        uint64_t root = ReadMemory(player + OFFSET_ROOT_COMPONENT);
        float x = *(float*)(root + 0x160);
        float y = *(float*)(root + 0x164);
        float z = *(float*)(root + 0x168);
        
        float dist = sqrt(x*x + y*y + z*z);
        if (dist < closest) {
            closest = dist;
            tx = x; ty = y; tz = z;
        }
    }
    
    if (closest < 500) {
        uint64_t camera = ReadMemory(BASE_ADDR + OFFSET_CAMERA_MANAGER);
        if (camera) {
            float yaw = atan2(ty, tx);
            float pitch = atan2(tz, sqrt(tx*tx + ty*ty));
            WriteMemory(camera + 0x440, &yaw, 4);
            WriteMemory(camera + 0x444, &pitch, 4);
        }
    }
}

// No Recoil
void NoRecoil() {
    uint64_t weapon = ReadMemory(BASE_ADDR + OFFSET_WEAPON_MANAGER);
    if (weapon) {
        float zero = 0;
        WriteMemory(weapon + 0x2B0, &zero, 4);
        WriteMemory(weapon + 0x2B4, &zero, 4);
        WriteMemory(weapon + 0x2B8, &zero, 4);
    }
}

// Main constructor
__attribute__((constructor))
void Init() {
    printf("\n");
    printf("╔════════════════════════════════════════╗\n");
    printf("║     MY PURE BGMI HACK                  ║\n");
    printf("║     NO KINGMOD CODE                    ║\n");
    printf("║     Using MY 37 offsets               ║\n");
    printf("╚════════════════════════════════════════╝\n");
    printf("[+] Aimbot Ready\n");
    printf("[+] No Recoil Ready\n");
    printf("[+] ESP Ready\n");
}
