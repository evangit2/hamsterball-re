/* physics_objects v2 — pushable + TIPPING boxes for Physicus */
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <math.h>
typedef unsigned __int64 QWORD;
/* BASS proxy */
static HMODULE g_hRealBass=NULL; static void lazy_load_real_bass(void);
typedef int (__stdcall *BASS_ChannelSetAttributes_t)(DWORD,float,int,int); static BASS_ChannelSetAttributes_t real_BASS_ChannelSetAttributes=NULL;
__declspec(dllexport) int __stdcall BASS_ChannelSetAttributes(DWORD a,float b,int c,int d){lazy_load_real_bass(); if(real_BASS_ChannelSetAttributes) return real_BASS_ChannelSetAttributes(a,b,c,d); return 1;}
typedef int (__stdcall *BASS_MusicPlayEx_t)(DWORD,DWORD,BOOL); static BASS_MusicPlayEx_t real_BASS_MusicPlayEx=NULL;
__declspec(dllexport) int __stdcall BASS_MusicPlayEx(DWORD a,DWORD b,BOOL c){lazy_load_real_bass(); if(real_BASS_MusicPlayEx) return real_BASS_MusicPlayEx(a,b,c); return 1;}
typedef int (__stdcall *BASS_MusicLoad_t)(int,const char*,QWORD,DWORD,DWORD,DWORD); static BASS_MusicLoad_t real_BASS_MusicLoad=NULL;
__declspec(dllexport) int __stdcall BASS_MusicLoad(int a,const char* b,QWORD c,DWORD d,DWORD e,DWORD f){lazy_load_real_bass(); if(real_BASS_MusicLoad) return real_BASS_MusicLoad(a,b,c,d,e,f); return 0;}
typedef int (__stdcall *BASS_SampleLoad_t)(int,const char*,QWORD,DWORD,DWORD,DWORD); static BASS_SampleLoad_t real_BASS_SampleLoad=NULL;
__declspec(dllexport) int __stdcall BASS_SampleLoad(int a,const char* b,QWORD c,DWORD d,DWORD e,DWORD f){lazy_load_real_bass(); if(real_BASS_SampleLoad) return real_BASS_SampleLoad(a,b,c,d,e,f); return 0;}
typedef int (__stdcall *BASS_StreamCreateFile_t)(int,const char*,QWORD,QWORD,DWORD); static BASS_StreamCreateFile_t real_BASS_StreamCreateFile=NULL;
__declspec(dllexport) int __stdcall BASS_StreamCreateFile(int a,const char* b,QWORD c,QWORD d,DWORD e){lazy_load_real_bass(); if(real_BASS_StreamCreateFile) return real_BASS_StreamCreateFile(a,b,c,d,e); return 0;}
typedef int (__stdcall *BASS_SampleGetChannel_t)(DWORD,BOOL); static BASS_SampleGetChannel_t real_BASS_SampleGetChannel=NULL;
__declspec(dllexport) int __stdcall BASS_SampleGetChannel(DWORD a,BOOL b){lazy_load_real_bass(); if(real_BASS_SampleGetChannel) return real_BASS_SampleGetChannel(a,b); return 0;}
typedef int (__stdcall *BASS_ChannelPlay_t)(DWORD,BOOL); static BASS_ChannelPlay_t real_BASS_ChannelPlay=NULL;
__declspec(dllexport) int __stdcall BASS_ChannelPlay(DWORD a,BOOL b){lazy_load_real_bass(); if(real_BASS_ChannelPlay) return real_BASS_ChannelPlay(a,b); return 1;}
typedef int (__stdcall *BASS_ChannelStop_t)(DWORD); static BASS_ChannelStop_t real_BASS_ChannelStop=NULL;
__declspec(dllexport) int __stdcall BASS_ChannelStop(DWORD a){lazy_load_real_bass(); if(real_BASS_ChannelStop) return real_BASS_ChannelStop(a); return 1;}
typedef int (__stdcall *BASS_Init_t)(int,DWORD,DWORD,HWND,void*); static BASS_Init_t real_BASS_Init=NULL;
__declspec(dllexport) int __stdcall BASS_Init(int a,DWORD b,DWORD c,HWND d,void* e){lazy_load_real_bass(); if(real_BASS_Init) return real_BASS_Init(a,b,c,d,e); return 1;}
typedef int (__stdcall *BASS_Free_t)(void); static BASS_Free_t real_BASS_Free=NULL;
__declspec(dllexport) int __stdcall BASS_Free(void){lazy_load_real_bass(); if(real_BASS_Free) return real_BASS_Free(); return 1;}
typedef int (__stdcall *BASS_Stop_t)(void); static BASS_Stop_t real_BASS_Stop=NULL;
__declspec(dllexport) int __stdcall BASS_Stop(void){lazy_load_real_bass(); if(real_BASS_Stop) return real_BASS_Stop(); return 1;}
typedef int (__stdcall *BASS_Start_t)(void); static BASS_Start_t real_BASS_Start=NULL;
__declspec(dllexport) int __stdcall BASS_Start(void){lazy_load_real_bass(); if(real_BASS_Start) return real_BASS_Start(); return 1;}
typedef int (__stdcall *BASS_SetConfig_t)(DWORD,DWORD); static BASS_SetConfig_t real_BASS_SetConfig=NULL;
__declspec(dllexport) int __stdcall BASS_SetConfig(DWORD a,DWORD b){lazy_load_real_bass(); if(real_BASS_SetConfig) return real_BASS_SetConfig(a,b); return 1;}
typedef int (__stdcall *BASS_ErrorGetCode_t)(void); static BASS_ErrorGetCode_t real_BASS_ErrorGetCode=NULL;
__declspec(dllexport) int __stdcall BASS_ErrorGetCode(void){lazy_load_real_bass(); if(real_BASS_ErrorGetCode) return real_BASS_ErrorGetCode(); return 0;}

/* Game addresses */
#define GLOBAL_APP            0x005341E0
#define APP_BOARD             0x178
#define APP_QUIT_FLAG         0x159
#define BOARD_BALL_LIST       0x29D4
#define BOARD_SCENE_OBJ       0x8AC
#define LEVEL_SCENE_OBJ       0x480
#define SCENE_REF_COUNT       0x898
#define SCENE_REF_ITEMS       0xCA0
#define PAUSE_FLAG            0x874
#define BALL_PLAYERIDX        0x18
#define BALL_POS              0x164
#define BALL_RADIUS           0x284
#define ADDR_Mesh_FindClosestCollision 0x00465D90
#define FN_operator_new       0x004BA57B
typedef struct { float x,y,z; } Vec3;
typedef Vec3* (__thiscall *MeshRaycast_t)(void* cl, Vec3* out, Vec3 origin, Vec3 dir, float max_dist);
static MeshRaycast_t pfn_raycast = (MeshRaycast_t)ADDR_Mesh_FindClosestCollision;
#define FN_MeshNode_ctor      0x00471750
typedef void* (__cdecl *operator_new_t)(unsigned int);
typedef unsigned int (__thiscall *MeshNodeCtor_t)(void*,DWORD,const char*,char);
#define pfn_operator_new   ((operator_new_t)FN_operator_new)
#define pfn_MeshNode_ctor  ((MeshNodeCtor_t)FN_MeshNode_ctor)
#define MW_WORLD_MATRIX       0x04
#define MW_MESHBUFFER_LIST    0x2C
#define MB_COUNT(lb)    (*(DWORD*)((char*)(lb)+0x04))
#define MB_ITEMS(lb)    (*(DWORD**)((char*)(lb)+0x40C))
#define STRIP_COUNT(mb) (*(DWORD*)((char*)(mb)+0x10))
#define STRIP_ITEMS(mb) (*(float**)((char*)(mb)+0x418))
#define STRIDE_VERTS          3
#define VERT_STRIDE           8
#define MESH_RENDER_SLOT      18

/* Config */
#define CFG_GRAVITY         0.55f
#define CFG_MAXFALL         12.0f
#define CFG_FRICTION        0.90f
#define CFG_PUSHFORCE       0.30f
#define CFG_MAXPUSHVEL      2.2f
#define CFG_SNAPDIST        6.0f
#define CFG_CONTACTGAP      14.0f
#define CFG_ANG_DAMP_GROUNDED 0.88f
#define CFG_ANG_DAMP_AIR      0.995f
#define CFG_TIP_THRESHOLD     0.35f  /* rad ~20deg before we let it fall */

static float cfg_gravity     = CFG_GRAVITY;
static float cfg_maxfall     = CFG_MAXFALL;
static float cfg_friction    = CFG_FRICTION;
static float cfg_pushforce   = CFG_PUSHFORCE;
static float cfg_maxpushvel  = CFG_MAXPUSHVEL;
static float cfg_snapdist    = CFG_SNAPDIST;

static void get_dll_dir(char *out,int len){
    MEMORY_BASIC_INFORMATION mbi; HMODULE hm=NULL;
    if(VirtualQuery((LPCVOID)&get_dll_dir,&mbi,sizeof(mbi))&&mbi.AllocationBase) hm=(HMODULE)mbi.AllocationBase;
    if(!GetModuleFileNameA(hm,out,len)) out[0]=0; char *s=strrchr(out,'\\'); if(s) *(s+1)=0;
}
static void load_config(void){
    char path[MAX_PATH]; get_dll_dir(path,MAX_PATH); lstrcatA(path,"physics_objects.ini");
    FILE *f=fopen(path,"r");
    if(!f){ f=fopen(path,"w"); if(f){ fprintf(f,"# physics_objects v2 — push + tip\n"); fprintf(f,"Gravity=%.2f\n",CFG_GRAVITY); fprintf(f,"MaxFall=%.2f\n",CFG_MAXFALL); fprintf(f,"Friction=%.2f\n",CFG_FRICTION); fprintf(f,"PushForce=%.2f\n",CFG_PUSHFORCE); fprintf(f,"MaxPushVel=%.2f\n",CFG_MAXPUSHVEL); fclose(f);} return; }
    char line[256]; while(fgets(line,sizeof(line),f)){ char *h=strchr(line,'#'); if(h)*h=0; float v;
        if(sscanf(line," Gravity = %f",&v)==1&&v>0.f) cfg_gravity=v;
        else if(sscanf(line," MaxFall = %f",&v)==1&&v>0.f) cfg_maxfall=v;
        else if(sscanf(line," Friction = %f",&v)==1&&v>=0.5f&&v<=1.0f) cfg_friction=v;
        else if(sscanf(line," PushForce = %f",&v)==1&&v>0.f) cfg_pushforce=v;
        else if(sscanf(line," MaxPushVel = %f",&v)==1&&v>0.f) cfg_maxpushvel=v;
    } fclose(f);
}
static void ilog(const char *fmt,...){
    char buf[768]; va_list ap; va_start(ap,fmt); _vsnprintf(buf,sizeof(buf)-1,fmt,ap); va_end(ap); buf[sizeof(buf)-1]=0;
    char path[MAX_PATH]; get_dll_dir(path,MAX_PATH); lstrcatA(path,"physics_objects.log");
    FILE *f=fopen(path,"a"); if(f){ fputs(buf,f); fputc('\n',f); fclose(f); }
}

/* ── Object table ── */
#define MAX_OBJECTS 16
#define MAX_NAME    64
typedef struct {
    DWORD  meshworld;
    char   mesh_name[MAX_NAME];
    float  min[3], max[3]; /* current AABB */
    float  x,y,z;          /* center */
    float  vx,vy,vz;
    float  half[3];        /* half extents */
    float  qx,qy,qz,qw;    /* orientation */
    float  wx,wy,wz;       /* angular velocity */
    float  invInertia[3];  /* diagonal */
    float  home[3];
    int    grounded;
    struct { float *live; float *localPos; float *localNrm; int vertCount; } bufs[16];
    int    buf_count;
    DWORD  render_fn;
} PhysObj;
static DWORD g_shared_render_fn=0;
static PhysObj g_objs[MAX_OBJECTS];
static int g_obj_count=0;
static DWORD g_last_board=0;
static int g_level_done=0;
static BYTE *g_tick_cave=NULL;

static int game_is_quitting(void){ DWORD app=*(DWORD*)GLOBAL_APP; if(!app||app<0x10000||IsBadReadPtr((void*)app,0x160)) return 1; return *(BYTE*)(app+APP_QUIT_FLAG)!=0; }
static DWORD get_board(void){ DWORD app=*(DWORD*)GLOBAL_APP; if(!app||app<0x10000||IsBadReadPtr((void*)app,0x200)) return 0; DWORD b=*(DWORD*)(app+APP_BOARD); if(!b||b<0x10000||IsBadReadPtr((void*)b,0x3000)) return 0; return b; }
static DWORD get_player_ball(DWORD board){ if(!board) return 0; DWORD lb=board+BOARD_BALL_LIST; if(IsBadReadPtr((void*)lb,0x410)) return 0; int c=*(int*)(lb+0x04); if(c<1||c>64) return 0; DWORD items=*(DWORD*)(lb+0x40C); if(!items||IsBadReadPtr((void*)items,c*4)) return 0; for(int i=0;i<c;i++){ DWORD ball=*(DWORD*)(items+i*4); if(!ball||ball<0x10000||IsBadReadPtr((void*)ball,0x300)) continue; if(*(DWORD*)(ball+BALL_PLAYERIDX)==0) return ball; } return 0; }

/* ── Quaternion helpers ── */
static void quat_normalize(PhysObj *o){ float l=sqrtf(o->qx*o->qx+o->qy*o->qy+o->qz*o->qz+o->qw*o->qw); if(l>1e-6f){ o->qx/=l; o->qy/=l; o->qz/=l; o->qw/=l; } else { o->qx=0; o->qy=0; o->qz=0; o->qw=1; } }
static void quat_to_matrix(PhysObj *o,float *m){
    float x=o->qx,y=o->qy,z=o->qz,w=o->qw;
    float xx=x*x, yy=y*y, zz=z*z;
    float xy=x*y, xz=x*z, yz=y*z;
    float wx=w*x, wy=w*y, wz=w*z;
    m[0]=1-2*(yy+zz); m[1]=2*(xy-wz);   m[2]=2*(xz+wy);   m[3]=o->x;
    m[4]=2*(xy+wz);   m[5]=1-2*(xx+zz); m[6]=2*(yz-wx);   m[7]=o->y;
    m[8]=2*(xz-wy);   m[9]=2*(yz+wx);   m[10]=1-2*(xx+yy); m[11]=o->z;
    m[12]=0; m[13]=0; m[14]=0; m[15]=1;
}
static void rotate_vec(PhysObj *o,float lx,float ly,float lz,float *rx,float *ry,float *rz){
    float x=o->qx,y=o->qy,z=o->qz,w=o->qw;
    float xx=x*x, yy=y*y, zz=z*z;
    float xy=x*y, xz=x*z, yz=y*z;
    float wx=w*x, wy=w*y, wz=w*z;
    float m00=1-2*(yy+zz), m01=2*(xy-wz), m02=2*(xz+wy);
    float m10=2*(xy+wz), m11=1-2*(xx+zz), m12=2*(yz-wx);
    float m20=2*(xz-wy), m21=2*(yz+wx), m22=1-2*(xx+yy);
    *rx = m00*lx + m01*ly + m02*lz;
    *ry = m10*lx + m11*ly + m12*lz;
    *rz = m20*lx + m21*ly + m22*lz;
}

/* ── Level raycast — correct __thiscall, no trampoline ── */
static DWORD get_collision_level(void){
    DWORD app=*(DWORD*)GLOBAL_APP;
    if(!app||app<0x10000||IsBadReadPtr((void*)app,0x200)) return 0;
    DWORD board=*(DWORD*)(app+APP_BOARD);
    if(!board||board<0x10000||IsBadReadPtr((void*)board,0x3000)) return 0;
    // try board->sceneObj->collision, fallback to ball->scene
    // board+0x8AC is Level* -> Level+0x480 SceneObj -> SceneObj+0x8B0 is CollisionLevel? Actually scene is at 0x005341E4
    DWORD scene=*(DWORD*)0x005341E4;
    if(scene&&scene>=0x10000&&!IsBadReadPtr((void*)scene,0x900)){
        DWORD cl=*(DWORD*)(scene+0x8B0);
        if(cl&&cl>=0x10000) return cl;
    }
    // fallback via board's ball
    DWORD lb=board+BOARD_BALL_LIST;
    if(!IsBadReadPtr((void*)lb,0x410)){
        int c=*(int*)(lb+0x04);
        DWORD items=*(DWORD*)(lb+0x40C);
        if(items&&c>0&&c<64&&!IsBadReadPtr((void*)items,c*4)){
            for(int i=0;i<c;i++){
                DWORD ball=*(DWORD*)(items+i*4);
                if(!ball||ball<0x10000||IsBadReadPtr((void*)ball,0x300)) continue;
                DWORD sc=*(DWORD*)(ball+0x14);
                if(!sc||sc<0x10000||IsBadReadPtr((void*)sc,0x900)) continue;
                DWORD mesh_data=*(DWORD*)(sc+0x8B0);
                if(mesh_data&&mesh_data>=0x10000) return mesh_data;
            }
        }
    }
    return 0;
}
static int level_raycast(float ox,float oy,float oz, float *outHit){
    DWORD cl=get_collision_level();
    if(!cl) return 0;
    Vec3 origin={ox,oy,oz};
    Vec3 dir={0.0f,-1.0f,0.0f};
    Vec3 hit={0,0,0};
    float max_dist=64.0f; // broad-phase radius, not distance limit — 64 covers any box
    // __thiscall: ECX=cl, stack=out,origin,dir,max_dist (RET 0x20)
    // guard against bad CL; don't use MSVC __try in MinGW
    if(IsBadReadPtr((void*)cl,4) || IsBadCodePtr((FARPROC)pfn_raycast)) return 0;
    pfn_raycast((void*)cl, &hit, origin, dir, max_dist);
    // miss = ray endpoint ~994 units below origin (docs/RAYCASTING_FOR_DLL_MODS.md)
    float dy2 = hit.y - oy;
    if(dy2 < -900.0f) return 0; // no hit (far endpoint)
    // also reject NaN
    if(hit.y!=hit.y) return 0;
    outHit[0]=hit.x; outHit[1]=hit.y; outHit[2]=hit.z;
    return 1;
}
static float ground_height_at(float x,float z){
    float hit[3]; float oy= 2000.0f;
    if(!level_raycast(x,oy,z,hit)) return -1e30f;
    return hit[1];
}

/* ── Geometry ── */
static void compute_aabb_from_originals(PhysObj *o){
    float mn[3]={1e30f,1e30f,1e30f}, mx[3]={-1e30f,-1e30f,-1e30f};
    for(int i=0;i<o->buf_count;i++){ float *v=o->bufs[i].live; int n=o->bufs[i].vertCount*VERT_STRIDE; for(int k=0;k<n;k+=VERT_STRIDE){ if(v[k]<mn[0])mn[0]=v[k]; if(v[k+1]<mn[1])mn[1]=v[k+1]; if(v[k+2]<mn[2])mn[2]=v[k+2]; if(v[k]>mx[0])mx[0]=v[k]; if(v[k+1]>mx[1])mx[1]=v[k+1]; if(v[k+2]>mx[2])mx[2]=v[k+2]; } }
    o->min[0]=mn[0]; o->min[1]=mn[1]; o->min[2]=mn[2]; o->max[0]=mx[0]; o->max[1]=mx[1]; o->max[2]=mx[2];
}
static void capture_geometry(PhysObj *o){
    o->buf_count=0; if(!o->meshworld||IsBadReadPtr((void*)o->meshworld,0x440)) return;
    DWORD cnt=MB_COUNT(o->meshworld+MW_MESHBUFFER_LIST); DWORD *items=MB_ITEMS(o->meshworld+MW_MESHBUFFER_LIST);
    if(!items||cnt==0||cnt>64) return;
    for(DWORD i=0;i<cnt&&o->buf_count<16;i++){ DWORD mb=items[i]; if(!mb||mb<0x10000||IsBadReadPtr((void*)mb,0x420)) continue; DWORD sc=STRIP_COUNT(mb); float *sv=STRIP_ITEMS(mb); if(!sv||sc==0||sc>20000) continue; if(IsBadReadPtr((void*)sv, sc*STRIDE_VERTS*VERT_STRIDE*4)) continue; int vc=(int)(sc*STRIDE_VERTS); o->bufs[o->buf_count].live=sv; o->bufs[o->buf_count].vertCount=vc; o->bufs[o->buf_count].localPos=NULL; o->bufs[o->buf_count].localNrm=NULL; o->buf_count++; }
}
static void alloc_local_storage(PhysObj *o){
    for(int i=0;i<o->buf_count;i++){ int vc=o->bufs[i].vertCount; o->bufs[i].localPos=(float*)HeapAlloc(GetProcessHeap(),0,vc*3*sizeof(float)); o->bufs[i].localNrm=(float*)HeapAlloc(GetProcessHeap(),0,vc*3*sizeof(float)); }
}
static void free_local_storage(PhysObj *o){ for(int i=0;i<o->buf_count;i++){ if(o->bufs[i].localPos) HeapFree(GetProcessHeap(),0,o->bufs[i].localPos); if(o->bufs[i].localNrm) HeapFree(GetProcessHeap(),0,o->bufs[i].localNrm); o->bufs[i].localPos=o->bufs[i].localNrm=NULL; } }

static void store_locals(PhysObj *o){
    for(int i=0;i<o->buf_count;i++){ float *live=o->bufs[i].live; float *lp=o->bufs[i].localPos; float *ln=o->bufs[i].localNrm; int vc=o->bufs[i].vertCount; for(int v=0; v<vc; v++){ float *src=live+v*VERT_STRIDE; lp[v*3+0]=src[0]-o->x; lp[v*3+1]=src[1]-o->y; lp[v*3+2]=src[2]-o->z; ln[v*3+0]=src[3]; ln[v*3+1]=src[4]; ln[v*3+2]=src[5]; } }
}
static void rebuild_verts(PhysObj *o){
    for(int i=0;i<o->buf_count;i++){ float *live=o->bufs[i].live; float *lp=o->bufs[i].localPos; float *ln=o->bufs[i].localNrm; int vc=o->bufs[i].vertCount; for(int v=0; v<vc; v++){ float rx,ry,rz, nrx,nry,nrz; rotate_vec(o, lp[v*3+0], lp[v*3+1], lp[v*3+2], &rx,&ry,&rz); rotate_vec(o, ln[v*3+0], ln[v*3+1], ln[v*3+2], &nrx,&nry,&nrz); float *dst=live+v*VERT_STRIDE; dst[0]=o->x+rx; dst[1]=o->y+ry; dst[2]=o->z+rz; dst[3]=nrx; dst[4]=nry; dst[5]=nrz; } }
    /* recompute AABB from rotated verts */
    float mn[3]={1e30f,1e30f,1e30f}, mx[3]={-1e30f,-1e30f,-1e30f};
    for(int i=0;i<o->buf_count;i++){ float *live=o->bufs[i].live; int vc=o->bufs[i].vertCount; for(int v=0;v<vc;v++){ float *p=live+v*VERT_STRIDE; if(p[0]<mn[0])mn[0]=p[0]; if(p[1]<mn[1])mn[1]=p[1]; if(p[2]<mn[2])mn[2]=p[2]; if(p[0]>mx[0])mx[0]=p[0]; if(p[1]>mx[1])mx[1]=p[1]; if(p[2]>mx[2])mx[2]=p[2]; } }
    o->min[0]=mn[0]; o->min[1]=mn[1]; o->min[2]=mn[2]; o->max[0]=mx[0]; o->max[1]=mx[1]; o->max[2]=mx[2];
}
static void apply_render_transform(PhysObj *o){
    if(!o->meshworld||IsBadReadPtr((void*)o->meshworld,0x50)) return;
    float *m=(float*)((char*)o->meshworld+MW_WORLD_MATRIX);
    // verts are rebuilt in world space each frame (rebuild_verts), so matrix must be identity.
    // If we also put world matrix here, rendering double-transforms (vert*matrix).
    // Keep identity; rotation is already baked into verts. If you switch to local verts, restore quat_to_matrix here.
    m[0]=1; m[1]=0; m[2]=0; m[3]=0;
    m[4]=0; m[5]=1; m[6]=0; m[7]=0;
    m[8]=0; m[9]=0; m[10]=1; m[11]=0;
    m[12]=0; m[13]=0; m[14]=0; m[15]=1;
    (void)quat_to_matrix; // keep helper for future local-vert mode
}

/* ── Spawning ── */
static int spawn_object(DWORD board,const char *meshname,float rx,float ry,float rz){
    if(g_obj_count>=MAX_OBJECTS) return 0;
    DWORD gfx=0; DWORD app=*(DWORD*)GLOBAL_APP; if(app&&!IsBadReadPtr((void*)(app+0x174),4)) gfx=*(DWORD*)(app+0x174); if(!gfx) return 0;
    DWORD node=(DWORD)pfn_operator_new(0x18); if(!node) return 0; memset((void*)node,0,0x18);
    unsigned int ok=pfn_MeshNode_ctor((void*)node,gfx,meshname,0); DWORD mw=*(DWORD*)((char*)node+0x08);
    if(!ok||!mw||mw<0x10000||IsBadReadPtr((void*)mw,0x440)){ ilog("SPAWN: FAILED %s.MESH ok=%u",meshname,ok); return 0; }
    PhysObj *o=&g_objs[g_obj_count]; memset(o,0,sizeof(*o)); strncpy(o->mesh_name,meshname,MAX_NAME-1); o->meshworld=mw; capture_geometry(o);
    if(o->buf_count==0){ ilog("SPAWN: %s no strips",meshname); return 0; }
    compute_aabb_from_originals(o);
    float hx=(o->max[0]+o->min[0])*0.5f, hy=(o->max[1]+o->min[1])*0.5f, hz=(o->max[2]+o->min[2])*0.5f;
    float halfx=(o->max[0]-o->min[0])*0.5f, halfy=(o->max[1]-o->min[1])*0.5f, halfz=(o->max[2]-o->min[2])*0.5f;
    o->half[0]=halfx; o->half[1]=halfy; o->half[2]=halfz;
    o->x=rx; o->y=ry+halfy; o->z=rz;
    o->vx=o->vy=o->vz=0; o->wx=o->wy=o->wz=0; o->qx=0; o->qy=0; o->qz=0; o->qw=1;
    /* inertia for box: I = 1/3 m (half^2 sums), m=1 */
    float m=1.0f; o->invInertia[0]= 1.0f / ((1.0f/3.0f)*m*(o->half[1]*o->half[1]+o->half[2]*o->half[2] + 1e-4f));
    o->invInertia[1]= 1.0f / ((1.0f/3.0f)*m*(o->half[0]*o->half[0]+o->half[2]*o->half[2] + 1e-4f));
    o->invInertia[2]= 1.0f / ((1.0f/3.0f)*m*(o->half[0]*o->half[0]+o->half[1]*o->half[1] + 1e-4f));
    /* cap huge inertia for very flat objects */
    for(int k=0;k<3;k++) if(o->invInertia[k]>5.0f) o->invInertia[k]=5.0f;
    o->home[0]=o->x; o->home[1]=o->y; o->home[2]=o->z; o->grounded=0; o->render_fn=g_shared_render_fn;
    /* move verts so center aligns with pos */
    float cx=hx - o->x, cy=hy - o->y, cz=hz - o->z;
    for(int i=0;i<o->buf_count;i++){ float *v=o->bufs[i].live; int vc=o->bufs[i].vertCount; for(int k=0;k<vc;k++){ float *p=v+k*VERT_STRIDE; p[0]-=cx; p[1]-=cy; p[2]-=cz; } }
    /* recompute AABB after centering */
    compute_aabb_from_originals(o);
    alloc_local_storage(o); store_locals(o);
    apply_render_transform(o);
    g_obj_count++;
    ilog("SPAWN: '%s' at (%.1f,%.1f,%.1f) size=(%.0fx%.0fx%.0f) invI=(%.3f,%.3f,%.3f) bufs=%d", meshname,o->x,o->y,o->z, halfx*2,halfy*2,halfz*2, o->invInertia[0],o->invInertia[1],o->invInertia[2], o->buf_count);
    return 1;
}
static void setup_level(DWORD board){
    /* free previous locals */
    for(int i=0;i<g_obj_count;i++) free_local_storage(&g_objs[i]);
    g_level_done=1; g_obj_count=0;
    DWORD scene=*(DWORD*)0x005341E4; if(!scene||scene<0x10000||IsBadReadPtr((void*)(scene+BOARD_SCENE_OBJ),4)) return;
    DWORD level=*(DWORD*)((char*)scene+BOARD_SCENE_OBJ); if(!level||level<0x10000||IsBadReadPtr((void*)(level+LEVEL_SCENE_OBJ),4)) return;
    DWORD sceneobj=*(DWORD*)((char*)level+LEVEL_SCENE_OBJ); if(!sceneobj||sceneobj<0x10000) return;
    if(IsBadReadPtr((void*)((char*)sceneobj+SCENE_REF_COUNT),4)) return;
    int count=*(int*)((char*)sceneobj+SCENE_REF_COUNT); if(count<=0||count>4000) return;
    if(IsBadReadPtr((void*)((char*)sceneobj+SCENE_REF_ITEMS),4)) return;
    DWORD items_array=*(DWORD*)((char*)sceneobj+SCENE_REF_ITEMS); if(!items_array||items_array<0x10000) return;
    int found=0; for(int i=0;i<count;i++){ if(IsBadReadPtr((void*)(items_array+i*4),4)) break; DWORD item=*(DWORD*)(items_array+i*4); if(!item||item<0x10000||IsBadReadPtr((void*)item,16)) continue; char *name=*(char**)item; if(!name||IsBadReadPtr(name,6)) continue; if(_strnicmp(name,"CUBE:",5)!=0) continue; const char *meshname=name+5; if(!meshname[0]) continue; float rx=*(float*)(item+4), ry=*(float*)(item+8), rz=*(float*)(item+12); if(spawn_object(board,meshname,rx,ry,rz)) found++; }
    ilog("SETUP: %d CUBE objects (board=%p)",found,(void*)board);
}

/* ── Physics tick with tipping ── */
static void phys_tick(void){
    __asm__ volatile("fninit" ::: "memory", "st", "st(1)", "st(2)", "st(3)", "st(4)", "st(5)", "st(6)", "st(7)");
    if(game_is_quitting()) return;
    DWORD board=get_board(); if(!board) return;
    if(board!=g_last_board){ for(int i=0;i<g_obj_count;i++) free_local_storage(&g_objs[i]); g_last_board=board; g_level_done=0; g_obj_count=0; ilog("LEVEL: new board %p",(void*)board); }
    if(!g_level_done){ setup_level(board); return; }
    if(g_obj_count==0) return;
    int paused=*(BYTE*)((char*)board+PAUSE_FLAG)!=0; if(paused) return;
    DWORD ball=get_player_ball(board);
    const int SUBSTEPS=4;
    const float dt = 1.0f / (float)SUBSTEPS;
    for(int s=0; s<SUBSTEPS; s++){
    for(int i=0;i<g_obj_count;i++){
        PhysObj *o=&g_objs[i]; if(!o->meshworld) continue;
        /* pushing — only on first substep to avoid multi-accumulate */
        if(s==0 && ball && !IsBadReadPtr((void*)ball,0x300)){
            float px=*(float*)((char*)ball+BALL_POS+0), py=*(float*)((char*)ball+BALL_POS+4), pz=*(float*)((char*)ball+BALL_POS+8);
            float pr=*(float*)((char*)ball+BALL_RADIUS);
            float ex=pr+CFG_CONTACTGAP;
            int overlap_x=(px+ex>o->min[0])&&(px-ex<o->max[0]);
            int overlap_z=(pz+ex>o->min[2])&&(pz-ex<o->max[2]);
            int overlap_y=(py+pr>o->min[1])&&(py-pr<o->max[1]);
            int above=(py-pr) >= (o->max[1]-8.0f);
            if(overlap_x&&overlap_z&&overlap_y&&!above){
                float pen_x_lo=(px+ex)-o->min[0], pen_x_hi=o->max[0]-(px-ex);
                float pen_z_lo=(pz+ex)-o->min[2], pen_z_hi=o->max[2]-(pz-ex);
                float minpen=pen_x_lo; int axis=1, sign=-1;
                if(pen_x_hi<minpen){minpen=pen_x_hi; axis=1; sign=1;}
                if(pen_z_lo<minpen){minpen=pen_z_lo; axis=2; sign=-1;}
                if(pen_z_hi<minpen){minpen=pen_z_hi; axis=2; sign=1;}
                int push_axis=axis*sign;
                float pf = cfg_pushforce * 0.6f; /* per-substep portion handled via single apply */
                if(push_axis==1||push_axis==-1){ o->vx += pf*(push_axis>0?1:-1); if(o->vx>cfg_maxpushvel) o->vx=cfg_maxpushvel; if(o->vx<-cfg_maxpushvel) o->vx=-cfg_maxpushvel; }
                else { o->vz += pf*(push_axis>0?1:-1); if(o->vz>cfg_maxpushvel) o->vz=cfg_maxpushvel; if(o->vz<-cfg_maxpushvel) o->vz=-cfg_maxpushvel; }
                /* off-center push gives a little angular kick (feels more physical) */
                float offY = (py - o->y) / (o->half[1]+1.0f); /* -1..1 */
                if(axis==1) o->wz += pf*0.35f * (push_axis>0?1:-1) * offY;
                else        o->wx -= pf*0.35f * (push_axis>0?1:-1) * offY;
            }
        }
        /* gravity */
        o->vy -= cfg_gravity * dt;
        if(o->vy < -cfg_maxfall) o->vy = -cfg_maxfall;

        /* --- support / tipping logic (uses level mesh) --- */
        /* sample 4 bottom corners */
        float corners[4][3];
        float sgn[4][2]={{-1,-1},{1,-1},{1,1},{-1,1}};
        for(int c=0;c<4;c++){
            float lx=sgn[c][0]*o->half[0], ly=-o->half[1], lz=sgn[c][1]*o->half[2];
            float rx,ry,rz; rotate_vec(o,lx,ly,lz,&rx,&ry,&rz);
            corners[c][0]=o->x+rx; corners[c][1]=o->y+ry; corners[c][2]=o->z+rz;
        }
        int supported=0; float supCenter[3]={0,0,0}; float lowestPen= -1e30f;
        float minCornerY=1e30f;
        for(int c=0;c<4;c++) if(corners[c][1]<minCornerY) minCornerY=corners[c][1];
        for(int c=0;c<4;c++){
            float gh = ground_height_at(corners[c][0],corners[c][2]);
            if(gh==-1e30f) continue;
            float gap = corners[c][1] - gh; /* positive = above ground */
            /* supported if within snap band and not clearly airborne */
            if(gap >= -2.0f && gap <= cfg_snapdist){
                supported++; supCenter[0]+=corners[c][0]; supCenter[1]+=gh; supCenter[2]+=corners[c][2];
                float pen = gh - corners[c][1]; if(pen>lowestPen) lowestPen=pen;
            }
        }
        if(supported>0){ supCenter[0]/=supported; supCenter[1]/=supported; supCenter[2]/=supported; }

        if(supported>=3){
            /* fully supported — snap up if penetrating, kill vertical fall */
            if(lowestPen>0){ o->y += lowestPen; if(o->vy<0) o->vy=0; }
            /* small snap if hovering just above */
            else if(supported==4){
                float avgGap=0; for(int c=0;c<4;c++){ float gh=ground_height_at(corners[c][0],corners[c][2]); if(gh!=-1e30f) avgGap+=(corners[c][1]-gh); } avgGap/=4.0f;
                if(avgGap>0 && avgGap<cfg_snapdist && o->vy<0.05f){ o->y -= avgGap*0.5f*dt*4.0f; o->vy*=0.5f; }
            }
            o->grounded=1;
            /* friction */
            o->vx *= powf(cfg_friction, dt);
            o->vz *= powf(cfg_friction, dt);
            if(o->vx>-0.02f&&o->vx<0.02f) o->vx=0;
            if(o->vz>-0.02f&&o->vz<0.02f) o->vz=0;
            /* angular damping when grounded */
            o->wx *= powf(CFG_ANG_DAMP_GROUNDED, dt*4);
            o->wy *= powf(CFG_ANG_DAMP_GROUNDED, dt*4);
            o->wz *= powf(CFG_ANG_DAMP_GROUNDED, dt*4);
            if(fabsf(o->wx)<0.002f) o->wx=0;
            if(fabsf(o->wz)<0.002f) o->wz=0;
            /* if barely tilted while fully supported, add restoring torque toward flat */
            float tilt = sqrtf(o->qx*o->qx + o->qz*o->qz);
            if(tilt>0.02f){
                o->wx -= o->qx * 0.08f * dt;
                o->wz -= o->qz * 0.08f * dt;
            }
        } else if(supported==1 || supported==2){
            o->grounded=0;
            /* edge pivot — torque toward the unsupported side */
            float rx = supCenter[0]-o->x, rz = supCenter[2]-o->z;
            /* gravity torque: torque = r x F, F=(0,-mg,0) => (rz*mg, 0, -rx*mg) */
            float mg = 1.0f * cfg_gravity * 18.0f; /* scale to feel good */
            float tx = rz * mg;
            float tz = -rx * mg;
            /* apply */
            o->wx += tx * o->invInertia[0] * dt;
            o->wz += tz * o->invInertia[2] * dt;
            /* also a little forward tip from horizontal velocity carrying over edge */
            /* let it fall — no vertical snap */
            o->wx *= powf(CFG_ANG_DAMP_AIR, dt);
            o->wz *= powf(CFG_ANG_DAMP_AIR, dt);
        } else { /* 0 supported — airborne */
            o->grounded=0;
            /* slight air damping */
            o->wx *= powf(CFG_ANG_DAMP_AIR, dt);
            o->wz *= powf(CFG_ANG_DAMP_AIR, dt);
            o->wy *= powf(CFG_ANG_DAMP_AIR, dt);
            /* if falling with no support and not yet rotating, seed a tumble so it doesn't fall flat */
            float angLen = sqrtf(o->wx*o->wx+o->wz*o->wz);
            if(angLen < 0.02f && o->vy < -0.5f){
                /* bias based on which side overhung last frame — use velocity dir */
                float vlen = sqrtf(o->vx*o->vx+o->vz*o->vz);
                if(vlen>0.2f){ o->wx += (o->vz/vlen)*0.06f; o->wz -= (o->vx/vlen)*0.06f; }
            }
            /* ground probe below center to catch landing */
            {
                float gh = ground_height_at(o->x,o->z);
                if(gh!=-1e30f){
                    float bottomY = minCornerY;
                    float pen = gh - bottomY;
                    if(pen>0 && o->vy<=0){
                        o->y += pen;
                        o->vy = 0;
                        /* impact — convert some linear into angular */
                        float imp = fabsf(o->vy)*0.02f;
                        (void)imp;
                    }
                }
            }
        }

        /* integrate position */
        o->x += o->vx * dt;
        o->y += o->vy * dt;
        o->z += o->vz * dt;

        /* integrate orientation from angular velocity */
        /* qdot = 0.5 * q * omega */
        float qx=o->qx, qy=o->qy, qz=o->qz, qw=o->qw;
        float wx=o->wx, wy=o->wy, wz=o->wz;
        /* omega quat = (wx,wy,wz,0) */
        float qdotx = 0.5f*( qw*wx + qy*wz - qz*wy );
        float qdoty = 0.5f*( qw*wy + qz*wx - qx*wz );
        float qdotz = 0.5f*( qw*wz + qx*wy - qy*wx );
        float qdotw = 0.5f*( -qx*wx - qy*wy - qz*wz );
        o->qx += qdotx*dt; o->qy += qdoty*dt; o->qz += qdotz*dt; o->qw += qdotw*dt;
        quat_normalize(o);

        /* clamp angular velocity */
        const float amax=6.0f; if(o->wx>amax) o->wx=amax; if(o->wx<-amax) o->wx=-amax; if(o->wz>amax) o->wz=amax; if(o->wz<-amax) o->wz=-amax; if(o->wy>3.0f) o->wy=3.0f; if(o->wy<-3.0f) o->wy=-3.0f;
    } /* per object */
    } /* substeps */

    /* rebuild verts + render matrices after all substeps (guard freed MW) */
    for(int i=0;i<g_obj_count;i++){ PhysObj *o=&g_objs[i]; if(!o->meshworld||IsBadReadPtr((void*)o->meshworld,0x50)) { o->meshworld=0; continue; } // MW freed on level unload
        // also guard live verts pointer freed with MW
        int bad=0; for(int b=0;b<o->buf_count;b++) if(!o->bufs[b].live||IsBadReadPtr(o->bufs[b].live,4)) bad=1;
        if(bad){ o->meshworld=0; continue; }
        rebuild_verts(o); apply_render_transform(o); }

    /* carrying — ball on top follows (with rotation) */
    if(ball && !IsBadReadPtr((void*)ball,0x300)){
        float bx=*(float*)((char*)ball+BALL_POS+0), by=*(float*)((char*)ball+BALL_POS+4), bz=*(float*)((char*)ball+BALL_POS+8);
        for(int i=0;i<g_obj_count;i++){ PhysObj *o=&g_objs[i]; if(!o->meshworld||!o->grounded) continue;
            /* top face center in world (approx) — use y = max[1] */
            if(bx > o->min[0]-10 && bx < o->max[0]+10 && bz > o->min[2]-10 && bz < o->max[2]+10 && by >= o->max[1]-22 && by <= o->max[1]+40){
                /* move ball with box delta: for now use linear delta only (dy from last rebuild is implicit) */
                /* Estimate box top delta via its velocity */
                *(float*)((char*)ball+BALL_POS+0) += o->vx;
                *(float*)((char*)ball+BALL_POS+2) += o->vz;
                /* vertical follows top surface — snap to top */
                float topY = o->max[1];
                float by2 = *(float*)((char*)ball+BALL_POS+4);
                if(by2 < topY+30 && by2 > topY-20) *(float*)((char*)ball+BALL_POS+4) = topY + 26.0f;
                break;
            }
        }
    }

    /* out-of-world reset */
    for(int i=0;i<g_obj_count;i++){ PhysObj *o=&g_objs[i]; if(!o->meshworld||IsBadReadPtr((void*)o->meshworld,0x50)) continue; if(o->y < -800.0f){ o->x=o->home[0]; o->y=o->home[1]; o->z=o->home[2]; o->vx=o->vy=o->vz=0; o->wx=o->wy=o->wz=0; o->qx=0; o->qy=0; o->qz=0; o->qw=1; rebuild_verts(o); apply_render_transform(o); ilog("RESET: '%s' fell out of world",o->mesh_name); } }
}

/* Render driver */
BYTE *g_render_trampoline=NULL;
static int g_render_suspended=0;
typedef void (__thiscall *SceneRender_t)(void*,int);
static void __fastcall render_hook_c(int gfx_this,void *edx_dummy){ (void)edx_dummy;(void)gfx_this; if(!game_is_quitting()&&g_obj_count>0&&!g_render_suspended){ for(int i=0;i<g_obj_count;i++){ PhysObj *o=&g_objs[i]; if(!o->meshworld||!o->render_fn) continue; if(IsBadCodePtr((FARPROC)o->render_fn)) continue; ((SceneRender_t)o->render_fn)((void*)o->meshworld,0); } } }
static void patch_bytes(DWORD addr,BYTE *bytes,int len){ DWORD old; VirtualProtect((void*)addr,len,PAGE_EXECUTE_READWRITE,&old); memcpy((void*)addr,bytes,len); VirtualProtect((void*)addr,len,old,&old); FlushInstructionCache(GetCurrentProcess(),(void*)addr,len); }
static BYTE *alloc_executable(int size){ return (BYTE*)VirtualAlloc(NULL,size,MEM_COMMIT|MEM_RESERVE,PAGE_EXECUTE_READWRITE); }
#define HOOK_TICK       0x0046C1F1
#define HOOK_TICK_RET   0x0046C1F6
#define HOOK_RENDER     0x00454BC0
static BYTE g_render_orig[6]; static BYTE *g_render_stub=NULL;
static void build_tick_hook(void){
    static const BYTE expect[5]={0x5E,0x83,0xC4,0x08,0xC3};
    if(memcmp((const void*)HOOK_TICK,expect,5)!=0){ ilog("HOOK: tick mismatch - skipped"); return; }
    g_tick_cave=alloc_executable(32); if(!g_tick_cave) return; int i=0; BYTE *c=g_tick_cave;
    c[i++]=0x60; c[i++]=0x9C; c[i++]=0xB8; *(DWORD*)(c+i)=(DWORD)&phys_tick; i+=4; c[i++]=0xFF; c[i++]=0xD0; c[i++]=0x9D; c[i++]=0x61; memcpy(c+i,expect,5); i+=5; c[i++]=0xE9; *(DWORD*)(c+i)=HOOK_TICK_RET-(DWORD)(c+i+4);
    BYTE patch[5]; patch[0]=0xE9; *(DWORD*)(patch+1)=(DWORD)g_tick_cave-HOOK_TICK-5; patch_bytes(HOOK_TICK,patch,5); ilog("HOOK: tick installed");
}
static void build_render_hook(void){
    memcpy(g_render_orig,(void*)HOOK_RENDER,6); g_render_trampoline=alloc_executable(16); if(!g_render_trampoline) return; memcpy(g_render_trampoline,g_render_orig,6); g_render_trampoline[6]=0xE9; *(DWORD*)(g_render_trampoline+7)=(DWORD)(HOOK_RENDER+6)-(DWORD)(g_render_trampoline+11);
    g_render_stub=alloc_executable(32); if(!g_render_stub) return; int i=0; BYTE *c=g_render_stub; c[i++]=0x51; c[i++]=0x52; c[i++]=0xE8; *(DWORD*)(c+i)=(DWORD)&render_hook_c-(DWORD)(c+i+4); i+=4; c[i++]=0x5A; c[i++]=0x59; c[i++]=0xB9; *(DWORD*)(c+i)=(DWORD)g_render_trampoline; i+=4; c[i++]=0xFF; c[i++]=0xE1; BYTE patch[6]; patch[0]=0xE9; *(DWORD*)(patch+1)=(DWORD)g_render_stub-HOOK_RENDER-5; patch[5]=0x90; patch_bytes(HOOK_RENDER,patch,6); ilog("HOOK: Graphics_RenderScene installed");
}
static void lazy_load_real_bass(void){
    static int done=0; if(done) return; done=1;
    char path[MAX_PATH]; get_dll_dir(path,MAX_PATH); lstrcatA(path,"bass_real.dll");
    g_hRealBass=LoadLibraryA(path);
    if(!g_hRealBass){ char sys[MAX_PATH]; GetSystemDirectoryA(sys,MAX_PATH); lstrcatA(sys,"\\bass.dll"); g_hRealBass=LoadLibraryA(sys); }
    if(!g_hRealBass) return;
    real_BASS_ChannelSetAttributes=(BASS_ChannelSetAttributes_t)GetProcAddress(g_hRealBass,"BASS_ChannelSetAttributes");
    real_BASS_MusicPlayEx=(BASS_MusicPlayEx_t)GetProcAddress(g_hRealBass,"BASS_MusicPlayEx");
    real_BASS_MusicLoad=(BASS_MusicLoad_t)GetProcAddress(g_hRealBass,"BASS_MusicLoad");
    real_BASS_SampleLoad=(BASS_SampleLoad_t)GetProcAddress(g_hRealBass,"BASS_SampleLoad");
    real_BASS_StreamCreateFile=(BASS_StreamCreateFile_t)GetProcAddress(g_hRealBass,"BASS_StreamCreateFile");
    real_BASS_SampleGetChannel=(BASS_SampleGetChannel_t)GetProcAddress(g_hRealBass,"BASS_SampleGetChannel");
    real_BASS_ChannelPlay=(BASS_ChannelPlay_t)GetProcAddress(g_hRealBass,"BASS_ChannelPlay");
    real_BASS_ChannelStop=(BASS_ChannelStop_t)GetProcAddress(g_hRealBass,"BASS_ChannelStop");
    real_BASS_Init=(BASS_Init_t)GetProcAddress(g_hRealBass,"BASS_Init");
    real_BASS_Free=(BASS_Free_t)GetProcAddress(g_hRealBass,"BASS_Free");
    real_BASS_Stop=(BASS_Stop_t)GetProcAddress(g_hRealBass,"BASS_Stop");
    real_BASS_Start=(BASS_Start_t)GetProcAddress(g_hRealBass,"BASS_Start");
    real_BASS_SetConfig=(BASS_SetConfig_t)GetProcAddress(g_hRealBass,"BASS_SetConfig");
    real_BASS_ErrorGetCode=(BASS_ErrorGetCode_t)GetProcAddress(g_hRealBass,"BASS_ErrorGetCode");
}
static DWORD WINAPI patch_thread(LPVOID p){
    (void)p; Sleep(600); load_config();
    /* resolve render fn from live MeshWorld vtable if possible, else fallback */
    g_shared_render_fn=0x0045E0E0;
    HMODULE hm=GetModuleHandleA(NULL); if(hm){ DWORD *vt=(DWORD*)0x004D8FB0; if(!IsBadReadPtr(vt,0x60)) g_shared_render_fn=vt[18]; }
    ilog("=== physics_objects v2 (tipping) started ==="); ilog("RENDER: vtable[18]=%p", (void*)g_shared_render_fn);
    build_tick_hook(); build_render_hook(); return 0;
}
BOOL WINAPI DllMain(HINSTANCE h,DWORD r,LPVOID v){ (void)v; if(r==DLL_PROCESS_ATTACH){ DisableThreadLibraryCalls(h); CreateThread(NULL,0,patch_thread,NULL,0,NULL); } return TRUE; }

